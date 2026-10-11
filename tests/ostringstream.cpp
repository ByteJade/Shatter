#include <sstream>
#include <iostream>

int main() {
    std::string s = "hello";
    std::cout << "len=" << s.size() << " [" << s << "]\n";
    std::ostringstream oss;
    oss << "hello";
    s = oss.str();
    std::cout << "len=" << s.size() << " [" << s << "]\n";
    return 0;
}