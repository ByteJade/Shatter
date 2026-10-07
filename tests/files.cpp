#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

int main() {
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(".", ec)) {
        std::cout << entry.path() << "\n";
    }
    return 0;
}