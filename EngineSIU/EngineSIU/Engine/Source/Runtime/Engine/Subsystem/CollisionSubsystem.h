#pragma once
#include "Container/Pair.h"
#include "Container/Set.h"

class UPrimitiveComponent;

class FCollisionSubsystem
{
public:
    void RegisterComponent(UPrimitiveComponent* NewComponent);
    void UnregisterComponent(UPrimitiveComponent* WantsComponent);

    void Tick();

    TArray<TPair<UPrimitiveComponent*, UPrimitiveComponent*>> BroadPhaseCullByBrute(const TArray<UPrimitiveComponent*>& Comps);
    
private:
    TArray<UPrimitiveComponent*> Components;
    TArray<TPair<UPrimitiveComponent*, UPrimitiveComponent*>> CurrentlyOverlapping;
};
