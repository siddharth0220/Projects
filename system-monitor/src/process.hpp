#pragma once
#include <string>
#include <vector>
struct ProcessInfo {
    int pid;
    std::string name;
    unsigned long long utime_ticks; // user time (clock ticks)
    unsigned long long stime_ticks; // system time (clock ticks)
    unsigned long long total_time_ticks() const { return utime_ticks + stime_ticks; }
    long rss_kb; // resident set size in KB
    double cpu_percent;
    double mem_percent;
};
bool read_process_info(int pid, ProcessInfo &p);
std::vector<int> list_pids();