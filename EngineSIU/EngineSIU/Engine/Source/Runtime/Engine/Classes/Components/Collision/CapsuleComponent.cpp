#include "CapsuleComponent.h"
#include "BoxComponent.h"
#include "SphereComponent.h"
#include "Engine/Engine.h"
#include "Math/FCollision.h"
#include "UObject/Casts.h"
#include "World/World.h"

UCapsuleComponent::UCapsuleComponent()
{
    ShapeColor = FColor(223, 149, 157, 255);

    CapsuleRadius = 2.0f;
    CapsuleHalfHeight = 4.0f;
    
    UpdateAABB();
}

UCapsuleComponent::~UCapsuleComponent()
{
    if (bRegisterCollision)
    {
        GEngineLoop.GetCollisionSubsystem()->UnregisterComponent(this);
    }
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
}

void UCapsuleComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GEngine->ActiveWorld->WorldType == EWorldType::PIE)
    {
        GEngineLoop.GetCollisionSubsystem()->RegisterComponent(this);
        bRegisterCollision = true;
    }
}

void UCapsuleComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UCapsuleComponent::SetCapsuleSize(float InRadius, float InHalfHeight)
{
    CapsuleHalfHeight = FMath::Max(InHalfHeight, InRadius);
    CapsuleRadius = InRadius;

    UpdateAABB();
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
