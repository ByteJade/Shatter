#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void dump(const fs::directory_iterator& it, const char* name) {
    void* dir_ptr = *reinterpret_cast<void* const*>(&it);

    std::cout << name << ": _M_dir=" << dir_ptr;

    if (it != fs::directory_iterator{}) {
        std::cout << " path=\"" << it->path().string() << "\"";
    } else {
        std::cout << " (end)";
    }
    std::cout << "\n";
}

int main() {
    std::error_code ec;
    auto&& __range = fs::directory_iterator(".", ec);
    auto __begin = begin(__range);
    auto __end   = end(__range);
    dump(__begin, "__begin");
    dump(__end, "__end");
    for (; __begin != __end; ++__begin) {
        dump(__begin, "it");
        auto& entry = *__begin;
        std::cout << entry.path() << "\n";
    }
    dump(__begin, "__begin");
    dump(__end, "__end");
    return 0;
}
