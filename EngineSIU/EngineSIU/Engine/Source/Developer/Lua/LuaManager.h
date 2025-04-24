#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol2/sol.hpp>


class FLuaManager
{
    FLuaManager();

public:
    static FLuaManager& Get();

private:
    sol::state LuaState;
};
