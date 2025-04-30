#pragma once
#include "Components/SceneComponent.h"


class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();

    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& Properties) override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersectionLocal(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;

protected:
    FBoundingBox AABB;

public:
    FBoundingBox GetBoundingBox() const { return AABB; }
};
