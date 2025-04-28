#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <filesystem>

#include "Container/Map.h"
#include "Container/Set.h"
namespace fs = std::filesystem;


class ALuaActor;

/** 스크립트 로드 결과 및 관련 정보를 담는 구조체 */
struct LuaScriptData
{
    // 스크립트가 반환하는 팩토리 함수 (테이블 생성용)
    sol::protected_function FactoryFunction = sol::lua_nil;

    // 파일이 마지막으로 수정된 시각
    std::filesystem::file_time_type LastWriteTime;

    // 스크립트 경로에 대해 로드를 시도했는지 여부
    bool LoadAttempted = false;

    // 스크립트가 완전히 로드 되었는지 여부
    bool LoadSucceeded = false;

    // 필요하다면 컴파일된 청크(chunk) 저장 등 최적화 가능
};

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

    // --- 액터 등록/해제 ---
    /** 액터가 특정 스크립트를 사용한다고 매니저에 등록합니다. */
    void RegisterActor(ALuaActor* Actor, const std::filesystem::path& ScriptPath);

    /** 액터가 더 이상 스크립트를 사용하지 않을 때 매니저에서 등록 해제합니다. */
    void DeregisterActor(ALuaActor* Actor, const std::filesystem::path& ScriptPath);

    // --- 스크립트 로드/제공 ---
    /** 지정된 경로의 스크립트를 로드하고 액터 인스턴스를 생성할 팩토리 함수를 반환합니다. */
    sol::protected_function GetActorFactory(const std::filesystem::path& ScriptPath);

    // --- 핫 리로드 관련 ---
    /** 매 프레임 호출되어 파일 변경 사항을 감지하고 리로드를 트리거합니다. (폴링 방식) */
    void CheckForScriptChanges();

    /** 특정 스크립트 경로에 대해 수동으로 리로드를 트리거합니다. */
    void ForceReloadScript(const std::filesystem::path& ScriptPath);

    // 파일 시스템 감시 콜백 등에서 호출될 수 있음 (이벤트 기반 방식)
    [[maybe_unused]]
    void NotifyScriptChanged(const std::filesystem::path& ScriptPath);

    // --- ImGui ---
    void RenderImGuiFromLua();
private:
    // 내부 헬퍼 함수
    LuaScriptData& LoadOrGetScriptData(const std::filesystem::path& Path);
    void TriggerReloadForPath(const std::filesystem::path& Path);
    void LoadImGuiScript();
    
private:
    bool bInitialized = false;
    sol::state LuaState;
    fs::path ImGuiScriptPath;

    TMap<std::filesystem::path, LuaScriptData> ScriptCache;
    std::filesystem::file_time_type ImGuiScriptLastWriteTime = std::filesystem::file_time_type::min();;
    TMap<std::filesystem::path, TSet<ALuaActor*>> ActorRegistry;
};
