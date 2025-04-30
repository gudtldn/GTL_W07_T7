#pragma once
#include "GameFramework/Actor.h"


class APawn;
class APlayerController;

class AGameMode : public AActor
{
    DECLARE_CLASS(AGameMode, AActor)

public:
    AGameMode() = default;

    virtual void BeginPlay() override;

public:
    APlayerController* PlayerControllerClass;
    APawn* DefaultPawnClass;
};
