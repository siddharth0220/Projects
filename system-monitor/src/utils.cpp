#include "utils.hpp"
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <unistd.h>

using namespace std;
vector<string> split_whitespace(const string &s) {
    istringstream iss(s);
    vector<string> out;
    string tok;
    while (iss >> tok) out.push_back(tok);
    return out;
}

unsigned long long get_uptime_ticks() {
    ifstream f("/proc/uptime");
    if (!f.is_open()) return 0;
    double uptime_seconds = 0.0;
    f >> uptime_seconds;
    f.close();
    long ticks = sysconf(_SC_CLK_TCK);
    return static_cast<unsigned long long>(uptime_seconds * ticks);
}
