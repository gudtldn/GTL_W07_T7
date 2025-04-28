#include "LuaManager.h"

#include "LuaActor.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "LevelEditor/SLevelEditor.h"
#include "LuaTypes/LuaUserTypes.h"
#include "UserInterface/Console.h"
#include "LuaUtils/LuaBindImGui.h"
#include "UnrealEd/EditorViewportClient.h"
#include "World/World.h"

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

void FLuaManager::RegisterActor(ALuaActor* Actor, const std::filesystem::path& ScriptPath)
{
    ActorRegistry.FindOrAdd(ScriptPath).Add(Actor);
}

void FLuaManager::DeregisterActor(ALuaActor* Actor, const std::filesystem::path& ScriptPath)
{
    if (TSet<ALuaActor*>* Actors = ActorRegistry.Find(ScriptPath))
    {
        Actors->Remove(Actor);
        if (Actors->Num() == 0)
        {
            ActorRegistry.Remove(ScriptPath);
        }
    }
}

sol::protected_function FLuaManager::GetActorFactory(const std::filesystem::path& ScriptPath)
{
    LuaScriptData& Data = LoadOrGetScriptData(ScriptPath);
    if (Data.LoadSucceeded)
    {
        return Data.FactoryFunction;
    }
    return sol::lua_nil; // 실패 시 nil 반환
}

void FLuaManager::CheckForScriptChanges()
{
    TArray<std::filesystem::path> PathsToReload;
    // for (const auto& [Path, ScriptData] : ScriptCache)
    for (const auto& Cache : ScriptCache)
    {
        // if (!ScriptData.LoadSucceeded) continue;

        try
        {
            if (fs::exists(Cache.Key))
            {
                fs::file_time_type CurrentWriteTime = fs::last_write_time(Cache.Key);
                if (CurrentWriteTime > Cache.Value.LastWriteTime)
                {
                    PathsToReload.Add(Cache.Key); // 변경된 경로 기록
                }
            }
            else
            {
                // 파일이 사라짐 - 캐시 무효화 또는 다른 처리?
                // ScriptCache.erase(pathStr); // 주의: 순회 중 삭제 문제
            }
        }
        catch (const fs::filesystem_error& Error)
        {
            UE_LOG(ELogLevel::Error, "[LuaManager] Error checking script file: %s (%s)", Cache.Key.generic_string().c_str(), Error.what());
        }
    }

    // LuaActor에게 리로드 알림
    for (const auto& Path : PathsToReload)
    {
        TriggerReloadForPath(Path);
    }

    // ImGui
    if (fs::exists(ImGuiScriptPath))
    {
        fs::file_time_type CurrentWriteTime = fs::last_write_time(ImGuiScriptPath);
        if (CurrentWriteTime > ImGuiScriptLastWriteTime)
        {
            ImGuiScriptLastWriteTime = CurrentWriteTime;
            LoadImGuiScript();
            UE_LOG(ELogLevel::Display, "[LuaManager] Reloaded ImGui script");
        }
    }
}

void FLuaManager::ForceReloadScript(const std::filesystem::path& ScriptPath)
{
    TriggerReloadForPath(ScriptPath);
}

void FLuaManager::NotifyScriptChanged(const std::filesystem::path& ScriptPath)
{
    // 이벤트 기반 파일 감시 시스템에서 이 함수를 호출하도록 연결
    // 메인 스레드에서 안전하게 TriggerReloadForPath를 호출하도록 큐잉(queuing) 필요할 수 있음
    TriggerReloadForPath(ScriptPath);
}

LuaScriptData& FLuaManager::LoadOrGetScriptData(const std::filesystem::path& Path)
{
    if (LuaScriptData* Data = ScriptCache.Find(Path))
    {
        return *Data;
    }

    LuaScriptData& Data = ScriptCache.Emplace(Path);
    Data.LoadAttempted = true;
    Data.LoadSucceeded = false;
    Data.FactoryFunction = sol::lua_nil;

    if (!fs::exists(Path))
    {
        UE_LOG(ELogLevel::Error, "[LuaManager] Lua script file not found: %s", Path.generic_string().c_str());
        return Data;
    }

    const std::string StringPath = Path.generic_string();
    try
    {
        Data.LastWriteTime = fs::last_write_time(Path);
        sol::protected_function_result Result = LuaState.script_file(StringPath);

        if (Result.valid())
        {
            if (Result.get_type() == sol::type::function)
            {
                Data.FactoryFunction = Result;
                Data.LoadSucceeded = true;
                UE_LOG(ELogLevel::Display, "[LuaManager] Lua script loaded: %s", StringPath.c_str());
            }
            else if (Result.get_type() == sol::type::table)
            {
                // 스크립트가 클래스 테이블을 직접 반환하는 경우 (다른 방식)
                // data.ClassTable = result;
                // data.FactoryFunction = data.ClassTable["new"]; // 예시: new 함수를 팩토리로 사용
                // if(data.FactoryFunction.valid()) data.LoadSucceeded = true;
                // else { std::cerr << ... "Script table has no 'new' function" << std::endl; }
                Data.LoadSucceeded = false; // 임시: 팩토리 함수 방식만 지원
                UE_LOG(ELogLevel::Display, "[LuaManager] Script did not return a function (factory expected): %s", StringPath.c_str());
            }
            else
            {
                UE_LOG(ELogLevel::Error, "[LuaManager] Script did not return a function: %s", StringPath.c_str());
            }
        }
        else
        {
            const sol::error Error = Result;
            UE_LOG(ELogLevel::Error, "[LuaManager] Failed to execute script: %s (%s)", StringPath.c_str(), Error.what());
        }
    }
    catch (const std::exception& Error)
    {
        UE_LOG(ELogLevel::Error, "[LuaManager] Exception loading script: %s (%s)", StringPath.c_str(), Error.what());
    }

    return Data;
}

