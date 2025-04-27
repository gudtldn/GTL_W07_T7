#pragma once
#include "LuaBindUtils.h"

/**
 * Lua에 새로운 UserType을 추가합니다.
 * @param TypeName lua에서 사용될 커스텀 타입
 * @note Ex) sol::state Lua; Lua.Lua_New_UserType(FTestStruct, ...);
 */
#define Lua_NewUserType(TypeName, ...) new_usertype<TypeName>(#TypeName, __VA_ARGS__)

// TODO: 참고: https://www.perplexity.ai/search/sol-varyi-sayongbeob-IrR1v4nCQXyMo8ebRmGzxA

#define LUA_BIND_MEMBER(MemberName) LuaBindUtils::GetMemberName(#MemberName, true), MemberName

#define LUA_BIND_STATIC(MemberName) LuaBindUtils::GetMemberName(#MemberName, false), sol::var(MemberName)
