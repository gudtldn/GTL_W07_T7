#pragma once
#include "GameFramework/Actor.h"

class APlayerController;


class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)

public:
    APlayerCameraManager() = default;

public:
    /** APlayerCameraManager를 소유하고 있는 APlayerController */
    APlayerController* PCOwner;
};
