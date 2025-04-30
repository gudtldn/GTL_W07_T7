#include "CameraModifier_CameraShake.h"

#include "CameraShake.h"

float UCameraModifier_CameraShake::InitializeOffset(const struct FOscillator& Param)
{
    switch ( Param.InitialOffset )
    {
        case EOO_OffsetRandom : return rand() * 2.f * PI;
        case EOO_OffsetZero : return 0.f;
        case EOO_MAX: return 0.f;
        default : return 0.f;
    }
}

void UCameraModifier_CameraShake::ReinitShake(int32 ActiveShakeIdx, float Scale)
{
    ActiveShakes[ActiveShakeIdx].Scale = Scale;
    
    UCameraShake* SourceShake = ActiveShakes[ActiveShakeIdx].SourceShake;

    if (SourceShake->OscillationDuration != 0.f)
    {
        ActiveShakes[ActiveShakeIdx].OscillatorTimeRemaining = SourceShake->OscillationDuration;

        if (ActiveShakes[ActiveShakeIdx].bBlendingOut)
        {
            ActiveShakes[ActiveShakeIdx].bBlendingOut = false;
            ActiveShakes[ActiveShakeIdx].CurrentBlendOutTime = 0.f;

            // stop any blendout and reverse it to a blendin
            ActiveShakes[ActiveShakeIdx].bBlendingIn = true;
            ActiveShakes[ActiveShakeIdx].CurrentBlendInTime = ActiveShakes[ActiveShakeIdx].SourceShake->OscillationBlendInTime * (1.f - ActiveShakes[ActiveShakeIdx].CurrentBlendOutTime / ActiveShakes[ActiveShakeIdx].SourceShake->OscillationBlendOutTime);
        }
    }
}

FCameraShakeInstance UCameraModifier_CameraShake::InitializeShake(UCameraShake* Shake, float Scale)
{
    FCameraShakeInstance Instance;
    Instance.SourceShakeName = Shake->GetName();

    
}

void UCameraModifier_CameraShake::AddCameraShake(UCameraShake* NewShake, float Scale)
{
}

void UCameraModifier_CameraShake::RemoveCameraShake(int32 ActiveShakeIdx)
{
}

void UCameraModifier_CameraShake::RemoveAllCameraShakes()
{
}

void UCameraModifier_CameraShake::UpdateCameraShake(float DeltaTime, FCameraShakeInstance& ShakeInstance)
{
}
