#include "LuaBindCameraEffect.h"
#include <UObject/UObjectIterator.h>
#include <Engine/Engine.h>

sol::table LuaBindCameraEffect::Bind(sol::table& parentTable)
{ 
    sol::table CameraEffectTable = parentTable.create_named("CameraEffect");

    CameraEffectTable.set_function("StartCameraFade", &LuaBindCameraEffect::StartCameraFade);

    return CameraEffectTable;
}

void LuaBindCameraEffect::StartCameraFade(
    float FromAlpha, float ToAlpha, float Duration, FLinearColor Color, bool bShouldFadeAudio, bool bHoldWhenFinished
)
{
    // TODO 현재 Player가 소유한 1개의 PlayerCameraManager를 정확히 가져올것
    for (const auto iter : TObjectRange<APlayerCameraManager>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            iter->StartCameraFade(FromAlpha, ToAlpha, Duration, Color, bShouldFadeAudio, bHoldWhenFinished);
        }
    }
}
