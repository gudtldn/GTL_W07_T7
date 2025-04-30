#include "PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "World/World.h"


void APlayerController::SpawnPlayerCameraManager()
{
    PlayerCameraManager = GetWorld()->SpawnActor<APlayerCameraManager>(PlayerCameraManagerClass);

    if (PlayerCameraManager != nullptr)
    {
        PlayerCameraManager->InitializeFor(this);
    }
    else
    {
        UE_LOG(ELogLevel::Warning, "[LogPlayerController]Couldn't Spawn PlayerCameraManager for Player!!");
    }
}
