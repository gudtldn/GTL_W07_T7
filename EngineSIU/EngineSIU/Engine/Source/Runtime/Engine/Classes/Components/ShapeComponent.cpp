#include "ShapeComponent.h"
#include "UObject/Casts.h"

UShapeComponent::UShapeComponent()
{
    ShapeColor = FColor(223, 149, 157, 255);
    bDrawOnlyIfSelected = true;
    bRegisterCollision = false;
}

UObject* UShapeComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->ShapeColor = ShapeColor;
    NewComponent->bDrawOnlyIfSelected = bDrawOnlyIfSelected;
    
    return NewComponent;
}

void UShapeComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UShapeComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
