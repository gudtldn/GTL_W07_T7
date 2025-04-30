#include "PlayerCameraManager.h"


APlayerCameraManager::APlayerCameraManager()
{
    DefaultFOV = 90.0f;
    DefaultAspectRatio = 1.33333f;

    ViewPitchMin = -89.9f;
    ViewPitchMax = 89.9f;
    ViewYawMin = 0.f;
    ViewYawMax = 359.999f;
    ViewRollMin = -89.9f;
    ViewRollMax = 89.9f;

    TransformComponent = AddComponent<USceneComponent>(TEXT("TransformComponent0"));
    RootComponent = TransformComponent;

    // DefaultModifiers.Add(UCameraModifier_CameraShake::StaticClass());
}

void APlayerCameraManager::InitializeFor(APlayerController* PC)
{
    PCOwner = PC;
}

APlayerController* APlayerCameraManager::GetOwningPlayerController() const
{
    return PCOwner;
}
