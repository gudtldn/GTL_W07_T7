#pragma once
#include <optional>
#include <filesystem>

#include "Engine/StaticMeshActor.h"
#include "sol/sol.hpp"


class ALuaActor : public AStaticMeshActor
{
    DECLARE_CLASS(ALuaActor, AStaticMeshActor)

public:
    ALuaActor();

    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    virtual void OnOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp);

    /** Lua Script에 변경사항이 생겼을 때 Manager에서 호출되는 함수 */
    void HandleScriptReload(const sol::protected_function& NewFactory);

public:
    std::optional<std::filesystem::path> GetScriptPath() const;
    void SetScriptPath(const std::optional<std::filesystem::path>& Path);

protected:
    template<typename... ArgsType>
    sol::protected_function_result CallLuaFunction(const char* FuncName, ArgsType&&... Args) const;

    void CleanupLuaState();

protected:
    sol::table SelfTable = sol::lua_nil;

//private:
    // TODO: .scene에 저장을 어떻게 하지?
    std::optional<std::filesystem::path> LuaScriptPath;
};


template <typename... ArgsType>
sol::protected_function_result ALuaActor::CallLuaFunction(const char* FuncName, ArgsType&&... Args) const
{
    if (!SelfTable.valid())
    {
        // 테이블이 유효하지 않으면 호출 시도 안 함
        return sol::protected_function_result{};
    }

    sol::protected_function LuaFunc = SelfTable[FuncName];
    if (LuaFunc.valid())
    {
        try
        {
            // 첫 번째 인자로 항상 self 테이블 전달 (Lua의 self 역할)
            sol::protected_function_result Result = LuaFunc(SelfTable, std::forward<ArgsType>(Args)...);
            if (!Result.valid())
            {
                const sol::error Error = Result;
                UE_LOG(ELogLevel::Error, "ALuaActor [%s]: Error calling Lua function '%s': %s", *GetName(), FuncName, Error.what());
            }
            return Result;
        }
        catch (const sol::error& Error)
        {
            UE_LOG(ELogLevel::Error, "ALuaActor [%s]: Lua error calling function '%s': %s", *GetName(), FuncName, Error.what());
        }
        catch (const std::exception& Error)
        {
            UE_LOG(ELogLevel::Error, "ALuaActor [%s]: C++ exception calling Lua function '%s': %s", *GetName(), FuncName, Error.what());
        }
    }
    else
    {
        UE_LOG(ELogLevel::Error, TEXT("LuaActor::CallLuaFunction: %s() is not found"), FuncName);
    }
    return sol::protected_function_result{};
}
