#pragma once
#include "Actor.h"

class APawn;


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

private:
    /** Controller가 현재 조종중인 Pawn */
    APawn* Pawn = nullptr;
};
