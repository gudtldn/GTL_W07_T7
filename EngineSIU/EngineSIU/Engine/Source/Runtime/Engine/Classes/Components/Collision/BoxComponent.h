#pragma once
#include "Components/ShapeComponent.h"

class UBoxComponent : public UShapeComponent
{
    DECLARE_CLASS(UBoxComponent, UShapeComponent)

public:
    UBoxComponent();
    ~UBoxComponent() override;

    virtual UObject* Duplicate(UObject* InOuter) override;
    
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    bool IntersectCollision(const UPrimitiveComponent* Other) override;
    
protected:
    FVector BoxExtent;

public:
    void SetBoxExtent(FVector InBoxExtent);

    /**
     * @return the box extent, scaled by the component scale.
     */
    FORCEINLINE FVector GetScaledBoxExtent() const;

    /**
     * @return the box extent, ignoring component scale.
     */
    FORCEINLINE FVector GetUnscaledBoxExtent() const;
};


// ********** INLINES **********

FORCEINLINE FVector UBoxComponent::GetScaledBoxExtent() const
{
    return BoxExtent * GetWorldScale3D();
}

FORCEINLINE FVector UBoxComponent::GetUnscaledBoxExtent() const
{
    return BoxExtent;
}
