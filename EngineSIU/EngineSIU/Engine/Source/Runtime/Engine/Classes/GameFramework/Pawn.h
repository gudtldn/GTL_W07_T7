#pragma once
#include "Actor.h"

class APawn;
class AController;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPawnControllerChanged, APawn* Pawn, AController* Controller)


class APawn : public AActor
{
    DECLARE_CLASS(APawn, AActor)

public:
    APawn() = default;

    virtual void PossessedBy(AController* NewController);
    virtual void UnPossessed();


    FOnPawnControllerChanged OnPawnControllerChangedDelegates;

public:
    FORCEINLINE AController* GetController() const { return Controller; }

    template <typename T>
    T* GetController() const
    {
        return Cast<T>(GetController());
    }

public:
    AController* Controller;
};
