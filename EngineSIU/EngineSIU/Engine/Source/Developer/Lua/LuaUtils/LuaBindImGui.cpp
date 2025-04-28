#include "LuaBindImGui.h"
#include "ImGui/imgui.h"

sol::table LuaBindImGui::Bind(sol::table& parentTable)
{
    sol::table ImGuiTable = parentTable.create_named("ImGui");

    ImGuiTable.set_function("SetNextWindowPos",   &LuaBindImGui::SetNextWindowPos);
    ImGuiTable.set_function("SetNextWindowSize",  &LuaBindImGui::SetNextWindowSize);
    ImGuiTable.set_function("Begin",              &LuaBindImGui::Begin);
    ImGuiTable.set_function("End",                &LuaBindImGui::End);
    ImGuiTable.set_function("Text",               &LuaBindImGui::Text);
    ImGuiTable.set_function("Separator",         &LuaBindImGui::Separator);
    
    return ImGuiTable;
}

void LuaBindImGui::SetNextWindowPos(float x, float y, sol::optional<int> cond_opt) {
    ImGuiCond cond = cond_opt.value_or(ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2{x, y}, cond);
}
void LuaBindImGui::SetNextWindowSize(float w, float h, sol::optional<int> cond_opt) {
    ImGuiCond cond = cond_opt.value_or(ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2{w, h}, cond);
}
void LuaBindImGui::Begin(const std::string& name) {
    ImGui::Begin(name.c_str());
}
void LuaBindImGui::End() {
    ImGui::End();
}
void LuaBindImGui::Text(const std::string& text) {
    ImGui::Text("%s", text.c_str());
}

void LuaBindImGui::Separator() {
    ImGui::Separator();
}
