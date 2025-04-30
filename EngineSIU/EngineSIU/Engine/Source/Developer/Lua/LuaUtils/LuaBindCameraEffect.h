#pragma once

#include <sol/sol.hpp>
#include "Engine/Classes/Camera/PlayerCameraManager.h"

class LuaBindCameraEffect
{
public:
    static sol::table Bind(sol::table& parentTable);

private:
    static void StartCameraFade(
        float FromAlpha, float ToAlpha, 
        float Duration, 
        FLinearColor Color, 
        bool bShouldFadeAudio = false, 
        bool bHoldWhenFinished = true);

    static float GetFadeAmount();
};
