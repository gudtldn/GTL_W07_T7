#include "PlayerCameraManager.h"

#include "CameraModifier.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"

APlayerCameraManager::APlayerCameraManager():
    PCOwner(nullptr),
    ViewCamera(nullptr)
{
    USceneComponent* RootComp = AddComponent<USceneComponent>(TEXT("RootComp"));
    SetRootComponent(RootComp);
    
    UCameraModifier* CameraShake = FObjectFactory::ConstructObject<UCameraModifier>(this);
    ModifierList.Add(CameraShake);

    GEngineLoop.PCM = this;
}

UObject* APlayerCameraManager::Duplicate(UObject* InOuter)
{
    ThisClass* NewCameraManager = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewCameraManager->PCOwner = PCOwner;
    NewCameraManager->ViewCamera = ViewCamera;
    NewCameraManager->ModifierList = ModifierList;
    return NewCameraManager;
}

void APlayerCameraManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ViewCamera = &GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->PerspectiveCamera;
    
    UpdateCamera(DeltaTime);
}

void APlayerCameraManager::UpdateCamera(float DeltaTime)
{
    for (int32 i = 0; i < ModifierList.Num(); i++)
    {
        UCameraModifier* Mod = ModifierList[i];
        if (Mod && !Mod->bDisable)
        {
            Mod->ModifyCamera(DeltaTime, ViewCamera);
        }
    }
}
