#include "animation.hh"

#include "loader.hh"

namespace gdt {
animation::animation(std::string filename, const skeleton & s, bool loop) {
    _frames = read_animation(filename.c_str());
    _skeleton = s;
    _loop = loop;
}
animixer::animixer(const skeleton & s) {
    _skeleton = s;
}
}
