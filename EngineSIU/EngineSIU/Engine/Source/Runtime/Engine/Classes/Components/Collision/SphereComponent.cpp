#include "SphereComponent.h"
#include "BoxComponent.h"
#include "CapsuleComponent.h"

#include "Math/FCollision.h"
#include "UObject/Casts.h"

USphereComponent::USphereComponent()
{
    SphereRadius = 1.0f;
    ShapeColor = FColor(255, 0, 0, 255);
}

USphereComponent::~USphereComponent()
{
    GEngineLoop.GetCollisionSubsystem()->UnregisterComponent(this);
}

UObject* USphereComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->SphereRadius = SphereRadius;
    
    return NewComponent;
}

void USphereComponent::InitializeComponent()
{
    Super::InitializeComponent();

    GEngineLoop.GetCollisionSubsystem()->RegisterComponent(this);
}

void USphereComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void USphereComponent::SetSphereRadius(float InRadius)
{
    SphereRadius = InRadius;
}

bool USphereComponent::IntersectCollision(const UPrimitiveComponent* Other)
{
    // Box
    if (UBoxComponent* OtherBox = Cast<UBoxComponent>(Other))
    {
        return FCollision::CheckOverlapBoxToSphere(*OtherBox, *this);
    }

    // Sphere
    if (USphereComponent* OtherSphere = Cast<USphereComponent>(Other))
    {
        return FCollision::CheckOverlapSphereToSphere(*this, *OtherSphere);
    }

    // Capsule
    if (UCapsuleComponent* OtherCapsule = Cast<UCapsuleComponent>(Other))
    {
        return FCollision::CheckOverlapSphereToCapsule(*this, *OtherCapsule);
    }
    
    return false;
}
