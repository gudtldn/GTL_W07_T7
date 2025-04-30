#pragma once
#include "GameFramework/Actor.h"

class APlayerController;


class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)

public:
    APlayerCameraManager();

    virtual void InitializeFor(APlayerController* PC);
    APlayerController* GetOwningPlayerController() const;

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
    float DefaultFOV;
    float DefaultAspectRatio;

    float ViewPitchMin;
    float ViewPitchMax;
    float ViewYawMin;
    float ViewYawMax;
    float ViewRollMin;
    float ViewRollMax;

    bool bDefaultConstrainAspectRatio;

    /** APlayerCameraManager를 소유하고 있는 APlayerController */
    APlayerController* PCOwner = nullptr;

private:
    USceneComponent* TransformComponent;
};
