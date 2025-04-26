#include "FireballActor.h"
#include "Engine/FLoaderOBJ.h"

#include "Components/Light/PointLightComponent.h"

#include "Components/ProjectileMovementComponent.h"

#include "Components/SphereComp.h"
#include "Components/Collision/BoxComponent.h"
#include "Components/Collision/SphereComponent.h"
#include "Components/Collision/CapsuleComponent.h"

AFireballActor::AFireballActor()
{
    FManagerOBJ::CreateStaticMesh("Contents/Sphere.obj");
    
    SphereComp = AddComponent<USphereComp>(TEXT("SphereComp"));
    SphereComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Sphere.obj"));
    RootComponent = SphereComp;
  
    PointLightComponent = AddComponent<UPointLightComponent>(TEXT("PointLightComponent"));
    PointLightComponent->SetLightColor(FLinearColor::Red);
    PointLightComponent->AttachToComponent(RootComponent);

    ProjectileMovementComponent = AddComponent<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->SetGravity(0);
    ProjectileMovementComponent->SetVelocity(FVector(100, 0, 0));
    ProjectileMovementComponent->SetInitialSpeed(100);
    ProjectileMovementComponent->SetMaxSpeed(100);
    ProjectileMovementComponent->SetLifetime(10);

    BoxComponent = AddComponent<UBoxComponent>(TEXT("BoxComponent"));
    BoxComponent->AttachToComponent(RootComponent);

    SphereComponent = AddComponent<USphereComponent>(TEXT("SphereComponent1"));
    SphereComponent->AttachToComponent(RootComponent);

    CapsuleComponent = AddComponent<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->AttachToComponent(RootComponent);
    
}

void AFireballActor::BeginPlay()
{
}
