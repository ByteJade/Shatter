#include <vector>
#include <filesystem>
#include <iostream>

int main() {
    std::vector<std::filesystem::path> paths;
    std::filesystem::path p = "/some/path";

    for (int i = 0; i < 32; i++) {
        paths.push_back(p);
        std::cout << "append " << i << std::endl;
    }
    std::cout << "Total size: " << paths.size() << std::endl;
    std::cout << "Elements: " << std::endl;
    for (int i = 0; i < 32; i++) {
        std::cout << i << ": " << paths[i] << std::endl;
    }
}