void FLuaManager::TriggerReloadForPath(const std::filesystem::path& Path)
{
    TArray<ALuaActor*> ActorsToNotify;

    // 스크립트 리로드
    ScriptCache.Remove(Path);
    LuaScriptData ReloadedData = LoadOrGetScriptData(Path); // 여기서 ScriptCache에 다시 들어감

    if (const TSet<ALuaActor*>* Reg = ActorRegistry.Find(Path))
    {
        ActorsToNotify = Reg->Array();
    }

    // 성공적으로 불러왔다면 Actor들에게 알림
    if (ReloadedData.LoadSucceeded)
    {
        for (ALuaActor* Actor : ActorsToNotify)
        {
            Actor->HandleScriptReload(ReloadedData.FactoryFunction);
        }
        UE_LOG(ELogLevel::Display, "[LuaManager] Reload notification sent for: %s", Path.generic_string().c_str());
    }
    else
    {
        UE_LOG(ELogLevel::Error, "[LuaManager] Reload failed for: %s, actors not notified.", Path.generic_string().c_str());
    }
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

    sol::table Ns = LuaState.create_named_table("EngineSIU");

    // Globals
    Ns["SpawnActor"] = [](const std::string& ClassName)
    {
        if (UClass** Class = UClass::GetClassMap().Find(ClassName.c_str()))
        {
            return GEngine->ActiveWorld->SpawnActor(*Class);
        }
        UE_LOG(ELogLevel::Error, "[LuaManager] Class not found: '%s'", ClassName.c_str());
        return static_cast<AActor*>(nullptr);
    };

    Ns["LOG_ERROR"] = [](const std::string& Message)
    {
        UE_LOG(ELogLevel::Error, "[LuaScript] %s", Message.c_str());
    };

    Ns["LOG_WARNING"] = [](const std::string& Message)
    {
        UE_LOG(ELogLevel::Warning, "[LuaScript] %s", Message.c_str());
    };

    Ns["LOG_INFO"] = [](const std::string& Message)
    {
        UE_LOG(ELogLevel::Display, "[LuaScript] %s", Message.c_str());
    };

    FViewportCamera& Camera = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->PerspectiveCamera;
    Ns["GetCameraLocation"] = [&Camera]()
    {
        return Camera.GetLocation();
    };

    Ns["SetCameraLocation"] = [&Camera](const FVector& Location)
    {
        Camera.SetLocation(Location);
    };

    Ns["GetCameraRotation"] = [&Camera]()
    {
        return Camera.GetRotation();
    };

    Ns["SetCameraRotation"] = [&Camera](const FRotator& Rotation)
    {
        Camera.SetRotation(Rotation.ToVector());
    };

    Ns["GetCameraForwardVector"] = [&Camera]()
    {
        return Camera.GetForwardVector();
    };

    Ns["GetCameraRightVector"] = [&Camera]()
    {
        return Camera.GetRightVector();
    };

    Ns["GetCameraUpVector"] = [&Camera]()
    {
        return Camera.GetUpVector();
    };

    // Math Types
    LuaTypes::FBindLua<FColor>::Bind(Ns);
    LuaTypes::FBindLua<FLinearColor>::Bind(Ns);
    LuaTypes::FBindLua<FVector>::Bind(Ns);
    LuaTypes::FBindLua<FVector2D>::Bind(Ns);
    LuaTypes::FBindLua<FVector4>::Bind(Ns);
    LuaTypes::FBindLua<FRotator>::Bind(Ns);
    LuaTypes::FBindLua<FQuat>::Bind(Ns);
    LuaTypes::FBindLua<FMatrix>::Bind(Ns);

    // Object Types
    LuaTypes::FBindLua<ALuaActor>::Bind(Ns);
    LuaTypes::FBindLua<ALuaPlayer>::Bind(Ns);
    LuaTypes::FBindLua<ALuaCoachActor>::Bind(Ns);

    // ImGui
    sol::table ImGuiTable = LuaBindImGui::Bind(Ns);
    LuaState["ImGui"] = ImGuiTable;
    
    bInitialized = true;

    ImGuiScriptPath = fs::current_path().parent_path() / "GameJam" / "Lua" / "LuaImGui.lua";
    LoadImGuiScript();
}

void FLuaManager::LoadImGuiScript()
{
    if (!fs::exists(ImGuiScriptPath)) {
        UE_LOG(ELogLevel::Error,
               "Lua script not found: %s",
               ImGuiScriptPath.generic_string().c_str());
        return;
    }

    LuaState["EngineSIU"]["LuaImGui"] = sol::lua_nil;
    
    sol::protected_function_result result =
        LuaState.script_file(ImGuiScriptPath.generic_string());

    if (!result.valid()) {
        sol::error err = result;
        UE_LOG(ELogLevel::Error,
               "Failed to load %s: %s",
               ImGuiScriptPath.generic_string().c_str(),
               err.what());
    }
}

void FLuaManager::RenderImGuiFromLua()
{
    sol::protected_function RenderFunc = LuaState["EngineSIU"]["LuaImGui"];
    if (RenderFunc.valid())
    {
        sol::protected_function_result Result = RenderFunc();
        if (!Result.valid())
        {
            const sol::error Error = Result;
            UE_LOG(ELogLevel::Error, "[LuaManager] LuaImGui error: %s", Error.what());
        }
    }
}
