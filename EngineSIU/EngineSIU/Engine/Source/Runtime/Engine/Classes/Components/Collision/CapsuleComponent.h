#pragma once
#include "Components/ShapeComponent.h"

class UCapsuleComponent : public UShapeComponent
{
    DECLARE_CLASS(UCapsuleComponent, UShapeComponent)

public:
    UCapsuleComponent();
    ~UCapsuleComponent() override;

    virtual UObject* Duplicate(UObject* InOuter) override;
    
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual bool IntersectCollision(const UPrimitiveComponent* Other) override;
    
protected:
    /** Half-height, i.e. from center of capsule to end of top or bottom hemisphere. */
    float CapsuleHalfHeight;

    /** Radius of cap hemisphere and center cylinder. */
    float CapsuleRadius;

public:
    /**
     * Change the capsule size. This is the unscaled size
     */
    void SetCapsuleSize(float InRadius, float InHalfHeight);

    /**
     * Set the capsule radius. This is the unscaled radius.
     * @param InRadius radius of end-cap hemispheres and center cylinder.
     */
    FORCEINLINE void SetCapsuleRadius(float InRadius);

    /**
     * Set the capsule half-height. This is the unscaled half-height.
     * @param InHalfHeight half-height, from capusle center to end of top or bottom hemisphere.
     */
    FORCEINLINE void SetCapsuleHalfHeight(float InHalfHeight);

    FORCEINLINE float GetScaledCapsuleHalfHeight() const;

    FORCEINLINE float GetScaledCapsuleRadius() const;

    FORCEINLINE float GetUnScaledCapsuleHalfHeight() const;

    FORCEINLINE float GetUnScaledCapsuleRadius() const;

    FORCEINLINE float GetShapeScale() const;

    FORCEINLINE void UpdateAABB();
};

// ********** INLINES **********

FORCEINLINE void UCapsuleComponent::SetCapsuleRadius(float InRadius)
{
    CapsuleRadius = InRadius;
}

FORCEINLINE void UCapsuleComponent::SetCapsuleHalfHeight(float InHalfHeight)
{
    CapsuleHalfHeight = InHalfHeight;
}

FORCEINLINE float UCapsuleComponent::GetScaledCapsuleHalfHeight() const
{
    return CapsuleHalfHeight * GetShapeScale();
}

FORCEINLINE float UCapsuleComponent::GetScaledCapsuleRadius() const
{
    return CapsuleRadius * GetShapeScale();
}

FORCEINLINE float UCapsuleComponent::GetUnScaledCapsuleHalfHeight() const
{
    return CapsuleHalfHeight;
}

FORCEINLINE float UCapsuleComponent::GetUnScaledCapsuleRadius() const
{
    return CapsuleRadius;
}

FORCEINLINE float UCapsuleComponent::GetShapeScale() const
{
    return GetWorldScale3D().GetAbsMin();
}

FORCEINLINE void UCapsuleComponent::UpdateAABB()
{
    FBoundingBox newBox;
    newBox.min = FVector(-1.0f * CapsuleRadius, -1.0f * CapsuleRadius, -1.0f * (CapsuleRadius + CapsuleHalfHeight));
    newBox.max = FVector(CapsuleRadius, CapsuleRadius, CapsuleRadius + CapsuleHalfHeight);

    SetBoundingBox(newBox);
}
