#include <fstream>
#include <iostream>

double get_idle_time() {
    double uptime, idle;
    std::ifstream ifs("/proc/uptime");
    ifs >> uptime >> idle;
    std::cout << "idle: " << idle << " uptime: " << uptime << std::endl;
    if (!ifs.fail()) return idle;
    return 0.0;
}

int main() {
    get_idle_time();
    return 0;
}