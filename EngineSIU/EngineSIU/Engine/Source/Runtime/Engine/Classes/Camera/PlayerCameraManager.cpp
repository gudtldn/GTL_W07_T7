#include "PlayerCameraManager.h"

void APlayerCameraManager::BeginPlay() 
{ 
    Super::BeginPlay();
    // 여기서 값을 초기화할 수도 있는데
    // 그럴 경우 게임 처음에 시작할 때 어둡게 시작하는 연출에 문제가 있을 수 있어보임
}

void APlayerCameraManager::Tick(float DeltaTime) 
{ 
    Super::Tick(DeltaTime);

    float Elapsed = FadeTime - FadeTimeRemaining;
    float AlphaT = 0.0f;
    if (FadeTime > 0.0f)
    {
        AlphaT = FMath::Clamp(Elapsed / FadeTime, 0.f, 1.f);
    }

    if (FadeTimeRemaining < 0.0f && !bHoldWhenFinished)
    {
        FadeAmount = 0.0f;
        return;
    }

    FadeAmount = FMath::Lerp(FadeAlpha.X, FadeAlpha.Y, AlphaT);
    FadeTimeRemaining -= DeltaTime;
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
