#pragma once
#include "Controller.h"
#include "Template/SubclassOf.h"

class APlayerCameraManager;


class APlayerController : public AController
{
    DECLARE_CLASS(APlayerController, AController)

public:
    APlayerController() = default;

    virtual void SpawnPlayerCameraManager();

public:
    APlayerCameraManager* PlayerCameraManager = nullptr;

    TSubclassOf<APlayerCameraManager> PlayerCameraManagerClass;
};
