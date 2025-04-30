#pragma once
#include "GameFramework/Actor.h"

struct FViewportCamera;
class UCameraModifier;
class APlayerController;


class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)

public:
    APlayerCameraManager();

    UObject* Duplicate(UObject* InOuter) override;
    
    void Tick(float DeltaTime) override;
    void UpdateCamera(float DeltaTime);
public:
    /** APlayerCameraManager를 소유하고 있는 APlayerController */
    APlayerController* PCOwner;

    FViewportCamera* ViewCamera;
    
    TArray<UCameraModifier*> ModifierList;

    /* LetterBox 관련 변수 */
    float DefaultAspectRatio;
    bool bDefaultConstrainAspectRatio ;

    /**
    * 화면 크기(ScreenW×ScreenH)에 대해,
    * 레터박스 적용 후 실제 렌더링할 Viewport 영역을 계산.
    */
    void GetLetterBoxViewport(
        int ScreenW, int ScreenH,
        int& OutX, int& OutY,
        int& OutW, int& OutH) const;
};
