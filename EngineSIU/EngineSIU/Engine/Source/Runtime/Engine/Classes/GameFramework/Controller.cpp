#include "Controller.h"

void AController::SetPawn(APawn* NewPawn)
{
    // TODO: 나중에 Controller.cpp:526 참조
    Pawn = NewPawn;
}

void AController::Possess(APawn* InPawn)
{
    APawn* CurrentPawn = GetPawn();

    // A notification is required when the current assigned pawn is not possessed (i.e. pawn assigned before calling Possess)
    const bool bNotificationRequired = (CurrentPawn != nullptr) && (CurrentPawn->GetController() == nullptr);

    OnPossess(InPawn);

    APawn* NewPawn = GetPawn();
    if ((NewPawn != CurrentPawn) || bNotificationRequired)
    {
        OnPossessedPawnChanged.Broadcast(bNotificationRequired ? nullptr : CurrentPawn, NewPawn);
    }
}

void AController::UnPossess()
{
    APawn* CurrentPawn = GetPawn();
    if (CurrentPawn == nullptr)
    {
        return;
    }

    OnUnPossess();

    APawn* NewPawn = GetPawn();
    if (NewPawn != CurrentPawn)
    {
        OnPossessedPawnChanged.Broadcast(CurrentPawn, NewPawn);
    }
}

void AController::OnPossess(APawn* InPawn)
{
    const bool bNewPawn = GetPawn() != InPawn;
    if (bNewPawn && GetPawn() != nullptr)
    {
        UnPossess();
    }

    if (InPawn == nullptr)
    {
        return;
    }

    if (InPawn->Controller != nullptr)
    {
        InPawn->Controller->UnPossess();
    }

    InPawn->PossessedBy(this);
    SetPawn(InPawn);

    // SetControlRotation(Pawn->GetActorRotation());
}

void AController::OnUnPossess()
{
    if (Pawn != nullptr)
    {
        Pawn->UnPossessed();
        SetPawn(nullptr);
    }
}

void AController::AttachToPawn(APawn* InPawn)
{
    if (InPawn)
    {
        // TODO: 나중에 AttachToPawn 구현
        // Controller.cpp:212 참조
    }
}

void AController::DetachFromPawn()
{
    // TODO: 나중에 DetachFromPawn 구현
    // Controller.cpp:229 참조
}
