#include "CapsuleComponent.h"
#include "BoxComponent.h"
#include "SphereComponent.h"
#include "Math/FCollision.h"
#include "UObject/Casts.h"

UCapsuleComponent::UCapsuleComponent()
{
    ShapeColor = FColor(223, 149, 157, 255);

    CapsuleRadius = 22.0f;
    CapsuleHalfHeight = 44.0f;
}

UCapsuleComponent::~UCapsuleComponent()
{
    GEngineLoop.GetCollisionSubsystem()->UnregisterComponent(this);
}

UObject* UCapsuleComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->CapsuleRadius = CapsuleRadius;
    NewComponent->CapsuleHalfHeight = CapsuleHalfHeight;
    
    return NewComponent;
}

void UCapsuleComponent::InitializeComponent()
{
    Super::InitializeComponent();

    GEngineLoop.GetCollisionSubsystem()->RegisterComponent(this);
}

void UCapsuleComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UCapsuleComponent::SetCapsuleSize(float InRadius, float InHalfHeight)
{
    CapsuleHalfHeight = FMath::Max(InHalfHeight, InRadius);
    CapsuleRadius = InRadius;
}

bool UCapsuleComponent::IntersectCollision(const UPrimitiveComponent* Other)
{
    // Box
    if (UBoxComponent* OtherBox = Cast<UBoxComponent>(Other))
    {
        return FCollision::CheckOverlapBoxToCapsule(*OtherBox, *this);
    }

    // Sphere
    if (USphereComponent* OtherSphere = Cast<USphereComponent>(Other))
    {
        return FCollision::CheckOverlapSphereToCapsule(*OtherSphere, *this);
    }

    // Capsule
    if (UCapsuleComponent* OtherCapsule = Cast<UCapsuleComponent>(Other))
    {
        return FCollision::CheckOverlapCapsuleToCapsule(*this, *OtherCapsule);
    }

    return false;
}
