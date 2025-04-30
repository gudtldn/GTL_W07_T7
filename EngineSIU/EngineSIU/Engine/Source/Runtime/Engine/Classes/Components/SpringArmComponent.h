#pragma once

#include "SceneComponent.h"
#include "Engine/Source/Runtime/Core/Math/Vector.h"

class USpringArmComponent : public USceneComponent
{
    DECLARE_CLASS(USpringArmComponent, USceneComponent)

public:
    USpringArmComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    virtual void TickComponent(float DeltaTime) override;

    FVector GetSpringArmLocation() const { return SpringArmLocation; }
    
    bool GetObstacleDistance(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);

private:
    float TargetArmLength;
    FVector TargetOffset;
    FVector SocketOffset;
    bool bDoCollisionTest;

    FVector SpringArmLocation;
};
