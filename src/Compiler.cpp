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
    for (; reader < points.size(); reader++) {
        uint8_t* p = points[reader].point;
        if (!has_block(p)) {
            jump(p);
            return true;
        }
    }
    return false;
}
void Compiler::set_point(uint8_t* guest) {
    for (Point& p : points) {
        if (p.point == guest) return;
    }
    points.push_back({guest, 0});
}
void Compiler::decode(uint8_t* code) {
    need_entry = false;
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
            Block& prev = blocks.back();
            prev.size = buffer.size() - prev.buffer;
            prev.end = cur_pos;
            if (buf.type == RET) need_entry = true;
            if (buf.dst.type == IMM) {
                set_point(cur_pos + buf.dst.imm);
            }
            if(!forward()) break;
        }
        for (Point& p : points) {
            if (p.point == cur_pos) {
                if(!forward()) goto exit;
            }
        }
    }
    exit:
    std::sort(blocks.begin(), blocks.end(),
    [](const auto& a, const auto& b) { return a.start < b.start; });
    logger.deb() << "End compile, blocks: " << blocks.size() << std::endl;
}
void Compiler::iterate(Block& block) {
    guest = block.start;
    uint32_t end = block.buffer + block.size;
    for (reader = block.buffer; reader < end; reader++) {
        for (Point& p : points) {
            if (p.point == guest) {
                p.host = cache.get_host();
                break;
            }
        }
        guest += sizes[reader];
        X86_64& buf = buffer[reader];
        print(logger.log(), buf);
        encode(buf);
    }
}

void Compiler::compile(uint8_t* code) {
    reader = 0;
    decode(code);
    cache.start_block(code);
    if (need_entry) emit_entry();
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