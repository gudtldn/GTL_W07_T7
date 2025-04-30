// LuaPlayer.cpp
#include "LuaPlayer.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "LuaManager.h"
#include "World/WorldType.h"
#include "Engine/Engine.h"
#include "World/World.h"
#include "LuaHeartActor.h"
#include "Camera/CameraModifier.h"
#include "Camera/PlayerCameraManager.h"
#include "Developer/FMOD/SoundManager.h"
#include "Engine/Classes/Components/RigidbodyComponent.h"

ALuaPlayer::ALuaPlayer()
{
    LuaScriptPath = PlayerLuaScriptPath;
    
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
                (void)CallLuaFunction("BeginPlay"); // 경로 변경 시에도 BeginPlay 호출
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

UObject* ALuaPlayer::Duplicate(UObject* InOuter)
{
    return Super::Duplicate(InOuter);
}

void ALuaPlayer::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void ALuaPlayer::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
}

void ALuaPlayer::BeginPlay()
{
    Super::BeginPlay();
    (void)CallLuaFunction("BeginPlay");

    if (GEngine->ActiveWorld->WorldType == EWorldType::Editor)
        return;

    FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();
    Handler->OnMouseDownDelegate.AddUObject(this, &ALuaPlayer::OnLeftMouseDown);
    Handler->OnMouseUpDelegate.AddUObject(this, &ALuaPlayer::OnLeftMouseUp);
}

void ALuaPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    (void)CallLuaFunction("Tick", DeltaTime);
}

void ALuaPlayer::Destroyed()
{
    (void)CallLuaFunction("Destroyed");
    Super::Destroyed();
}

void ALuaPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    (void)CallLuaFunction("EndPlay", EndPlayReason);
    Super::EndPlay(EndPlayReason);
}

void ALuaPlayer::OnOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
    Super::OnOverlap(OtherActor, OtherComp);
    CallLuaFunction("OnOverlap", OtherActor, OtherComp);
}

void ALuaPlayer::HandleScriptReload(const sol::protected_function& NewFactory)
{
    Super::HandleScriptReload(NewFactory);
    (void)CallLuaFunction("OnScriptReload");
}

void ALuaPlayer::OnLeftMouseDown(const FPointerEvent& InMouseEvent)
{
    if (GEngine->ActiveWorld->WorldType == EWorldType::Editor)
        return;
    
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
        (void)CallLuaFunction("OnLeftMouseDown");
    
}

void ALuaPlayer::OnLeftMouseUp(const FPointerEvent& InMouseEvent)
{
    if (GEngine->ActiveWorld->WorldType == EWorldType::Editor)
        return;

    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        (void)CallLuaFunction("OnLeftMouseUp");
        FSoundManager::Get()->PlaySound("Contents\\Sound\\shoot.wav", ShootChannel, false);
        if (GEngineLoop.PCM->ModifierList.Num() > 0)
        {
            GEngineLoop.PCM->ModifierList[0]->StartShake(0.12f);
        }
    }
    
    UE_LOG(ELogLevel::Display, "OnLeftMouseUp %s", *GetName());
}

FVector ALuaPlayer::GetAimDirection()
{
    UE_LOG(ELogLevel::Display, TEXT("GetAimDirection"));
    return FVector(0.0f, 0.0f, 0.0f);
}

void ALuaPlayer::SpawnHeart(FVector pos, FVector dir, float chargeForce, int playerIndex)
{
    UE_LOG(ELogLevel::Display, TEXT("SpawnHeart"));

    ALuaHeartActor* SpawnedActor = GetWorld()->SpawnActor<ALuaHeartActor>();
    SpawnedActor->SetActorLabel(TEXT("OBJ_LUA_HEART_ACTOR"));

    SpawnedActor->SetActorLocation(pos);
    SpawnedActor->RigidbodyComp->ApplyForceAtPoint(dir * chargeForce, FVector(0, 0, 0.0f));

    SpawnedActor->SetPlayerIndex(playerIndex);
}
