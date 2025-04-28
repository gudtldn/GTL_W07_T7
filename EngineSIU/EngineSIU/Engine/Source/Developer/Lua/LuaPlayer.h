// LuaPlayer.h
#pragma once

#include <optional>
#include <filesystem>

#include "fmod.hpp"
#include "Engine/StaticMeshActor.h"
#include "LuaActor.h"
namespace fs = std::filesystem;

class ALuaPlayer : public ALuaActor
{
    // UE4 리플렉션(반사) 시스템에 등록
    DECLARE_CLASS(ALuaPlayer, ALuaActor)

public:
    ALuaPlayer();

    // 복제, 프로퍼티 직렬화는 부모 그대로
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    // 주요 라이프사이클 훅
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 충돌 이벤트
    virtual void OnOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;

protected:
    virtual void HandleScriptReload(const sol::protected_function& NewFactory);

    const fs::path PlayerLuaScriptPath = fs::current_path().parent_path() / "GameJam" / "Lua" / "Player.lua";
    
    // 입력 처리용 콜백
    void OnLeftMouseDown(const FPointerEvent& InMouseEvent);
    void OnLeftMouseUp(const FPointerEvent& InMouseEvent);

    FMOD::Channel* ShootChannel;
public:
    FVector GetAimDirection();
    void SpawnHeart(FVector pos, FVector dir, float chargeForce, int playerIndex);
};
