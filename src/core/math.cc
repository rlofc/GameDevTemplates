#include "math.hh"

namespace gdt {
namespace math {

float max(float x, float y) {
    return x > y ? x : y;
}

float min(float x, float y) {
    return x < y ? x : y;
}

void vec2i_to_array(vec2i v, uint32_t* out) {
    out[0] = v.x;
    out[1] = v.y;
}

void vec2_to_array(vec2 v, float* out) {
    out[0] = v.x;
    out[1] = v.y;
}

void vec3_to_array(vec3 v, float* out) {
    out[0] = v.x;
    out[1] = v.y;
    out[2] = v.z;
}

void vec4_to_array(vec4 v, float* out) {
    out[0] = v.x;
    out[1] = v.y;
    out[2] = v.z;
    out[3] = v.w;
}

void quat_to_array(quat v, float* out) {
    vec4_to_array(v, out);
}

void mat4_to_array(mat4 m, float* out) {
    out[0] = m.xx;
    out[1] = m.yx;
    out[2] = m.zx;
    out[3] = m.wx;

    out[4] = m.xy;
    out[5] = m.yy;
    out[6] = m.zy;
    out[7] = m.wy;

    out[8] = m.xz;
    out[9] = m.yz;
    out[10] = m.zz;
    out[11] = m.wz;

    out[12] = m.xw;
    out[13] = m.yw;
    out[14] = m.zw;
    out[15] = m.ww;
}
};
};

std::ostream &operator<<(std::ostream &os, const gdt::math::vec2 &v) { 
    return os << "[ " << v.x << ", " << v.y << " ]";
}

std::ostream &operator<<(std::ostream &os, const gdt::math::vec2i &v) { 
    return os << "[ " << v.x << ", " << v.y << " ]";
}

std::ostream &operator<<(std::ostream &os, const gdt::math::vec3 &v) { 
    return os << "[ " << v.x << ", " << v.y << ", " << v.z << " ]";
}

std::ostream &operator<<(std::ostream &os, const gdt::math::vec4 &v) { 
    return os << "[ " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << " ]";
}
