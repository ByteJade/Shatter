#include  "../include/Compiler.hpp"
#include  "../include/Logger.hpp"
#include  "../include/Cache.hpp"
#include  "../include/Printer_X86_64.hpp"
#include <algorithm>
#include <cstdint>

void Compiler::jump(uint8_t* dst) {
    uint32_t buf = buffer.size();
    blocks.push_back({dst, 0,buf, 0});
    decoder.set_guest(dst);
}
bool Compiler::has_block(uint8_t* p) {
    for (Block& b : blocks) {
        if (p >= b.start && p < b.end) {
            return true;
        }
    }
    return false;
}
bool Compiler::forward() {
    Block& prev = blocks.back();
    prev.size = buffer.size() - prev.buffer;
    prev.end = decoder.get_guest();
    for (; reader < points.size(); reader++) {
        uint8_t* p = points[reader].point;
        if (!has_block(p)) {
            jump(p);
            return true;
        }
    }
    return false;
}
Point* Compiler::search_point(uint8_t* guest) {
    if (points.empty()) return nullptr;
    if (guest > points.back().point) return nullptr;
    size_t left = 0;
    size_t right = points.size();
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        Point* p = &points[mid];
        if (p->point == guest) return p;
        if (p->point < guest) left = mid + 1;
        else right = mid;
    }
    return nullptr;
}
void Compiler::set_point(uint8_t* guest) {
    size_t i = 0;
    for (; i < points.size(); i++) {
        uint8_t* p = points[i].point;
        if (p == guest) return;
        if (p > guest) break;
    }
    size_t end = points.size();
    points.push_back({});
    for (size_t y = end; y > i; y--) {
        points[y].point = points[y-1].point;
    }
    points[i].point = guest;
}
void Compiler::decode(uint8_t* code) {
    need_entry = false;
    has_jmp = false;
    logger.deb() << "Start compile " << (size_t)code << std::endl;
    blocks.push_back({code, 0, 0, 0});
    decoder.set_guest(code);
    while (1) {
        X86_64 buf;
        uint8_t* prev_pos = decoder.get_guest();
        decoder.decode(buf);
        uint8_t* cur_pos  = decoder.get_guest();
        sizes.push_back(cur_pos - prev_pos);
        buffer.push_back(buf);
        if (buf.type >= JO && buf.type <= JG) {
            set_point(cur_pos + buf.dst.imm);
        } else if (buf.type == JMP || buf.type == RET || buf.type == HLT){
            if (buf.type == RET) need_entry = true;
            if (buf.dst.type == IMM) {
                set_point(cur_pos + buf.dst.imm);
            } else if (buf.type == JMP) has_jmp = true;
            if(!forward()) break;
            continue;
        }
        if (search_point(cur_pos) && !forward()) break;
    }
    std::sort(blocks.begin(), blocks.end(),
    [](const auto& a, const auto& b) { return a.start < b.start; });
    logger.deb() << "End compile, blocks: " << blocks.size() << std::endl;
}
void Compiler::iterate(Block& block) {
    guest = block.start;
    uint32_t end = block.buffer + block.size;
    for (reader = block.buffer; reader < end; reader++) {
        Point* p = search_point(guest);
        if (p) p->host = cache.get_host();
        guest += sizes[reader];
        X86_64& buf = buffer[reader];
        print(logger.log(), buf);
        encode(buf);
    }
}
void Compiler::patch() {
    logger.deb() << "Patch " << patches.size() << " jumps" << std::endl;
    for (Patch& p : patches) {
        Point* n = search_point(p.guest);
        if (n) {
            emit_jump(*p.host, p.host, n->host);
        } else logger.err() << "Compiler: Cannot patch jump" << std::endl;
    }
}

void Compiler::compile(uint8_t* code) {
    reader = 0;
    decode(code);
    cache.start_block(code);
    if (!has_jmp) emit_entry();
    for (Block& block : blocks) {
        logger.log() << "start" << std::endl;
        iterate(block);
    }
    patch();
    cache.end_block();
    blocks.clear();
    sizes.clear();
    buffer.clear();
    points.clear();
    patches.clear();
}

X86_64& Compiler::next(int i) {
    return buffer[reader+i];
}
void Compiler::skip(int i) {
    reader += i;
}
