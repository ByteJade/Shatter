#include <sstream>
#include <iostream>

int main() {
    std::stringbuf sb;
    sb.sputc('h');
    sb.sputc('e');
    sb.sputc('l');
    sb.sputc('l');
    sb.sputc('o');
    std::string s = sb.str();
    std::cout << "len=" << s.size() << " [" << s << "]\n";
    return 0;
}