#pragma once
#include "Components/ShapeComponent.h"

class USphereComponent : public UShapeComponent
{
    DECLARE_CLASS(USphereComponent, UShapeComponent)

public:
    USphereComponent();
    ~USphereComponent() override;
    
    virtual UObject* Duplicate(UObject* InOuter) override;
    
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual bool IntersectCollision(const UPrimitiveComponent* Other) override;
    
protected:
    /* The radius of the sphere */
    float SphereRadius;

public:
    /**
     * Change the sphere radius. This is unscaled size.
     * @param InRadius the new sphere radius.
     */
    void SetSphereRadius(float InRadius);

    /** @return the radius of the sphere, with component scale applied. */
    FORCEINLINE float GetScaledSphereRadius() const;

    /** @return the radius of the sphere, ignoring component scale. */
    FORCEINLINE float GetUnscaledSphereRadius() const;

    /** @return the scale used by this shape. */
    FORCEINLINE float GetShapeScale() const;
};

FORCEINLINE float USphereComponent::GetScaledSphereRadius() const
{
    return SphereRadius * GetShapeScale();
}

FORCEINLINE float USphereComponent::GetUnscaledSphereRadius() const
{
    return SphereRadius;
}

FORCEINLINE float USphereComponent::GetShapeScale() const
{
    return GetWorldScale3D().GetAbsMin();
}
