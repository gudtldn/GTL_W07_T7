#pragma once
#include "GameFramework/Actor.h"

struct FViewportCamera;
class UCameraModifier;
class APlayerController;


class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)

public:
    APlayerCameraManager();

    UObject* Duplicate(UObject* InOuter) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual void InitializeFor(APlayerController* PC);
    APlayerController* GetOwningPlayerController() const;

    void UpdateCamera(float DeltaTime);
    void FadeTick(float DeltaTime);
    void SpringArmTick();

    void StartCameraFade(
        float FromAlpha,
        float ToAlpha,
        float Duration,
        FLinearColor Color,
        bool InShouldFadeAudio = true,
        bool InHoldWhenFinished = true
    );

    /**
     * 화면 크기(ScreenW×ScreenH)에 대해,
     * 레터박스 적용 후 실제 렌더링할 Viewport 영역을 계산.
     */
    void GetLetterBoxViewport(
        int ScreenW, int ScreenH,
        int& OutX, int& OutY,
        int& OutW, int& OutH
    ) const;

public:
    FLinearColor GetFadeConstant() const;
    float GetFadeAmount() const { return FadeAmount; }

public:
    FViewportCamera* ViewCamera;
    TArray<UCameraModifier*> ModifierList;

    /** APlayerCameraManager를 소유하고 있는 APlayerController */
    APlayerController* PCOwner = nullptr;


    float DefaultFOV;
    float DefaultAspectRatio;

    float ViewPitchMin;
    float ViewPitchMax;
    float ViewYawMin;
    float ViewYawMax;
    float ViewRollMin;
    float ViewRollMax;

    bool bDefaultConstrainAspectRatio;

private:
    FLinearColor FadeColor;
    float FadeAmount = 0.f;   // 보간을 거친 현재 Alpha 값
    FVector2D FadeAlpha = {0.0f, 0.0f}; // StartAlpha 와 DestAlpha
    float FadeTime;
    float FadeTimeRemaining;
    bool bHoldWhenFinished;

private:
    USceneComponent* TransformComponent;
};
