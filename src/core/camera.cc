#include "camera.hh"

namespace gdt {

void camera::on_screen_resize(unsigned int width, unsigned int height)
{
    proj = math::mat4().perspective(_fov, _near, _far, (float)height / (float)width);
}

void camera::imgui()
{
    if (ImGui::CollapsingHeader("camera")) {
        ImGui::PushItemWidth(100);
        ImGui::SliderFloat("fov", &_fov, 0.01f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("near", &_near, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::SliderFloat("far", &_far, 0.0f, 3000.0f);
        ImGui::PopItemWidth();
        ImGui::InputFloat3("pos", &pos.x, -100.0f, 100.0f);
        ImGui::InputFloat3("tgt", &tgt.x, -100.0f, 100.0f);
    }
}

void camera2d::on_screen_resize(unsigned int width, unsigned int height)
{
    float scale = 4.0;
    proj = math::mat4().ortho(scale * -1, scale, scale * -1 * ((float)height / (float)width),
                              scale * ((float)height / (float)width), _near, _far);
}

void camera2dpp::on_screen_resize(unsigned int width, unsigned int height)
{
    this->_width = width;
    this->_height = height;
    proj = math::mat4().ortho((width / 2.0) * -1, width / 2.0, (height / 2.0) * -1,
                              height / 2.0, _near, _far);
}
}

