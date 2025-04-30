#include "PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "World/World.h"

// PostInitializeComponents에서 SpawnPlayerCameraManager() 호출
// PlayerController.cpp:1050 참조

void APlayerController::SpawnPlayerCameraManager()
{
    PlayerCameraManager = GetWorld()->SpawnActor<APlayerCameraManager>(PlayerCameraManagerClass);
    PlayerCameraManager->SetOwner(this);

    if (PlayerCameraManager != nullptr)
    {
        PlayerCameraManager->InitializeFor(this);
    }
    else
    {
        UE_LOG(ELogLevel::Warning, "[LogPlayerController]Couldn't Spawn PlayerCameraManager for Player!!");
    }
}
