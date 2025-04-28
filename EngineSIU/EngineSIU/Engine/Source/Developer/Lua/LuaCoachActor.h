#pragma once

#include "LuaActor.h"

class UCapsuleComponent;

class ALuaCoachActor : public ALuaActor
{
    DECLARE_CLASS(ALuaCoachActor, ALuaActor)

public:
    ALuaCoachActor();

    UCapsuleComponent* CapsuleComponent;
};
