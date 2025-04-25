#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>


class FLuaManager
{
    FLuaManager() = default;
    ~FLuaManager() = default;

public:
    // 복사 & 이동 생성자 제거
    FLuaManager(const FLuaManager&) = delete;
    FLuaManager& operator=(const FLuaManager&) = delete;
    FLuaManager(FLuaManager&&) = delete;
    FLuaManager& operator=(FLuaManager&&) = delete;

public:
    static FLuaManager& Get();

    void Initialize();
    sol::state& GetLuaState();

private:
    bool bInitialized = false;
    sol::state LuaState;
};
