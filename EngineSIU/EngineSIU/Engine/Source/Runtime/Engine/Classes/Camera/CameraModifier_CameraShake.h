#pragma once
#include "Container/Array.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"

struct FCameraShakeInstance
{
    /** Source shake */
    class UCameraShake* SourceShake;

    FName SourceShakeName;

    /** (V < 0.f) means play infinitely. */
    float OscillatorTimeRemaining;

    /** Blending Vars */
    bool bBlendingIn;

    float CurrentBlendInTime;
    
    bool bBlendingOut;

    float CurrentBlendOutTime;

    /** Current Offsets. */
    FVector LocSinOffset;

    FVector RotSinOffset;

    float FOVSinOffset;

    float Scale;

    FCameraShakeInstance():
        SourceShake(nullptr),
        OscillatorTimeRemaining(0),
        bBlendingIn(false),
        CurrentBlendInTime(0),
        bBlendingOut(false),
        CurrentBlendOutTime(0),
        FOVSinOffset(0),
        Scale(0)
    {

    }
};


class UCameraModifier_CameraShake
{
    TArray<struct FCameraShakeInstance> ActiveShakes;
    
public:
    UCameraModifier_CameraShake() = default;
    
    float InitializeOffset(const struct FOscillator& Param);
    virtual void ReinitShake(int32 ActiveShakeIdx, float Scale);

    /** Initialize camera shake structure */
    virtual FCameraShakeInstance InitializeShake(UCameraShake* Shake, float Scale);

    /** Add a new screen shake to the list */
    virtual void AddCameraShake(UCameraShake* NewShake, float Scale);

    virtual void RemoveCameraShake(int32 ActiveShakeIdx);
    virtual void RemoveAllCameraShakes();

    /** Update a CameraShake */
    virtual void UpdateCameraShake(float DeltaTime, FCameraShakeInstance& ShakeInstance);

    /** Modifier Interface */
    // Add something...
    /** Modifier Interface */
};
