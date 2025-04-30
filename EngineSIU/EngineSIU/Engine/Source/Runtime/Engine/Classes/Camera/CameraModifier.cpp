#include "CameraModifier.h"

#include "PlayerCameraManager.h"
#include "UnrealEd/EditorViewportClient.h"

UCameraModifier::UCameraModifier():
    CameraOwner(nullptr),
    AlphaInTime(0),
    AlphaOutTime(0),
    Priority(127)
{
    ElapsedTime = 0.f;
    Duration = 0.f;
    Amplitude = 5.0f;
    Frequency = 20.f;
    InternalPhase = 0.f;
    Alpha = 1.f;
    bDisable = true;
    bStartShake = false;
}

void UCameraModifier::StartShake(float InDuration)
{
    bDisable = false;
    Duration = InDuration;
}

bool UCameraModifier::ModifyCamera(float DeltaTime, FViewportCamera* ViewInfo)
{
    if (bDisable || Duration <= 0.f)
    {
        bStartShake = false;
        return false;
    }

    if (!bStartShake)
    {
        bStartShake = true;
        OriginLocation = ViewInfo->ViewLocation;
        OriginRotation = ViewInfo->ViewRotation;

        // 매 반동마다 좌우 랜덤 방향 설정 (-1 or +1)
        YawDirection = (rand() % 2 == 0) ? 1.f : -1.f;
    }
    
    ElapsedTime += DeltaTime;
    if (ElapsedTime >= Duration)
    {
        ViewInfo->ViewLocation = OriginLocation;
        ViewInfo->ViewRotation = OriginRotation;
        ElapsedTime = 0.f;
        bStartShake = false;
        bDisable = true; // 자동 비활성화
        return false;
    }

    InternalPhase = ElapsedTime / Duration;
    float Offset = Amplitude * std::sin(InternalPhase * PI);  // 0 ~ 1 ~ 0

    // 반동 곡선: sin(π * t) → 튀었다가 천천히 돌아감
    float RecoilStrength = Amplitude * std::sin(Offset * PI);
    float YawShake = (Amplitude * 0.4f) * std::sin(InternalPhase * PI) * YawDirection;

    // 현재 목표 각도 계산 (Pitch만 위로 튀기기)
    FVector RecoilRotation = OriginRotation;
    RecoilRotation.Y -= RecoilStrength;
    RecoilRotation.Z += YawShake;

    // 부드럽게 보간해서 회복
    ViewInfo->ViewRotation = FMath::Lerp(ViewInfo->ViewRotation, RecoilRotation, 0.5f);

    return false;
}
