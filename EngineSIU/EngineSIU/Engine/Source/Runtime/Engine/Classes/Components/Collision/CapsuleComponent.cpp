#include "CapsuleComponent.h"

#include "UObject/Casts.h"

UCapsuleComponent::UCapsuleComponent()
{
    ShapeColor = FColor(223, 149, 157, 255);

    CapsuleRadius = 22.0f;
    CapsuleHalfHeight = 44.0f;
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

void UCapsuleComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UCapsuleComponent::SetCapsuleSize(float InRadius, float InHalfHeight)
{
    CapsuleHalfHeight = FMath::Max(InHalfHeight, InRadius);
    CapsuleRadius = InRadius;
}
