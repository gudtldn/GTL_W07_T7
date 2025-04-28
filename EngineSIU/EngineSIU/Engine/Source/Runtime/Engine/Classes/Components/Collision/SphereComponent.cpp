#include "SphereComponent.h"
#include "BoxComponent.h"
#include "CapsuleComponent.h"
#include "Engine/Engine.h"

#include "Math/FCollision.h"
#include "UObject/Casts.h"
#include "World/World.h"

USphereComponent::USphereComponent()
{
    SphereRadius = 1.0f;
    ShapeColor = FColor(255, 0, 0, 255);
    
    AABB.max = {1.0f, 1.0f, 1.0f };
    AABB.min = { -1.f, -1.f, -1.f };
}

USphereComponent::~USphereComponent()
{
    if (bRegisterCollision)
    {
        GEngineLoop.GetCollisionSubsystem()->UnregisterComponent(this);
    }
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
}

void USphereComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GEngine->ActiveWorld->WorldType == EWorldType::PIE)
    {
        GEngineLoop.GetCollisionSubsystem()->RegisterComponent(this);
        bRegisterCollision = true;
    }
}

void USphereComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void USphereComponent::SetSphereRadius(float InRadius)
{
    SphereRadius = InRadius;

    FBoundingBox BoundingBox;
    BoundingBox.min = {-SphereRadius, -SphereRadius, -SphereRadius};
    BoundingBox.max = {SphereRadius, SphereRadius, SphereRadius};
    SetBoundingBox(BoundingBox);
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
