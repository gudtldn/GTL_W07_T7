#include "Lua.h"

FLuaManager& FLuaManager::Get()
{
    static FLuaManager Instance;
    return Instance;
}
