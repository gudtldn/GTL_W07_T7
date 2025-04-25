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

void BindFColorToLua(sol::state& Lua);
void BindFLinearColorToLua(sol::state& Lua);
void BindFVectorToLua(sol::state& Lua);
void BindFVector2DToLua(sol::state& Lua);
void BindFVector4ToLua(sol::state& Lua);
void BindFRotatorToLua(sol::state& Lua);
void BindFQuatToLua(sol::state& Lua);
void BindFMatrixToLua(sol::state& Lua);
}
