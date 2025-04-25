#include "LuaManager.h"
#include "LuaTypes/LuaUserTypes.h"


FLuaManager& FLuaManager::Get()
{
    static FLuaManager Instance;
    return Instance;
}

sol::state& FLuaManager::GetLuaState()
{
    assert(bInitialized && "LuaManager is not initialized.");
    return LuaState;
}

void FLuaManager::Initialize()
{
    if (bInitialized) return;

    LuaState.open_libraries(
        sol::lib::base,       // Lua를 사용하기 위한 기본 라이브러리 (print, type, pcall 등)
        // sol::lib::package,    // 모듈 로딩(require) 및 패키지 관리 기능
        sol::lib::coroutine,  // 코루틴 생성 및 관리 기능 (yield, resume 등)
        sol::lib::string,     // 문자열 검색, 치환, 포맷팅 등 문자열 처리 기능
        sol::lib::math,       // 수학 함수 (sin, random, pi 등) 및 상수 제공
        sol::lib::table,      // 테이블(배열/딕셔너리) 생성 및 조작 기능 (insert, sort 등)
        // sol::lib::io,         // 파일 읽기/쓰기 등 입출력 관련 기능
        // sol::lib::os,         // 운영체제 관련 기능 (시간, 날짜, 파일 시스템 접근 등)
        sol::lib::debug,      // 디버깅 및 introspection 기능 (traceback, getinfo 등)
        sol::lib::bit32,      // 32비트 정수 대상 비트 연산 기능 (Lua 5.2 이상)
        // sol::lib::jit,        // LuaJIT의 JIT 컴파일러 제어 기능 (LuaJIT 전용)
        // sol::lib::ffi,        // 외부 C 함수 및 데이터 구조 접근 기능 (LuaJIT 전용)
        sol::lib::utf8        // UTF-8 인코딩 문자열 처리 기능 (Lua 5.3 이상)
    );

    sol::table Ns = LuaState.create_named_table("SIUEngine");

    LuaTypes::FBindLua<FColor>::Bind(Ns);
    LuaTypes::FBindLua<FLinearColor>::Bind(Ns);
    LuaTypes::FBindLua<FVector>::Bind(Ns);
    LuaTypes::FBindLua<FVector2D>::Bind(Ns);
    LuaTypes::FBindLua<FVector4>::Bind(Ns);
    LuaTypes::FBindLua<FRotator>::Bind(Ns);
    LuaTypes::FBindLua<FQuat>::Bind(Ns);
    LuaTypes::FBindLua<FMatrix>::Bind(Ns);

    bInitialized = true;
}
