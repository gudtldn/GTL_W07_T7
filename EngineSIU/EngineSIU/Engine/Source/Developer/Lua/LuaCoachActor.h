#pragma once

#include "LuaActor.h"

class UBoxComponent;

class ALuaCoachActor : public ALuaActor
{
    DECLARE_CLASS(ALuaCoachActor, ALuaActor)

public:
    ALuaCoachActor();

    UBoxComponent* BoxComponent;
};
