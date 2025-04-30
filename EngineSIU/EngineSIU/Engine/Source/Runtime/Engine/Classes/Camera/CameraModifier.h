#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class APlayerCameraManager;

class UCameraModifier : public UObject
{
    DECLARE_CLASS(UCameraModifier, UObject)

public:
    UCameraModifier();

    void StartShake(float InDuration);
    
    bool ModifyCamera(float DeltaTime, struct FViewportCamera* ViewInfo);
    
public:
    APlayerCameraManager* CameraOwner;

    float AlphaInTime;

    float AlphaOutTime;

    float Alpha;

    bool bDisable;

    float ElapsedTime;

    float Duration;

    float Amplitude;

    float Frequency;

    float InternalPhase;
    
    uint8 Priority;

private:
    bool bStartShake;

    FVector OriginLocation;
    FVector OriginRotation;

    float YawDirection;
};
