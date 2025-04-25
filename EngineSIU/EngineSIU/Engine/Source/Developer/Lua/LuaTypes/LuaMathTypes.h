#pragma once
#include "sol/sol.hpp"

namespace LuaTypes::Math
{
// template <typename T>
// struct FBindLua
// {
//     static void BindToLua(sol::state& Lua)
//     {
//         static_assert([]{ return false; }(), "not implemented");
//     }
// };

void BindFColorToLua(sol::table& Table);
void BindFLinearColorToLua(sol::table& Table);
void BindFVectorToLua(sol::table& Table);
void BindFVector2DToLua(sol::table& Table);
void BindFVector4ToLua(sol::table& Table);
void BindFRotatorToLua(sol::table & Table);
void BindFQuatToLua(sol::table& Table);
void BindFMatrixToLua(sol::table& Table);
}
