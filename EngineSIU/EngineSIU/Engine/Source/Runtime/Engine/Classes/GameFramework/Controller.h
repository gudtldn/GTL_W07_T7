#pragma once
#include "Actor.h"
#include "Pawn.h"

class APawn;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPossessedPawnChanged, APawn* OldPawn, APawn* NewPawn)


class AController : public AActor
{
    DECLARE_CLASS(AController, AActor)

public:
    AController() = default;

    /** Getter for Pawn */
    FORCEINLINE APawn* GetPawn() const { return Pawn; }

    /** Templated version of GetPawn, will return nullptr if cast fails */
    template <typename T>
    T* GetPawn() const
    {
        return Cast<T>(Pawn);
    }

    virtual void SetPawn(APawn* NewPawn);

    virtual void Possess(APawn* InPawn);
    virtual void UnPossess();

    FOnPossessedPawnChanged OnPossessedPawnChanged;

protected:
    virtual void OnPossess(APawn* InPawn);
    virtual void OnUnPossess();

protected:
    virtual void AttachToPawn(APawn* InPawn);
    virtual void DetachFromPawn();

private:
    /** Controller가 현재 조종중인 Pawn */
    APawn* Pawn = nullptr;
};
