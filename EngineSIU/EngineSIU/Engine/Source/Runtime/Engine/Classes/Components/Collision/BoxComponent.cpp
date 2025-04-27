#include "BoxComponent.h"
#include "SphereComponent.h"
#include "CapsuleComponent.h"

#include "Math/FCollision.h"
#include "UObject/Casts.h"

UBoxComponent::UBoxComponent()
{
    BoxExtent = FVector(1.f, 1.f, 1.f);
}

UBoxComponent::~UBoxComponent()
{
    GEngineLoop.GetCollisionSubsystem()->UnregisterComponent(this);
}

UObject* UBoxComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->BoxExtent = BoxExtent;
    
    return NewComponent;
}

void UBoxComponent::InitializeComponent()
{
    Super::InitializeComponent();
    
    GEngineLoop.GetCollisionSubsystem()->RegisterComponent(this);
}

void UBoxComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UBoxComponent::SetBoxExtent(FVector InBoxExtent)
{
    BoxExtent = InBoxExtent;
}

bool UBoxComponent::IntersectCollision(const UPrimitiveComponent* Other)
{
    // Box
    if (UBoxComponent* OtherBox = Cast<UBoxComponent>(Other))
    {
        return FCollision::CheckOverlapBoxToBox(*this, *OtherBox);
    }

    // Sphere
    if (USphereComponent* OtherSphere = Cast<USphereComponent>(Other))
    {
        return FCollision::CheckOverlapBoxToSphere(*this, *OtherSphere);
    }

    // Capsule
    if (UCapsuleComponent* OtherCapsule = Cast<UCapsuleComponent>(Other))
    {
        return FCollision::CheckOverlapBoxToCapsule(*this, *OtherCapsule);
    }

    return false;
}
