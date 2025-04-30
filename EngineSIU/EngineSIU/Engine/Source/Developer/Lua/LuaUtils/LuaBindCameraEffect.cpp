#include "LuaBindCameraEffect.h"
#include <UObject/UObjectIterator.h>
#include <Engine/Engine.h>
#include "LuaBindMacros.h"

sol::table LuaBindCameraEffect::Bind(sol::table& parentTable)
{ 
    sol::table CameraEffectTable = parentTable.create_named("CameraEffect");

    CameraEffectTable.set_function(LUA_BIND_MEMBER(&LuaBindCameraEffect::StartCameraFade));
    CameraEffectTable.set_function(LUA_BIND_MEMBER(&LuaBindCameraEffect::GetFadeAmount));
    

    return CameraEffectTable;
}

void LuaBindCameraEffect::StartCameraFade(
    float FromAlpha, float ToAlpha, float Duration, FLinearColor Color, bool bShouldFadeAudio, bool bHoldWhenFinished
)
{
    // TODO GameMode 만들어지면 거기에서 접근하도록 수정
    for (const auto iter : TObjectRange<APlayerCameraManager>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            iter->StartCameraFade(FromAlpha, ToAlpha, Duration, Color, bShouldFadeAudio, bHoldWhenFinished);
        }
    }
}

float LuaBindCameraEffect::GetFadeAmount()
{
    // TODO GameMode 만들어지면 거기에서 접근하도록 수정
    for (const auto iter : TObjectRange<APlayerCameraManager>())
    {
        if (iter->GetWorld() == GEngine->ActiveWorld)
        {
            return iter->GetFadeAmount();
        }
    }
}
