#pragma once

#include <sol/sol.hpp>

class LuaBindImGui
{
public:
    static sol::table Bind(sol::table& parentTable);

private:
    static void SetNextWindowPos(float x, float y, sol::optional<int> cond_opt);
    static void SetNextWindowSize(float w, float h, sol::optional<int> cond_opt);
    static void SetCenterPos();
    static void Begin(const std::string& name);
    static void End();
    static void Text(const std::string& text);
    static void IntText(int t);
    static void SameLine();
    static void Separator();
    static void Button(const std::string& name, float Width, float Height, const sol::function& Callback);
    static void Spacing();
    static void SetGameMode(const std::string& name);
};
