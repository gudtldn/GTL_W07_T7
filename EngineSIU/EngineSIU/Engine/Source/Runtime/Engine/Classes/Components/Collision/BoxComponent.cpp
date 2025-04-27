#include "BoxComponent.h"
#include "SphereComponent.h"
#include "CapsuleComponent.h"
#include "Engine/Engine.h"

#include "Math/FCollision.h"
#include "UObject/Casts.h"
#include "World/World.h"

UBoxComponent::UBoxComponent()
{
    BoxExtent = FVector(1.f, 1.f, 1.f);

    AABB.max = {1.0f, 1.0f, 1.0f };
    AABB.min = { -1.f, -1.f, -1.f };
}

UBoxComponent::~UBoxComponent()
{
    if (bRegisterCollision)
    {
        GEngineLoop.GetCollisionSubsystem()->UnregisterComponent(this);
    }
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
}

void UBoxComponent::BeginPlay()
{
    Super::BeginPlay();

    if (GEngine->ActiveWorld->WorldType == EWorldType::PIE)
    {
        GEngineLoop.GetCollisionSubsystem()->RegisterComponent(this);
        bRegisterCollision = true;
    }
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
