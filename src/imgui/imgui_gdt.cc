#include "imgui.h"
namespace gdt {
void set_imgui_style()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.84f);
    style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.37f, 0.63f, 1.00f, 0.16f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.37f, 0.63f, 1.00f, 0.46f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.27f, 0.43f, 0.70f, 0.63f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.37f, 0.63f, 1.00f, 0.20f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.38f, 0.63f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.37f, 0.63f, 1.00f, 0.49f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.37f, 0.63f, 1.00f, 0.30f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.37f, 0.63f, 1.00f, 0.40f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.37f, 0.63f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_ComboBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.99f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.69f, 0.86f, 1.00f, 0.24f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.63f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.37f, 0.63f, 1.00f, 0.22f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.37f, 0.63f, 1.00f, 0.50f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.37f, 0.63f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(1.00f, 1.00f, 1.00f, 0.11f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.37f, 0.63f, 1.00f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.37f, 0.63f, 1.00f, 0.80f);
    style.Colors[ImGuiCol_Column] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.37f, 0.63f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.37f, 0.63f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_CloseButton] = ImVec4(0.37f, 0.63f, 1.00f, 0.50f);
    style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.37f, 0.63f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.36f, 0.73f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.36f, 0.73f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.36f, 0.73f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.36f, 0.73f, 1.00f, 0.53f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    style.WindowRounding = 0;
    style.ChildWindowRounding = 0;
    style.FrameRounding = 4;
    style.ScrollbarRounding = 0;
    style.GrabRounding = 3;
}
}
