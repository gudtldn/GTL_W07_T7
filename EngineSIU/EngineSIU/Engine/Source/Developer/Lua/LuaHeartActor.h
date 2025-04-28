#pragma once

#include "LuaActor.h"

class URigidbodyComponent;
class USphereComponent;

class ALuaHeartActor : public ALuaActor
{
    DECLARE_CLASS(ALuaHeartActor, ALuaActor)

public:
    ALuaHeartActor();

    USphereComponent* SphereComp;
    URigidbodyComponent* RigidbodyComp;

    void SetPlayerIndex(int InPlayerIndex);
};
