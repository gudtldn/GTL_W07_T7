#pragma once
#include "LuaBindUtils.h"

#define LUA_BIND_MEMBER_VAR(MemberName) LuaBindUtils::GetMemberName(#MemberName), MemberName
