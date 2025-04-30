#include "PlayerCameraManager.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Engine/Engine.h"
#include "Engine/EditorEngine.h"
#include "LevelEditor/SLevelEditor.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Classes/Components/SpringArmComponent.h"


void APlayerCameraManager::BeginPlay() 
{ 
    Super::BeginPlay();
    // 여기서 값을 초기화할 수도 있는데
    // 그럴 경우 게임 처음에 시작할 때 어둡게 시작하는 연출에 문제가 있을 수 있어보임
}

void APlayerCameraManager::Tick(float DeltaTime) 
{ 
    Super::Tick(DeltaTime);
    FadeTick(DeltaTime);
    SpringArmTick();
}

void APlayerCameraManager::FadeTick(float DeltaTime) 
{
    float Elapsed = FadeTime - FadeTimeRemaining;
    float AlphaT = FMath::Clamp(Elapsed / FadeTime, 0.f, 1.f);
    if (FadeTimeRemaining < 0.0f && !bHoldWhenFinished)
    {
        FadeAmount = 0.0f;
        return;
    }

    FadeAmount = FMath::Lerp(FadeAlpha.X, FadeAlpha.Y, AlphaT);
    FadeTimeRemaining -= DeltaTime;
}

void APlayerCameraManager::SpringArmTick() 
{
    FVector CameraWorldLocation;
    FViewportCamera& Camera = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->PerspectiveCamera;
    
    for (const auto iter : TObjectRange<USpringArmComponent>())
    {
        if (iter->GetWorld() != GEngine->ActiveWorld)
        {
            continue;
        }

        CameraWorldLocation = iter->GetSpringArmLocation();
    }

    Camera.SetLocation(CameraWorldLocation);
}

FLinearColor APlayerCameraManager::GetFadeConstant() const 
{
    return FLinearColor(FadeColor.R, FadeColor.G, FadeColor.B, FadeAmount);
}

void APlayerCameraManager::StartCameraFade(
    float FromAlpha, float ToAlpha,
    float Duration,
    FLinearColor Color,
    bool InShouldFadeAudio,
    bool InHoldWhenFinished
)
{
    FadeAlpha.X = FromAlpha;
    FadeAlpha.Y = ToAlpha;
    FadeTime = Duration;
    FadeTimeRemaining = Duration;
    FadeColor = Color;
    bHoldWhenFinished = InHoldWhenFinished;
}
