#pragma once
#include "PrimitiveComponent.h"

class UShapeComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UShapeComponent, UPrimitiveComponent)

public:
    UShapeComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;
    
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
public:
    /** Colour used to draw the shape */
    FColor ShapeColor;

    /** Only show this component if the actor is selected */
    bool bDrawOnlyIfSelected;

    /**
     * [Original Property]
     * If true, registered this collision in collision subsystem.
     */
    bool bRegisterCollision;
};
