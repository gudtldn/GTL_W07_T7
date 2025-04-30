#include "PlayerCameraManager.h"


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

    // DefaultModifiers.Add(UCameraModifier_CameraShake::StaticClass());
}

void APlayerCameraManager::InitializeFor(APlayerController* PC)
{
    PCOwner = PC;
}

APlayerController* APlayerCameraManager::GetOwningPlayerController() const
{
    return PCOwner;
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
