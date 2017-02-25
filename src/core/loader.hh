#ifndef SRC_CONSTRUCTS_LOADER_HH_INCLUDED
#define SRC_CONSTRUCTS_LOADER_HH_INCLUDED

#include <memory>
#include "animation.hh"
#include "mesh.hh"

namespace gdt {
std::unique_ptr<gdt::model> read_smd(const char* filename);
std::vector<frame> read_animation(const char* filename);
skeleton read_skeleton(const char* filename);
std::unique_ptr<model> obj_load_file(const char* filename);
bool load_png_for_texture(unsigned char** img,
                          unsigned int* width,
                          unsigned int* height,
                          const char* filename);
}

#endif // src/core/loader_hh_INCLUDED

