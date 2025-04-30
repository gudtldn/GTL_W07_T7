#pragma once
#include "TextComponent.h"

class UTextUUID : public UTextComponent
{
    DECLARE_CLASS(UTextUUID, UTextComponent)

public:
    UTextUUID();

    virtual int CheckRayIntersectionLocal(
        FVector& rayOrigin,
        FVector& rayDirection, float& pfNearHitDistance
    ) override;
    void SetUUID(uint32 UUID);
};
