#include "CameraShake.h"

UCameraShake::UCameraShake()
{
    bSingleInstance = false;
    OscillationDuration = 0.0f;
    OscillationBlendInTime = 0.1f;
    OscillationBlendOutTime = 0.2f;
}

float UCameraShake::GetRotOscillationMagnitude()
{
    FVector V;
    V.X = RotationOscillator.Pitch.Amplitude;
    V.Y = RotationOscillator.Yaw.Amplitude;
    V.Z = RotationOscillator.Roll.Amplitude;
    return V.Length();
}

float UCameraShake::GetLocOscillationMagnitude()
{
    FVector V;
    V.X = LocationOscillator.X.Amplitude;
    V.Y = LocationOscillator.Y.Amplitude;
    V.Z = LocationOscillator.Z.Amplitude;
    return V.Length();
}
