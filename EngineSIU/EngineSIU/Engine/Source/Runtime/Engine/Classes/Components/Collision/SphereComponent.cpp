#include "SphereComponent.h"

#include "UObject/Casts.h"

USphereComponent::USphereComponent()
{
    SphereRadius = 32.0f;
    ShapeColor = FColor(255, 0, 0, 255);
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

void USphereComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void USphereComponent::SetSphereRadius(float InRadius)
{
    SphereRadius = InRadius;
}
