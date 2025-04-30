#include "World.h"

#include "Actors/Cube.h"
#include "Actors/Player.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Components/SkySphereComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Actors/HeightFogActor.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "UnrealEd/SceneManager.h"
#include "Components/SceneComponent.h"

class UEditorEngine;

UWorld::UWorld()
{
    GameModeClass = AGameModeBase::StaticClass();
}

UWorld* UWorld::CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName)
{
    UWorld* NewWorld = FObjectFactory::ConstructObject<UWorld>(InOuter);
    NewWorld->WorldName = InWorldName;
    NewWorld->WorldType = InWorldType;
    NewWorld->InitializeNewWorld();

    return NewWorld;
}

void UWorld::InitializeNewWorld()
{
    ActiveLevel = FObjectFactory::ConstructObject<ULevel>(this);
    ActiveLevel->InitLevel(this);

}

UObject* UWorld::Duplicate(UObject* InOuter)
{
    // TODO: UWorld의 Duplicate는 역할 분리후 만드는것이 좋을듯
    UWorld* NewWorld = Cast<UWorld>(Super::Duplicate(InOuter));
    NewWorld->ActiveLevel = Cast<ULevel>(ActiveLevel->Duplicate(NewWorld));
    NewWorld->ActiveLevel->InitLevel(NewWorld);
    
    
    return NewWorld;
}

void UWorld::Tick(float DeltaTime)
{
    // SpawnActor()에 의해 Actor가 생성된 경우, 여기서 BeginPlay 호출
    for (AActor* Actor : PendingBeginPlayActors)
    {
        Actor->BeginPlay();
    }
    PendingBeginPlayActors.Empty();
}

void UWorld::BeginPlay()
{
    TArray Copied = ActiveLevel->Actors;
    for (AActor* Actor : Copied)
    {
        if (Actor->GetWorld() == this)
        {
            Actor->BeginPlay();
        }
    }
}

void UWorld::Release()
{
    if (ActiveLevel)
    {
        ActiveLevel->Release();
        GUObjectArray.MarkRemoveObject(ActiveLevel);
        ActiveLevel = nullptr;
    }
    
    GUObjectArray.ProcessPendingDestroyObjects();
}

AActor* UWorld::SpawnActor(UClass* InClass, FName InActorName)
{
    if (!InClass)
    {
        UE_LOG(ELogLevel::Error, TEXT("SpawnActor failed: ActorClass is null."));
        return nullptr;
    }

    
    // TODO: SpawnParams에서 이름 가져오거나, 필요시 여기서 자동 생성
    // if (SpawnParams.Name != NAME_None) ActorName = SpawnParams.Name;
    
    if (InClass->IsChildOf<AActor>())
    {
        AActor* NewActor = Cast<AActor>(FObjectFactory::ConstructObject(InClass, this, InActorName));

        if (!NewActor->GetRootComponent())
        {
            USceneComponent* SceneComp = NewActor->AddComponent<USceneComponent>(TEXT("DefaultSceneRoot"));
            NewActor->SetRootComponent(SceneComp);
        }

        // TODO: 일단 AddComponent에서 Component마다 초기화
        // 추후에 RegisterComponent() 만들어지면 주석 해제
        // Actor->InitializeComponents();
        ActiveLevel->Actors.Add(NewActor);
        PendingBeginPlayActors.Add(NewActor);
        return NewActor;
    }
    
    UE_LOG(ELogLevel::Error, TEXT("SpawnActor failed: Class '%s' is not derived from AActor."), *InClass->GetName());
    return nullptr;
}

bool UWorld::DestroyActor(AActor* ThisActor)
{
    if (ThisActor->GetWorld() == nullptr)
    {
        return false;
    }

    if (ThisActor->IsActorBeingDestroyed())
    {
        return true;
    }
    
    // UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    //
    // Engine->DeselectActor(ThisActor);

    // 액터의 Destroyed 호출
    ThisActor->Destroyed();

    if (ThisActor->GetOwner())
    {
        ThisActor->SetOwner(nullptr);
    }

    TSet<UActorComponent*> Components = ThisActor->GetComponents();
    for (UActorComponent* Component : Components)
    {
        Component->DestroyComponent();
    }

    // World에서 제거
    ActiveLevel->Actors.Remove(ThisActor);

    // 제거 대기열에 추가
    GUObjectArray.MarkRemoveObject(ThisActor);
    return true;
}

UWorld* UWorld::GetWorld() const
{
    return const_cast<UWorld*>(this);
}

void UWorld::SetGameModeClass(TSubclassOf<AGameModeBase> InGameModeClass)
{
    if (InGameModeClass == nullptr)
    {
        UE_LOG(ELogLevel::Error, TEXT("SetGameModeClass failed: GameModeClass is null."));
        return;
    }
    GameModeClass = InGameModeClass;
}

void UWorld::InitGameMode()
{
    if (
        WorldType == EWorldType::Editor            // 일반적인 에디터 환경
        || WorldType == EWorldType::EditorPreview  // 머티리얼 에디터나 애님 블루프린트 같은 프리뷰 뷰포트
    ) {
        UE_LOG(ELogLevel::Display, TEXT("Skipping GameMode spawn for Editor/Preview world."));
        return;
    }

    if (GameModeInstance)
    {
        UE_LOG(ELogLevel::Warning, TEXT("GameMode already exists in this world. Skipping spawn."));
        return;
    }

    UClass* SubClass = GameModeClass.Get();
    if (!SubClass)
    {
        SubClass = AGameModeBase::StaticClass();
    }

    AGameModeBase* SpawnGameMode = Cast<AGameModeBase>(SpawnActor(SubClass, "__World_AGameMode"));
    GameModeInstance = SpawnGameMode;
}

void UWorld::DestroyGameMode()
{
    if (GameModeInstance)
    {
        DestroyActor(GameModeInstance);
        GameModeInstance = nullptr;
    }
}

