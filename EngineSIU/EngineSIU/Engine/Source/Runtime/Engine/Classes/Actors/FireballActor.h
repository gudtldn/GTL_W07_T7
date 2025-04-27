#pragma once
#include"GameFramework/Actor.h"

class UCapsuleComponent;
class USphereComponent;
class UBoxComponent;
class UProjectileMovementComponent;
class UPointLightComponent;
class USphereComp;

class AFireballActor : public AActor
{
    DECLARE_CLASS(AFireballActor, AActor)

public:
    AFireballActor();

    virtual void BeginPlay() override;

protected:
    UPROPERTY
    (UProjectileMovementComponent*, ProjectileMovementComponent, = nullptr)

    UPROPERTY
    (UPointLightComponent*, PointLightComponent, = nullptr)

    UPROPERTY
    (USphereComp*, SphereComp, = nullptr)

    UPROPERTY(UBoxComponent*, BoxComponent, = nullptr)

    UPROPERTY(USphereComponent*, SphereComponent, = nullptr)

    UPROPERTY(UCapsuleComponent*, CapsuleComponent, = nullptr)
};
