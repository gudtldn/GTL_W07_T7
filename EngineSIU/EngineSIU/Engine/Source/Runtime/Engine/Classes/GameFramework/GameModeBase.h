#pragma once
#include "GameFramework/Actor.h"
#include "Template/SubclassOf.h"


class APawn;
class APlayerController;

class AGameModeBase : public AActor
{
    DECLARE_CLASS(AGameModeBase, AActor)

public:
    AGameModeBase();

    /** GameMode에 설정된 기본 클래스를 Spawn합니다. */
    virtual void InitGame();

public:
    TSubclassOf<APlayerController> PlayerControllerClass;
    TSubclassOf<APawn> DefaultPawnClass;
};
