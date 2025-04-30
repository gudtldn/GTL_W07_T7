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

public:
    float DefaultFOV;
    float DefaultAspectRatio;

    float ViewPitchMin;
    float ViewPitchMax;
    float ViewYawMin;
    float ViewYawMax;
    float ViewRollMin;
    float ViewRollMax;

    /** APlayerCameraManager를 소유하고 있는 APlayerController */
    APlayerController* PCOwner = nullptr;

private:
    USceneComponent* TransformComponent;
};
