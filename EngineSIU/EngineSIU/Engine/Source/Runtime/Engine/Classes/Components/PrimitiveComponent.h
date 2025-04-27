#pragma once
#include "Components/SceneComponent.h"

class UPrimitiveComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FComponentBeginOverlapSignature, AActor* /* OtherActor */, UPrimitiveComponent* /* OtherComp */)
DECLARE_MULTICAST_DELEGATE_TwoParams(FComponentEndOverlapSignature, AActor* /* OtherActor */, UPrimitiveComponent* /* OtherComp */)
DECLARE_DELEGATE_FourParams(FComponentHitSignature, AActor* /* OtherActor */,  UPrimitiveComponent* /* OtherComp */, FVector /* NormalImpulse */,  const FHitResult& /* Hit */)

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

    virtual bool IntersectCollision(const UPrimitiveComponent* Other);

    void BeginComponentOverlap(UPrimitiveComponent* OverlappedComponent);

    void EndComponentOverlap(UPrimitiveComponent* OverlappedComponent);
    
    /** Week08 - Not implemented */
    bool IsOverlappingActor(AActor* Other) const;

    bool IsOverlappingComponent(UPrimitiveComponent* Other) const;

    const TArray<UPrimitiveComponent*>& GetOverlappingComponents() const;
protected:
    /** The type of primitive. */
    FString Type;

    /** Bounding box used for spatial queries such as culling or collision detection. */
    FBoundingBox AABB;

    /** Set of components that this component is currently overlapping. */
    TArray<UPrimitiveComponent*> OverlappingComponents;


public:
    void SetType(const FString& InType);

    /** @return the primitive type. */
    FORCEINLINE FString GetType() { return Type; }

    /** @return the aligned-axis bounding box of this primitive */
    FORCEINLINE FBoundingBox GetBoundingBox() const { return AABB; }

    FORCEINLINE void SetBoundingBox(const FBoundingBox& InBoundingBox) { AABB = InBoundingBox; }
    
    /**
     * If true, generates overlap events when component initialize.
     */
    bool bGenerateOverlapEvents = true;

    /**
     * If true, detect other component.
     */
    bool bBlockComponent = true;
    
    // Events
    FComponentBeginOverlapSignature OnComponentBeginOverlap;
    FComponentEndOverlapSignature OnComponentEndOverlap;
    FComponentHitSignature OnComponentHit;
};
