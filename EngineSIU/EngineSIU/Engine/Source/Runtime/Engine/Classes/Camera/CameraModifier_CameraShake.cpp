#include "CameraModifier_CameraShake.h"

#include "CameraShake.h"

float UCameraModifier_CameraShake::InitializeOffset(const struct FOscillator& Param)
{
    switch ( Param.InitialOffset )
    {
        case EOO_OffsetRandom : return (rand() / (float)RAND_MAX) * 2.f * PI;
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
    Instance.Scale = Scale;
    Instance.OscillatorTimeRemaining = Shake->OscillationDuration;
    Instance.CurrentBlendInTime = 0.f;
    Instance.CurrentBlendOutTime = 0.f;
    Instance.bBlendingIn = Shake->OscillationBlendInTime > 0.f;
    Instance.bBlendingOut = false;

    // Initialize oscillators if applicable
    Instance.RotOscillator.X = InitializeOffset(Shake->RotOscillation.Pitch);
    Instance.RotOscillator.Y = InitializeOffset(Shake->RotOscillation.Yaw);
    Instance.RotOscillator.Z = InitializeOffset(Shake->RotOscillation.Roll);
    Instance.LocOscillator.X = InitializeOffset(Shake->LocOscillation.X);
    Instance.LocOscillator.Y = InitializeOffset(Shake->LocOscillation.Y);
    Instance.LocOscillator.Z = InitializeOffset(Shake->LocOscillation.Z);
    Instance.FOVOscillator = InitializeOffset(Shake->FOVOscillation);

    return Instance;
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
