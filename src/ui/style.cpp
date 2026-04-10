#include "style.h"

void ApplyStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding     = ImVec2(12, 12);
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 8);
    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 4.0f;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg]         = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_Header]           = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBg]          = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.24f, 0.24f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBgActive]    = ImVec4(0.28f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_SliderGrab]       = ImVec4(0.28f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.75f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]           = ImVec4(0.20f, 0.45f, 0.75f, 1.00f);
    colors[ImGuiCol_ButtonHovered]    = ImVec4(0.28f, 0.55f, 0.90f, 1.00f);
    colors[ImGuiCol_ButtonActive]     = ImVec4(0.15f, 0.38f, 0.65f, 1.00f);
    colors[ImGuiCol_CheckMark]        = ImVec4(0.28f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_SeparatorActive]  = ImVec4(0.28f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_TitleBgActive]    = ImVec4(0.13f, 0.13f, 0.16f, 1.00f);
}