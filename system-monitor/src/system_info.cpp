#include "system_info.hpp"
#include "process.hpp"
#include "utils.hpp"
#include <dirent.h>
#include <unordered_map>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <unistd.h>
using namespace std;
using namespace std::chrono;

SystemInfo::SystemInfo()
{
    last_total_cpu_ticks = 0;
    last_idle_ticks = 0;
    total_cpu_pct = 0.0;
    total_mem_pct = 0.0;
    mem_total_kb = 1; // avoid div by zero
    update_mem_total();
    last_sample_time_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    // initial CPU sample
    ifstream f("/proc/stat");
    if (f.is_open())
    {
        string line;
        getline(f, line);
        f.close();
        // parse
        istringstream iss(line);
        string cpu;
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
        iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
        last_idle_ticks = idle + iowait;
        last_total_cpu_ticks = user + nice + system + idle + iowait + irq + softirq + steal;
    }
}

unsigned long long SystemInfo::read_total_cpu_ticks() const
{
    ifstream f("/proc/stat");
    if (!f.is_open())
        return 0;
    string line;
    getline(f, line);
    f.close();
    istringstream iss(line);
    string cpu;
    unsigned long long user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0;
    iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    return total;
}

void SystemInfo::update_mem_total()
{
    ifstream f("/proc/meminfo");
    if (!f.is_open())
        return;
    string line;
    while (getline(f, line))
    {
        if (line.rfind("MemTotal:", 0) == 0)
        {
            // MemTotal:   16367472 kB
            istringstream iss(line);
            string label;
            unsigned long long value;
            string unit;
            iss >> label >> value >> unit;
            mem_total_kb = value;
            break;
        }
    }
    f.close();
}

void SystemInfo::sample()
{
    // read total CPU ticks and idle
    ifstream f("/proc/stat");
    unsigned long long user = 0, nice = 0, system_ = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0;
    if (f.is_open())
    {
        string line;
        getline(f, line);
        istringstream iss(line);
        string cpu;
        iss >> cpu >> user >> nice >> system_ >> idle >> iowait >> irq >> softirq >> steal;
        f.close();

        unsigned long long idle_ticks = idle + iowait;
        unsigned long long total_ticks = user + nice + system_ + idle + iowait + irq + softirq + steal;

        unsigned long long total_delta = total_ticks - last_total_cpu_ticks;
        unsigned long long idle_delta = idle_ticks - last_idle_ticks;
        if (total_delta > 0)
        {
            double cpu_usage = (double)(total_delta - idle_delta) * 100.0 / (double)total_delta;
            total_cpu_pct = cpu_usage;
        }
        else
        {
            total_cpu_pct = 0.0;
        }

        last_total_cpu_ticks = total_ticks;
        last_idle_ticks = idle_ticks;
    }

    // update mem total
    update_mem_total();

    // sample processes
    proc_list.clear();
    vector<int> pids;
    // list /proc pids (simple)
    DIR *dp = opendir("/proc");
    if (dp)
    {
        struct dirent *entry;
        while ((entry = readdir(dp)) != nullptr)
        {
            if (entry->d_type != DT_DIR)
                continue;
            string name(entry->d_name);
            if (all_of(name.begin(), name.end(), ::isdigit))
            {
                pids.push_back(stoi(name));
            }
        }
        closedir(dp);
    }

    vector<ProcessInfo> curr;
    for (int pid : pids)
    {
        ProcessInfo p;
        if (read_process_info(pid, p))
            curr.push_back(p);
    }

    unordered_map<int, unsigned long long> prev_total_ticks;
    unordered_map<int, long> prev_rss;

    for (const auto &pp : proc_list)
    {
        prev_total_ticks[pp.pid] = pp.total_time_ticks();
        prev_rss[pp.pid] = pp.rss_kb;
    }
    static unordered_map<int, unsigned long long> static_prev_ticks;
    static unordered_map<int, long> static_prev_rss;
    unsigned long long total_ticks_now = read_total_cpu_ticks();

    for (auto &p : curr)
    {
        if (total_ticks_now > 0)
        {
            // approximate
            unsigned long long proc_ticks = p.total_time_ticks();
            long clk = sysconf(_SC_CLK_TCK);
            // Convert to seconds, rough share:
            double proc_seconds = (double)proc_ticks / (double)clk;
            double total_seconds = (double)total_ticks_now / (double)clk;
            double pct = (total_seconds > 0.0) ? (proc_seconds / total_seconds) * 100.0 : 0.0;
            p.cpu_percent = pct;
        }
        else
        {
            p.cpu_percent = 0.0;
        }
        // mem%
        if (mem_total_kb > 0)
            p.mem_percent = (double)p.rss_kb * 100.0 / (double)mem_total_kb;
        else
            p.mem_percent = 0.0;
    }

    // sort default by cpu desc
    sort(curr.begin(), curr.end(), [](const ProcessInfo &a, const ProcessInfo &b)
         { return a.cpu_percent > b.cpu_percent; });

    proc_list = move(curr);

    // compute total mem %

    // compute total memory usage percent (used = MemTotal - MemAvailable/Free)
    ifstream memFs("/proc/meminfo");
    unsigned long long memTotal = 0, memFree = 0, memAvailable = 0, buffers = 0, cached = 0;
    if (memFs.is_open())
    {
        string key;
        unsigned long long val;
        string unit;
        while (memFs >> key >> val >> unit)
        {
            if (key == "MemTotal:")
                memTotal = val;
            else if (key == "MemFree:")
                memFree = val;
            else if (key == "MemAvailable:")
                memAvailable = val;
            else if (key == "Buffers:")
                buffers = val;
            else if (key == "Cached:")
                cached = val;
        }
        memFs.close();
    }
    if (memTotal == 0)
        memTotal = mem_total_kb;
    unsigned long long used = (memAvailable > 0) ? (memTotal - memAvailable) : (memTotal - memFree);
    if (memTotal > 0)
        total_mem_pct = (double)used * 100.0 / (double)memTotal;
    else
        total_mem_pct = 0.0;
}

vector<ProcessInfo> SystemInfo::processes() const
{
    return proc_list;
}

double SystemInfo::total_cpu_percent() const
{
    return total_cpu_pct;
}

double SystemInfo::total_mem_percent() const
{
    return total_mem_pct;
}
