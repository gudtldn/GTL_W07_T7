#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

enum EInitialOscillatorOffset : uint8
{
    /** Start with random offset (default) */
    EOO_OffsetRandom,

    /** Start with zero offset */
    EOO_OffsetZero,
    EOO_MAX,
};

struct FOscillator
{
    float Amplitude;

    float Frequency;

    EInitialOscillatorOffset InitialOffset;

    FOscillator()
        : Amplitude(0.0f)
        , Frequency(0.0f)
        , InitialOffset(EOO_OffsetRandom)
    {
    }
};

struct FROscillator
{
    FOscillator Pitch;
    FOscillator Yaw;
    FOscillator Roll;
};

struct FVOscillator
{
    FOscillator X;
    FOscillator Y;
    FOscillator Z;
};


class UCameraShake : public UObject
{
    DECLARE_CLASS(UCameraShake, UObject)

public:
    UCameraShake();

    virtual UObject* Duplicate(UObject* InOuter) override;

    bool bSingleInstance;

    /** Duration in seconds of current screen shake. (v < 0) means indefinite, (v = 0) means no oscillation */
    float OscillationDuration;

    float OscillationBlendInTime;

    float OscillationBlendOutTime;

    /** Rotation oscillation */
    FROscillator RotationOscillator;
    
    /** Positional oscillation */
    FVOscillator LocationOscillator;

    virtual float GetRotOscillationMagnitude();

    virtual float GetLocOscillationMagnitude();
};
