#pragma once

#include "LuaActor.h"

class URigidbodyComponent;

class ALuaHeartActor : public ALuaActor
{
    DECLARE_CLASS(ALuaHeartActor, ALuaActor)

public:
    ALuaHeartActor();


    URigidbodyComponent* RigidbodyComp;
};
