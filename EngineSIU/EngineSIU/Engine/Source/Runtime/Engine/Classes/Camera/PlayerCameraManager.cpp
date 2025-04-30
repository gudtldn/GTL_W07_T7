#include "PlayerCameraManager.h"
#include "CameraModifier.h"
#include "Components/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "LevelEditor/SLevelEditor.h"
#include "UObject/UObjectIterator.h"
#include "UnrealEd/EditorViewportClient.h"


APlayerCameraManager::APlayerCameraManager()
{
    DefaultFOV = 90.0f;
    // DefaultAspectRatio = 1.33333f;
    DefaultAspectRatio = 1.0f;

    ViewPitchMin = -89.9f;
    ViewPitchMax = 89.9f;
    ViewYawMin = 0.f;
    ViewYawMax = 359.999f;
    ViewRollMin = -89.9f;
    ViewRollMax = 89.9f;

    bDefaultConstrainAspectRatio = true;

    TransformComponent = AddComponent<USceneComponent>(TEXT("TransformComponent0"));
    RootComponent = TransformComponent;

    UCameraModifier* CameraShake = FObjectFactory::ConstructObject<UCameraModifier>(this);
    ModifierList.Add(CameraShake);

    GEngineLoop.PCM = this;

    // DefaultModifiers.Add(UCameraModifier_CameraShake::StaticClass());
}

UObject* APlayerCameraManager::Duplicate(UObject* InOuter)
{
    ThisClass* NewCameraManager = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewCameraManager->PCOwner = PCOwner;
    NewCameraManager->ViewCamera = ViewCamera;
    NewCameraManager->ModifierList = ModifierList;
    return NewCameraManager;
}

void APlayerCameraManager::BeginPlay() 
{ 
    Super::BeginPlay();
    // 여기서 값을 초기화할 수도 있는데
    // 그럴 경우 게임 처음에 시작할 때 어둡게 시작하는 연출에 문제가 있을 수 있어보임
}

void APlayerCameraManager::Tick(float DeltaTime) 
{ 
    Super::Tick(DeltaTime);
    FadeTick(DeltaTime);
    SpringArmTick();

    // TODO: 임시 카메라 수정
    ViewCamera = &GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->PerspectiveCamera;
    UpdateCamera(DeltaTime);
}


void APlayerCameraManager::FadeTick(float DeltaTime) 
{
    float Elapsed = FadeTime - FadeTimeRemaining;
    float AlphaT = FMath::Clamp(Elapsed / FadeTime, 0.f, 1.f);
    if (FadeTimeRemaining < 0.0f && !bHoldWhenFinished)
    {
        FadeAmount = 0.0f;
        return;
    }

    FadeAmount = FMath::Lerp(FadeAlpha.X, FadeAlpha.Y, AlphaT);
    FadeTimeRemaining -= DeltaTime;
}

void APlayerCameraManager::SpringArmTick() 
{
    FVector CameraWorldLocation;
    FViewportCamera& Camera = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->PerspectiveCamera;
    
    for (const auto iter : TObjectRange<USpringArmComponent>())
    {
        if (iter->GetWorld() != GEngine->ActiveWorld)
        {
            continue;
        }

        CameraWorldLocation = iter->GetSpringArmLocation();
    }

    Camera.SetLocation(CameraWorldLocation);
}

void APlayerCameraManager::InitializeFor(APlayerController* PC)
{
    PCOwner = PC;
}

APlayerController* APlayerCameraManager::GetOwningPlayerController() const
{
    return PCOwner;
}

FLinearColor APlayerCameraManager::GetFadeConstant() const 
{
    return FLinearColor{FadeColor.R, FadeColor.G, FadeColor.B, FadeAmount};
}

void APlayerCameraManager::StartCameraFade(
    float FromAlpha, float ToAlpha,
    float Duration,
    FLinearColor Color,
    bool InShouldFadeAudio,
    bool InHoldWhenFinished
)
{
    FadeAlpha.X = FromAlpha;
    FadeAlpha.Y = ToAlpha;
    FadeTime = Duration;
    FadeTimeRemaining = Duration;
    FadeColor = Color;
    bHoldWhenFinished = InHoldWhenFinished;
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

void APlayerCameraManager::GetLetterBoxViewport(
    int ScreenW, int ScreenH,
    int& OutX, int& OutY,
    int& OutW, int& OutH
) const {
    // 레터박스 비활성화 시 전체 화면
    if (!bDefaultConstrainAspectRatio)
    {
        OutX = 0; OutY = 0;
        OutW = ScreenW; OutH = ScreenH;
        return;
    }

    const float CurrentAspect = static_cast<float>(ScreenW) / static_cast<float>(ScreenH);
    const float TargetAspect  = DefaultAspectRatio;
    float& A = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->AspectRatio;
    A = TargetAspect;

    // 가로가 더 넓을 때 → 좌우에 검은 바 (레터박스)
    if (CurrentAspect > TargetAspect)
    {
        OutH = ScreenH;
        OutW = static_cast<int>(TargetAspect * ScreenH + 0.5f);
        OutX = (ScreenW - OutW) / 2;
        OutY = 0;
    }
    // 세로가 더 넓을 때 → 상하에 검은 바 (필러박스)
    else if (CurrentAspect < TargetAspect)
    {
        OutW = ScreenW;
        OutH = static_cast<int>(ScreenW / TargetAspect + 0.5f);
        OutX = 0;
        OutY = (ScreenH - OutH) / 2;
    }
    // 같을 때 → 전체
    else
    {
        OutX = 0; OutY = 0;
        OutW = ScreenW; OutH = ScreenH;
    }
}
