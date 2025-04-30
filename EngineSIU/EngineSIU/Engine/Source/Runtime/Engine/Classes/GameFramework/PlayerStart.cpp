#include "PlayerStart.h"
#include "Components/Collision/CapsuleComponent.h"
#include "InteractiveToolsFramework/BaseGizmos/GizmoArrowComponent.h"


APlayerStart::APlayerStart()
{
    CapsuleComponent = AddComponent<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->SetCapsuleSize(40.0f, 92.0f);
    CapsuleComponent->SetCollisionEnabled(false);
    SetRootComponent(CapsuleComponent);
}
