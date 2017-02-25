#ifndef LIGHT_HH__INCLUDED
#define LIGHT_HH__INCLUDED

#include "math.hh"
#include "imgui/imgui.h"

namespace gdt {

struct light {
    math::vec3 pos;
    math::vec3 color;
    float linear;
    float quadric;
    light() {}
    light(math::vec3 p, math::vec3 c, float l, float q) :
      pos(p), color(c), linear(l), quadric(q) {}
    static light randomize(float x, float y, float z, float linear = 0.001,
                           float quadric = 0.0002)
    {
        light l;
        l.pos = math::vec3::random(x, y, z);
        l.color = math::vec3::random(0.5, 0.5, 0.5) + math::vec3(0.5, 0.5, 0.5);
        l.linear = linear;
        l.quadric = quadric;
        return l;
    }
    static light off(float x, float y, float z, float linear = 0.001,
                           float quadric = 0.0002)
    {
        light l;
        l.pos = math::vec3::random(x, y, z);
        l.linear = linear;
        l.quadric = quadric;
        return l;
    }

    void imgui() {
        ImGui::SliderFloat3("##pos", &pos.x, -1000.0f, 1000.0f);
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::SliderFloat("##linear", &linear, -0.1f, 0.1, "%0.5f");
        ImGui::SameLine();
        ImGui::SliderFloat("##quadric", &quadric, 0.0f, 0.001, "%0.6f");
        ImGui::PopItemWidth();
        ImGui::ColorEdit3("##color",&color.x);
    }
};
}

#endif  // LIGHT_HH__INCLUDED
