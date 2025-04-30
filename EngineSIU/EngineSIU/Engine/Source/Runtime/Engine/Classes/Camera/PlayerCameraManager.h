#pragma once
#include "GameFramework/Actor.h"

class APlayerController;


class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)

public:
    APlayerCameraManager() = default;

    /** Actor가 게임에 배치되거나 스폰될 때 호출됩니다. */
    virtual void BeginPlay() override;

    /** 매 Tick마다 호출됩니다. */
    virtual void Tick(float DeltaTime) override;
    void FadeTick(float DeltaTime);
    void SpringArmTick();

    FLinearColor GetFadeConstant() const;

    float GetFadeAmount() { return FadeAmount; }

public:
    /** APlayerCameraManager를 소유하고 있는 APlayerController */
    APlayerController* PCOwner;

    void StartCameraFade(
        float FromAlpha,
        float ToAlpha,
        float Duration,
        FLinearColor Color,
        bool InShouldFadeAudio = true,
        bool InHoldWhenFinished = true
    );

private:
    FLinearColor FadeColor;
    float FadeAmount = 0.f;   // 보간을 거친 현재 Alpha 값
    FVector2D FadeAlpha = {0.0f, 0.0f}; // StartAlpha 와 DestAlpha
    float FadeTime;
    float FadeTimeRemaining;
    bool bHoldWhenFinished;
};
