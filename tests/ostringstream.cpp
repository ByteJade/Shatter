#include <sstream>
#include <iostream>

int main() {
    std::ostringstream oss;
    oss << "hello";
    std::string s = oss.str();
    std::cout << "len=" << s.size() << " [" << s << "]\n";
    return 0;
}