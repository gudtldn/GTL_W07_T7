#pragma once
#include "Actor.h"


class UCapsuleComponent;
class UGizmoArrowComponent;

class APlayerStart : public AActor
{
    DECLARE_CLASS(APlayerStart, AActor)

public:
    APlayerStart();

public:
    UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }

public:
    UCapsuleComponent* CapsuleComponent;

    FName PlayerStartTag;
};
