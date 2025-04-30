#include "GameModeBase.h"
#include "Pawn.h"
#include "PlayerController.h"


AGameModeBase::AGameModeBase()
{
    // World에서 사용시 여기 직접 바꾸지 말고, 상속해서 사용해 주세요!
    PlayerControllerClass = APlayerController::StaticClass();
    DefaultPawnClass = APawn::StaticClass();
}

void AGameModeBase::InitGame()
{
    AActor::BeginPlay();

    
}
