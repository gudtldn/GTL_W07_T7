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
    
    void Tick(float DeltaTime) override;
    void UpdateCamera(float DeltaTime);
public:
    /** APlayerCameraManager를 소유하고 있는 APlayerController */
    APlayerController* PCOwner;

    FViewportCamera* ViewCamera;
    
    TArray<UCameraModifier*> ModifierList;
};
