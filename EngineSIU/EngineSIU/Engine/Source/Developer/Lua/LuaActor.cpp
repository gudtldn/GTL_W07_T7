#include "LuaActor.h"

#include "LuaManager.h"
#include "Engine/Classes/Components/PrimitiveComponent.h"
#include "Engine/Source/Runtime/Core/Delegates/Delegate.h"

ALuaActor::ALuaActor()
    : LuaScriptPath(std::nullopt)
{
}

UObject* ALuaActor::Duplicate(UObject* InOuter)
{
    if (ThisClass* NewClass = Cast<ThisClass>(Super::Duplicate(InOuter)))
    {
        NewClass->SetScriptPath(LuaScriptPath);
        return NewClass;
    }
    return nullptr;
}

void ALuaActor::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);

    namespace fs = std::filesystem;
    const fs::path SolutionPath = fs::current_path().parent_path();
    const fs::path LuaFolderPath = SolutionPath / "GameJam/Lua";
    OutProperties["LuaScriptPath"] = LuaScriptPath.value_or("").lexically_relative(LuaFolderPath).generic_string();
}

void ALuaActor::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    namespace fs = std::filesystem;
    const fs::path SolutionPath = fs::current_path().parent_path();
    const fs::path LuaFolderPath = SolutionPath / "GameJam/Lua";
    const fs::path SavedPath = fs::path(InProperties["LuaScriptPath"].ToWideString());
    LuaScriptPath = SavedPath.empty()
        ? std::nullopt
        : std::optional(LuaFolderPath / SavedPath);
}

void ALuaActor::BeginPlay()
{
    Super::BeginPlay();

    (void)CallLuaFunction("BeginPlay");

    for (UActorComponent* iter : GetComponents()) 
    {
        if (iter == GetRootComponent()) 
        {
            continue;
        }

        if (UPrimitiveComponent* prim = Cast<UPrimitiveComponent>(iter)) 
        {
            prim->OnComponentBeginOverlap.AddUObject(this, &ALuaActor::OnOverlap);
        }
    }
}

void ALuaActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    (void)CallLuaFunction("Tick", DeltaTime);
}

void ALuaActor::Destroyed()
{
    (void)CallLuaFunction("Destroyed");

    Super::Destroyed();
}

void ALuaActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    (void)CallLuaFunction("EndPlay", EEndPlayReason::ToString(EndPlayReason));
    CleanupLuaState();

    if (LuaScriptPath.has_value())
    {
        FLuaManager::Get().DeregisterActor(this, *LuaScriptPath);
    }

    Super::EndPlay(EndPlayReason);
}

void ALuaActor::OnOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    sol::object LuaArg = sol::lua_nil;
    ALuaActor* OtherLuaActor = Cast<ALuaActor>(OtherActor);
    if (OtherLuaActor && OtherLuaActor->SelfTable.valid())
    {
        // 캐스팅 성공 및 상대방의 SelfTable이 유효하면 그것을 전달
        LuaArg = OtherLuaActor->SelfTable;
    }

    CallLuaFunction("OnOverlap", LuaArg);
}

