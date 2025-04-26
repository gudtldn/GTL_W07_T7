#pragma once
#include "Components/SceneComponent.h"

class UPrimitiveComponent : public USceneComponent
{
    DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

public:
    UPrimitiveComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    bool IntersectRayTriangle(
        const FVector& rayOrigin, const FVector& rayDirection,
        const FVector& v0, const FVector& v1, const FVector& v2, float& hitDistance
    ) const;

    
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;

    virtual bool IsZeroExtent() const;


protected:
    /** The type of primitive. */
    FString Type;

    /** Bounding box used for spatial queries such as culling or collision detection. */
    FBoundingBox AABB;

public:
    void SetType(const FString& InType);

    /** @return the primitive type. */
    FORCEINLINE FString GetType() { return Type; }

    /** @return the aligned-axis bounding box of this primitive */
    FORCEINLINE FBoundingBox GetBoundingBox() const { return AABB; }


    /**
     * If true, generates overlap events when component initialize.
     */
    bool bGenerateOverlapEvents = true;

    /**
     * If true, detect other component.
     */
    bool bBlockComponent = true;
};

