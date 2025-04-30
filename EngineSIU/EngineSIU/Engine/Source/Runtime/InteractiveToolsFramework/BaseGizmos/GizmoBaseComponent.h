#pragma once
#include "Components/StaticMeshComponent.h"


class UGizmoBaseComponent : public UStaticMeshComponent
{
    DECLARE_CLASS(UGizmoBaseComponent, UStaticMeshComponent)

public:
    enum EGizmoType : uint8
    {
        ArrowX,
        ArrowY,
        ArrowZ,
        CircleX,
        CircleY,
        CircleZ,
        ScaleX,
        ScaleY,
        ScaleZ
    };
    
public:
    UGizmoBaseComponent() = default;

    virtual int CheckRayIntersectionLocal(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;

    virtual void TickComponent(float DeltaTime) override;

private:
    EGizmoType GizmoType;

public:
    EGizmoType GetGizmoType() const { return GizmoType; }
    
    void SetGizmoType(EGizmoType InGizmoType) { GizmoType = InGizmoType; }

    float GizmoScale = 0.3f;
};