void ALuaActor::HandleScriptReload(const sol::protected_function& NewFactory)
{
    if (!NewFactory.valid())
    {
        UE_LOG(ELogLevel::Error, "ALuaActor [%s]: Received invalid factory on reload.", *GetName());
        return;
    }

    // 1. 현재 상태 백업 (Lua에 요청)
    // sol::object OldStateData = sol::lua_nil;
    // if (SelfTable.valid())
    // {
    //     sol::protected_function GetStateFunc = SelfTable["GetPersistentState"]; // 규칙 기반
    //     if (GetStateFunc.valid())
    //     {
    //         sol::protected_function_result StateResult = GetStateFunc(SelfTable);
    //         if (StateResult.valid())
    //         {
    //             OldStateData = StateResult;
    //         }
    //         else
    //         {
    //             /* 오류 로깅 */
    //         }
    //     }
    // }

    // 2. 새 팩토리 함수로 새 SelfTable 생성
    sol::table NewSelfTable = sol::lua_nil;
    sol::protected_function_result FactoryResult = NewFactory(this); // 액터 자신 전달
    if (FactoryResult.valid() && FactoryResult.get_type() == sol::type::table)
    {
        NewSelfTable = FactoryResult;
        NewSelfTable["cpp_actor"] = this; // cpp_actor 참조 설정
    }
    else
    {
        // LOG_ERROR("ALuaActor [%s]: Failed to create new instance from reloaded factory.", *GetName());
        // 리로드 중단 또는 다른 처리
        return;
    }

    // 3. 새 테이블에 이전 상태 전달 (OnReload 호출)
    // sol::protected_function OnReloadFunc = NewSelfTable["OnReload"]; // 규칙 기반
    // if (OnReloadFunc.valid())
    // {
    //     sol::protected_function_result ReloadCallResult = OnReloadFunc(NewSelfTable, OldStateData); // self, oldState 전달
    //     if (!ReloadCallResult.valid())
    //     {
    //         // OnReload 함수 실행 중 오류 발생
    //         sol::error Err = ReloadCallResult;
    //         // LOG_ERROR("ALuaActor [%s]: Error executing OnReload in new script: %s", *GetName(), Err.what());
    //         // 문제가 심각하면 리로드를 취소하고 이전 테이블로 복구할 수도 있음
    //         // NewSelfTable = sol::lua_nil; // 예: 롤백 결정
    //     }
    // }
    // else
    // {
    //     // OnReload 함수가 없음 (경고 로깅)
    //     // LOG_WARNING("ALuaActor [%s]: New script does not have OnReload function.", *GetName());
    // }

    // 4. SelfTable 교체 (성공적으로 OnReload까지 마쳤다면)
    if (NewSelfTable.valid())
    {
        // OnReload 실패 시 롤백했다면 nil일 수 있음
        SelfTable = NewSelfTable;
        // LOG_INFO("ALuaActor [%s]: Successfully reloaded script.", *GetName());
    }
    else
    {
        // LOG_WARNING("ALuaActor [%s]: Script reload aborted after OnReload failure.", *GetName());
    }
}

std::optional<std::filesystem::path> ALuaActor::GetScriptPath() const
{
    return LuaScriptPath;
}

void ALuaActor::SetScriptPath(const std::optional<std::filesystem::path>& Path)
{
    // 이전 스크립트 경로가 있었다면 등록 해제
    if (LuaScriptPath.has_value() && !LuaScriptPath->empty())
    {
        // 기존 경로와 같다면 무시
        if (Path.has_value() && LuaScriptPath == Path)
        {
            return;
        }

        if (HasActorBegunPlay())
        {
            // 이미 플레이 중이었다면
            FLuaManager::Get().DeregisterActor(this, *LuaScriptPath);
        }
        CleanupLuaState(); // 이전 상태 정리
    }

    LuaScriptPath = Path;

    // 새 경로가 유효하다면 새로 등록 및 로드
    if (LuaScriptPath.has_value() && !LuaScriptPath->empty())
    {
        FLuaManager::Get().RegisterActor(this, *LuaScriptPath);
        sol::protected_function Factory = FLuaManager::Get().GetActorFactory(*LuaScriptPath);
        if (Factory.valid())
        {
            // ... (BeginPlay와 유사하게 SelfTable 생성 및 BeginPlay 호출) ...
            sol::protected_function_result Result = Factory(this);
            if (Result.valid() && Result.get_type() == sol::type::table)
            {
                SelfTable = Result;
                SelfTable["cpp_actor"] = this;
                // (void)CallLuaFunction("BeginPlay"); // 경로 변경 시에도 BeginPlay 호출
            }
            else
            {
                // Log an error if the factory function result is invalid or not a table
                UE_LOG(ELogLevel::Error, "Lua factory function for script '%s' returned an invalid result or non-table type.", LuaScriptPath.value().generic_string().c_str());
                SelfTable = sol::lua_nil;
            }
        }
        else
        {
            // Log an error if the factory function itself is invalid
            UE_LOG(ELogLevel::Error, "Lua factory function for script '%s' is invalid.", LuaScriptPath.value().generic_string().c_str());
            SelfTable = sol::lua_nil;
        }
    }
}

void ALuaActor::CleanupLuaState()
{
    if (SelfTable.valid())
    {
        // C++ 포인터 참조 제거 (가비지 컬렉션 도움)
        SelfTable["cpp_actor"] = sol::lua_nil;
    }
    SelfTable = sol::lua_nil; // 테이블 무효화
}

