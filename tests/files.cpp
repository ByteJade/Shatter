#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

int main(int argc, char** argv) {
    fs::path p = "/some/path/file.tar.gz";
    fs::path ext = p.extension();
    std::cout << p << " first: " << ext << std::endl;
    fs::path my_p = argv[0];
    fs::path my_ext = my_p.extension();
    std::cout << my_p << " second: " << my_ext << std::endl;
}