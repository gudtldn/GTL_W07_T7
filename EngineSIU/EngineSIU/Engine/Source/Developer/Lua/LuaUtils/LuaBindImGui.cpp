#include "LuaBindImGui.h"

#include "EngineLoop.h"
#include "Developer/FMOD/SoundManager.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "ImGui/imgui.h"

sol::table LuaBindImGui::Bind(sol::table& parentTable)
{
    sol::table ImGuiTable = parentTable.create_named("ImGui");

    ImGuiTable.set_function("SetNextWindowPos",   &LuaBindImGui::SetNextWindowPos);
    ImGuiTable.set_function("SetNextWindowSize",  &LuaBindImGui::SetNextWindowSize);
    ImGuiTable.set_function("SetCenterPos",  &LuaBindImGui::SetCenterPos);
    ImGuiTable.set_function("Begin",              &LuaBindImGui::Begin);
    ImGuiTable.set_function("End",                &LuaBindImGui::End);
    ImGuiTable.set_function("Text",               &LuaBindImGui::Text);
    ImGuiTable.set_function("IntText",               &LuaBindImGui::IntText);
    ImGuiTable.set_function("Separator",         &LuaBindImGui::Separator);
    ImGuiTable.set_function("Button",            &LuaBindImGui::Button);
    ImGuiTable.set_function("Spacing",            &LuaBindImGui::Spacing);
    ImGuiTable.set_function("SameLine",             &LuaBindImGui::SameLine);
    ImGuiTable.set_function("SetGameMode",      &LuaBindImGui::SetGameMode);
    
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

void LuaBindImGui::SetCenterPos()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f));
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

void LuaBindImGui::IntText(int t)
{
    ImGui::Text("%d", t);
}

void LuaBindImGui::SameLine()
{
    ImGui::SameLine();
}

void LuaBindImGui::Separator() {
    ImGui::Separator();
}

void LuaBindImGui::Button(const std::string& name, float Width, float Height, const sol::function& Callback)
{
    if (ImGui::Button(name.c_str(), ImVec2(Width, Height)))
    {
        if (Callback.valid())
        {
            (void)Callback();
        }
    }
}

void LuaBindImGui::Spacing()
{
    ImGui::Spacing();
}

void LuaBindImGui::SetGameMode(const std::string& name)
{
    if (name == "Lobby")
    {
        FEngineLoop::GameState = Lobby;
        if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
        {
            EditorEngine->EndPIE();
            FSoundManager::Get()->StopSound("Contents\\Sound\\game.mp3");
            FSoundManager::Get()->MainChannel = FSoundManager::Get()->PlaySound("Contents\\Sound\\lobby.mp3", true);
        }
    }
    else
    {
        FEngineLoop::GameState = None;
    }
}
