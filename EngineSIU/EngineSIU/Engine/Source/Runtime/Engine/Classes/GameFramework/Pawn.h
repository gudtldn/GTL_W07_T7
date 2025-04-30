#pragma once
#include "Actor.h"

class AController;


class APawn : public AActor
{
    DECLARE_CLASS(APawn, AActor)

public:
    APawn() = default;

public:
    FORCEINLINE AController* GetController() const { return Controller; }

    template <typename T>
    T* GetController() const
    {
        return Cast<T>(GetController());
    }

protected:
    AController* Controller;
};
