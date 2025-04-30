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

#if WITH_EDITOR && 0 // TODO: 나중에 ArrowComponent 사용해서 Rotation 표시
    UGizmoArrowComponent* GizmoArrowComponent;
#endif
};
