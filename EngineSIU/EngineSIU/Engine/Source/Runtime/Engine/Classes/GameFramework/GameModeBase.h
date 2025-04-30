#pragma once
#include "GameFramework/Actor.h"


class APawn;
class APlayerController;

class AGameModeBase : public AActor
{
    DECLARE_CLASS(AGameModeBase, AActor)

public:
    AGameModeBase();

    virtual void BeginPlay() override;

public:
    APlayerController* PlayerControllerClass;
    APawn* DefaultPawnClass;
};
