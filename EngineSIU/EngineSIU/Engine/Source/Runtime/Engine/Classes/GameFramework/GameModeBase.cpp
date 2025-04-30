#include "GameModeBase.h"
#include "Pawn.h"
#include "PlayerController.h"
#include "PlayerStart.h"
#include "UObject/UObjectIterator.h"
#include "World/World.h"


AGameModeBase::AGameModeBase()
{
    // World에서 사용시 여기 직접 바꾸지 말고, 상속해서 사용해 주세요!
    PlayerControllerClass = APlayerController::StaticClass();
    DefaultPawnClass = APawn::StaticClass();
}

void AGameModeBase::InitGame()
{
    UWorld* World = GetWorld();

    APlayerController* NewPC = World->SpawnActor<APlayerController>(PlayerControllerClass);
    PlayerControllerInstance = NewPC;
    UE_LOG(ELogLevel::Display, "Spawned PlayerController: %s", *NewPC->GetName());

    AActor* PlayerStart = FindPlayerStart(NewPC);
    APawn* NewPawn = World->SpawnActor<APawn>(DefaultPawnClass);
    NewPawn->SetActorLocation(PlayerStart->GetActorLocation());
    NewPawn->SetActorRotation(PlayerStart->GetActorRotation());
    NewPC->SetPawn(NewPawn);
    UE_LOG(ELogLevel::Display, "Spawned Pawn: %s", *NewPawn->GetName());
}

AActor* AGameModeBase::FindPlayerStart(AController* PC, const FString& IncomingName) const
{
    UWorld* World = GetWorld();

    if (!IncomingName.IsEmpty())
    {
        const FName IncomingPlayerStartTag = FName(*IncomingName);
        // TODO: 나중에 TActorIterator 만들기
        for (APlayerStart* PlayerStart : TObjectRange<APlayerStart>())
        {
            if (PlayerStart->PlayerStartTag == IncomingPlayerStartTag && PlayerStart->GetWorld() == World)
            {
                return PlayerStart;
            }
        }
    }

    // TODO: 나중에 PC.StartSpot으로 가져오는 기능 추가하기
    // GameModeBase.cpp:1171

    if (!OriginActor)
    {
        OriginActor = World->SpawnActor<AActor>();
        OriginActor->SetActorLocation(FVector::ZeroVector);
        OriginActor->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
    }

    return OriginActor;
}
