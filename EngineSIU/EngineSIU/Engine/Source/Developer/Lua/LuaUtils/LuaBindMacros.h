#pragma once
#include "LuaBindUtils.h"

/**
 * Lua에 새로운 UserType을 추가합니다.
 * @param TypeName lua에서 사용될 커스텀 타입
 * @note Ex) sol::state Lua; Lua.Lua_New_UserType(FTestStruct, ...);
 */
#define Lua_NewUserType(TypeName, ...) new_usertype<TypeName>(#TypeName, __VA_ARGS__)

// TODO: 참고: https://www.perplexity.ai/search/sol-varyi-sayongbeob-IrR1v4nCQXyMo8ebRmGzxA

#define LUA_BIND_CONSTRUCTORS(...) \
    sol::call_constructor, \
    sol::constructors<__VA_ARGS__>()

#define LUA_BIND_MEMBER(MemberName) \
    LuaBindUtils::GetMemberName(#MemberName, true), MemberName

#define LUA_BIND_FUNC(MemberName) LUA_BIND_MEMBER(MemberName)

#define LUA_BIND_STATIC(MemberName) \
    LuaBindUtils::GetMemberName(#MemberName), sol::var(MemberName)

#define LUA_BIND_OVERLOAD_WITHOUT_NAME2(MemberFunc, Type1, Type2)                                           sol::overload(sol::resolve<Type1>(MemberFunc), sol::resolve<Type2>(MemberFunc))
#define LUA_BIND_OVERLOAD_WITHOUT_NAME3(MemberFunc, Type1, Type2, Type3)                                    sol::overload(sol::resolve<Type1>(MemberFunc), sol::resolve<Type2>(MemberFunc), sol::resolve<Type3>(MemberFunc))
#define LUA_BIND_OVERLOAD_WITHOUT_NAME4(MemberFunc, Type1, Type2, Type3, Type4)                             sol::overload(sol::resolve<Type1>(MemberFunc), sol::resolve<Type2>(MemberFunc), sol::resolve<Type3>(MemberFunc), sol::resolve<Type4>(MemberFunc))
#define LUA_BIND_OVERLOAD_WITHOUT_NAME5(MemberFunc, Type1, Type2, Type3, Type4, Type5)                      sol::overload(sol::resolve<Type1>(MemberFunc), sol::resolve<Type2>(MemberFunc), sol::resolve<Type3>(MemberFunc), sol::resolve<Type4>(MemberFunc), sol::resolve<Type5>(MemberFunc))
#define LUA_BIND_OVERLOAD_WITHOUT_NAME6(MemberFunc, Type1, Type2, Type3, Type4, Type5, Type6)               sol::overload(sol::resolve<Type1>(MemberFunc), sol::resolve<Type2>(MemberFunc), sol::resolve<Type3>(MemberFunc), sol::resolve<Type4>(MemberFunc), sol::resolve<Type5>(MemberFunc), sol::resolve<Type6>(MemberFunc))
#define LUA_BIND_OVERLOAD_WITHOUT_NAME7(MemberFunc, Type1, Type2, Type3, Type4, Type5, Type6, Type7)        sol::overload(sol::resolve<Type1>(MemberFunc), sol::resolve<Type2>(MemberFunc), sol::resolve<Type3>(MemberFunc), sol::resolve<Type4>(MemberFunc), sol::resolve<Type5>(MemberFunc), sol::resolve<Type6>(MemberFunc), sol::resolve<Type7>(MemberFunc))
#define LUA_BIND_OVERLOAD_WITHOUT_NAME8(MemberFunc, Type1, Type2, Type3, Type4, Type5, Type6, Type7, Type8) sol::overload(sol::resolve<Type1>(MemberFunc), sol::resolve<Type2>(MemberFunc), sol::resolve<Type3>(MemberFunc), sol::resolve<Type4>(MemberFunc), sol::resolve<Type5>(MemberFunc), sol::resolve<Type6>(MemberFunc), sol::resolve<Type7>(MemberFunc), sol::resolve<Type8>(MemberFunc))

#define LUA_BIND_OVERLOAD2(MemberFunc, Type1, Type2)                                           LuaBindUtils::GetMemberName(#MemberFunc), LUA_BIND_OVERLOAD_WITHOUT_NAME2(MemberFunc, Type1, Type2)
#define LUA_BIND_OVERLOAD3(MemberFunc, Type1, Type2, Type3)                                    LuaBindUtils::GetMemberName(#MemberFunc), LUA_BIND_OVERLOAD_WITHOUT_NAME3(MemberFunc, Type1, Type2, Type3)
#define LUA_BIND_OVERLOAD4(MemberFunc, Type1, Type2, Type3, Type4)                             LuaBindUtils::GetMemberName(#MemberFunc), LUA_BIND_OVERLOAD_WITHOUT_NAME4(MemberFunc, Type1, Type2, Type3, Type4)
#define LUA_BIND_OVERLOAD5(MemberFunc, Type1, Type2, Type3, Type4, Type5)                      LuaBindUtils::GetMemberName(#MemberFunc), LUA_BIND_OVERLOAD_WITHOUT_NAME5(MemberFunc, Type1, Type2, Type3, Type4, Type5)
#define LUA_BIND_OVERLOAD6(MemberFunc, Type1, Type2, Type3, Type4, Type5, Type6)               LuaBindUtils::GetMemberName(#MemberFunc), LUA_BIND_OVERLOAD_WITHOUT_NAME6(MemberFunc, Type1, Type2, Type3, Type4, Type5, Type6)
#define LUA_BIND_OVERLOAD7(MemberFunc, Type1, Type2, Type3, Type4, Type5, Type6, Type7)        LuaBindUtils::GetMemberName(#MemberFunc), LUA_BIND_OVERLOAD_WITHOUT_NAME7(MemberFunc, Type1, Type2, Type3, Type4, Type5, Type6, Type7)
#define LUA_BIND_OVERLOAD8(MemberFunc, Type1, Type2, Type3, Type4, Type5, Type6, Type7, Type8) LuaBindUtils::GetMemberName(#MemberFunc), LUA_BIND_OVERLOAD_WITHOUT_NAME8(MemberFunc, Type1, Type2, Type3, Type4, Type5, Type6, Type7, Type8)
