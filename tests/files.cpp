#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

namespace fs = std::filesystem;

struct fake_dir_iter {
    std::shared_ptr<void> _M_dir;
    char                  padding[16];
    fs::path              _M_path;
};

void dump(const fs::directory_iterator& it, const char* name) {
    auto* p = reinterpret_cast<const fake_dir_iter*>(&it);
    
    void* dir_ptr = p->_M_dir.get();
    std::string path_str = p->_M_path.string();

    std::cout << name
              << ": _M_dir=" << dir_ptr
              << " path=\"" << path_str << "\"\n";
}

int main() {
    std::error_code ec;
    fs::directory_iterator it(".", ec);
    fs::directory_iterator end;

    while (it != end) {
        dump(it, "cur");
        it++; 
    }
    dump(it, "end");
    return 0;
}