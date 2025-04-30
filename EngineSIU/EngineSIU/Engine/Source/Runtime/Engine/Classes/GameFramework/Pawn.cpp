#include "Pawn.h"
#include "Controller.h"


void APawn::PossessedBy(AController* NewController)
{
    SetOwner(NewController);

    AController* const OldController = Controller;
    Controller = NewController;

    if (OldController != NewController)
    {
        OnPawnControllerChangedDelegates.Broadcast(this, Controller);
    }
}

void APawn::UnPossessed()
{
    SetOwner(nullptr);
    Controller = nullptr;

    OnPawnControllerChangedDelegates.Broadcast(this, Controller);
}

// UInputComponent* APawn::CreatePlayerInputComponent()
// {
//     static const FName InputComponentName(TEXT("PawnInputComponent0"));
//     const UClass* OverrideClass = OverrideInputComponentClass.Get();
//     return NewObject<UInputComponent>(this, OverrideClass ? OverrideClass : UInputSettings::GetDefaultInputComponentClass(), InputComponentName);
// }
//
// void APawn::DestroyPlayerInputComponent()
// {
//     if (InputComponent)
//     {
//         InputComponent->DestroyComponent();
//         InputComponent = nullptr;
//     }
// }
