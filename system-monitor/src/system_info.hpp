#pragma once
#include "process.hpp"
#include <vector>

class SystemInfo {
public:
    SystemInfo();
    ~SystemInfo() = default;

    void sample();

    std::vector<ProcessInfo> processes() const;
    double total_cpu_percent() const;
    double total_mem_percent() const;

private:
    unsigned long long read_total_cpu_ticks() const;
    unsigned long long last_total_cpu_ticks;
    unsigned long long last_idle_ticks;
    unsigned long long last_sample_time_ms;

    double total_cpu_pct;
    double total_mem_pct;

    unsigned long long mem_total_kb;
    std::vector<ProcessInfo> proc_list;

    void update_mem_total();
};
