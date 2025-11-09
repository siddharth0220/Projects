#include "process.hpp"
#include "utils.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <iomanip>
#include <iostream>
#include <algorithm>

using namespace std;

bool read_process_info(int pid, ProcessInfo &p) {
    string statPath = "/proc/" + to_string(pid) + "/stat";
    string statusPath = "/proc/" + to_string(pid) + "/status";

    ifstream statFs(statPath);
    if (!statFs.is_open()) return false;

    // /proc/<pid>/stat fields: see man proc
    // format: pid (comm) state ppid ... utime stime cutime cstime ...
    string line;
    getline(statFs, line);
    statFs.close();

    auto lparen = line.find('(');
    auto rparen = line.rfind(')');
    if (lparen == string::npos || rparen == string::npos || rparen <= lparen) return false;

    string comm = line.substr(lparen + 1, rparen - lparen - 1);
    string after = line.substr(rparen + 2); // skip ") "

    // tokenise remaining
    istringstream iss(after);
    vector<string> tokens;
    string tok;
    while (iss >> tok) tokens.push_back(tok);

    if (tokens.size() < 15) return false;

    unsigned long long utime_ticks = stoull(tokens[11]);
    unsigned long long stime_ticks = stoull(tokens[12]);

    long rss_kb = 0;
    ifstream statusFs(statusPath);
    if (statusFs.is_open()) {
        string sline;
        while (getline(statusFs, sline)) {
            if (sline.rfind("VmRSS:", 0) == 0) {
                vector<string> parts = split_whitespace(sline);
                if (parts.size() >= 2) rss_kb = stol(parts[1]);
                break;
            }
        }
        statusFs.close();
    } else {
        // fallback: try /proc/<pid>/statm (pages)
        string statmPath = "/proc/" + to_string(pid) + "/statm";
        ifstream statmFs(statmPath);
        if (statmFs.is_open()) {
            long rss_pages = 0;
            if (statmFs >> ws >> tok >> rss_pages) {
                long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;
                rss_kb = rss_pages * page_size_kb;
            }
            statmFs.close();
        }
    }

    p.pid = pid;
    p.name = comm;
    p.utime_ticks = utime_ticks;
    p.stime_ticks = stime_ticks;
    p.rss_kb = rss_kb;
    p.cpu_percent = 0.0;
    p.mem_percent = 0.0;
    return true;
}
vector<int> list_pids() {
    vector<int> pids;
    DIR *dp = opendir("/proc");
    if (!dp) return pids;
    struct dirent *entry;
    while ((entry = readdir(dp)) != nullptr) {
        if (entry->d_type != DT_DIR) continue;
        string name(entry->d_name);
        if (all_of(name.begin(), name.end(), ::isdigit)) {
            pids.push_back(stoi(name));
        }
    }
    closedir(dp);
    return pids;
}
