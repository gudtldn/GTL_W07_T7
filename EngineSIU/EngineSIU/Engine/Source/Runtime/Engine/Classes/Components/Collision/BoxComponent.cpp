#include "BoxComponent.h"

#include "UObject/Casts.h"

UBoxComponent::UBoxComponent()
{
    BoxExtent = FVector(1.f, 1.f, 1.f);
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

void UBoxComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UBoxComponent::SetBoxExtent(FVector InBoxExtent)
{
    BoxExtent = InBoxExtent;
}
