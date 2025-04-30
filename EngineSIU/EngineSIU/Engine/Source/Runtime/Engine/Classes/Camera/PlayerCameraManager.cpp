#include "PlayerCameraManager.h"

APlayerCameraManager::APlayerCameraManager()
{
    DefaultAspectRatio = 9.0f / 9.0f;
    bDefaultConstrainAspectRatio = true;
}


void APlayerCameraManager::GetLetterBoxViewport(
    int ScreenW, int ScreenH,
    int& OutX, int& OutY,
    int& OutW, int& OutH) const
{
    // 레터박스 비활성화 시 전체 화면
    if (!bDefaultConstrainAspectRatio)
    {
        OutX = 0; OutY = 0;
        OutW = ScreenW; OutH = ScreenH;
        return;
    }

    const float CurrentAspect = float(ScreenW) / float(ScreenH);
    const float TargetAspect  = DefaultAspectRatio;

    // 가로가 더 넓을 때 → 좌우에 검은 바 (레터박스)
    if (CurrentAspect > TargetAspect)
    {
        OutH = ScreenH;
        OutW = int(TargetAspect * ScreenH + 0.5f);
        OutX = (ScreenW - OutW) / 2;
        OutY = 0;
    }
    // 세로가 더 넓을 때 → 상하에 검은 바 (필러박스)
    else if (CurrentAspect < TargetAspect)
    {
        OutW = ScreenW;
        OutH = int(ScreenW / TargetAspect + 0.5f);
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
