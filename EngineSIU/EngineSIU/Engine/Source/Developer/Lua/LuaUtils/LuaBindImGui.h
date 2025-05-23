#pragma once

#include <sol/sol.hpp>

class LuaBindImGui
{
public:
    static sol::table Bind(sol::table& parentTable);

private:
    static void SetNextWindowPos(float x, float y, sol::optional<int> cond_opt);
    static void SetNextWindowSize(float w, float h, sol::optional<int> cond_opt);
    static void Begin(const std::string& name);
    static void End();
    static void Text(const std::string& text);
    static void Separator();
};
