#pragma once
#include "Controller.h"

class APlayerCameraManager;


class APlayerController : public AController
{
    DECLARE_CLASS(APlayerController, AController)

public:
    APlayerController() = default;

public:
    APlayerCameraManager* PlayerCameraManager = nullptr;
};
