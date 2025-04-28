#pragma once

#include "LuaActor.h"

class UBoxComponent;

class ALuaCoachActor : public ALuaActor
{
    DECLARE_CLASS(ALuaCoachActor, ALuaActor)

public:
    ALuaCoachActor();

    UBoxComponent* BoxComponent;

    virtual void BeginPlay() override;

    void SetAffection(int NewAffection);

    int GetAffection() const;
    
private:
    int Affection;
};
