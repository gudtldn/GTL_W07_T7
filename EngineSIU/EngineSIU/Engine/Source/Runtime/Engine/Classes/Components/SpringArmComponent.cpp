#include "SpringArmComponent.h"
#include "UObject/Casts.h"
#include "Engine/Classes/GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Source/Runtime/InteractiveToolsFramework/BaseGizmos/GizmoBaseComponent.h"
#include <Engine/Engine.h>

USpringArmComponent::USpringArmComponent() 
    : TargetArmLength(3.f), 
    TargetOffset(FVector(0, 0, 0)), SocketOffset(FVector(0, 0, 3)), 
    bDoCollisionTest(true)
{

}

UObject* USpringArmComponent::Duplicate(UObject* InOuter) 
{ 
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->TargetArmLength = TargetArmLength;
    NewComponent->TargetOffset = TargetOffset;
    NewComponent->SocketOffset = SocketOffset;
    NewComponent->bDoCollisionTest = bDoCollisionTest;

    return NewComponent;
}

void USpringArmComponent::GetProperties(TMap<FString, FString>& OutProperties) const 
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("TargetArmLength"), FString::Printf(TEXT("%f"), TargetArmLength));
    OutProperties.Add(TEXT("TargetOffset"), TargetOffset.ToString());
    OutProperties.Add(TEXT("SocketOffset"), SocketOffset.ToString());
    OutProperties.Add(TEXT("bDoCollisionTest"), bDoCollisionTest ? TEXT("true") : TEXT("false"));
}

void USpringArmComponent::SetProperties(const TMap<FString, FString>& InProperties)
{ 
    Super::SetProperties(InProperties);

    const FString* TargetArmLengthStr = nullptr;

    TargetArmLengthStr = InProperties.Find(TEXT("TargetArmLength"));
    if (TargetArmLengthStr)
    {
        FString::ToFloat(*TargetArmLengthStr);
    }

    const FString* TargetOffsetStr = nullptr;
    if (TargetOffsetStr)
    {
        TargetOffset.InitFromString(*TargetOffsetStr);
    }

    const FString* SocketOffsetStr = nullptr;
    if (SocketOffsetStr)
    {
        TargetOffset.InitFromString(*SocketOffsetStr);
    }

    const FString* bDoCollisionTestStr = nullptr;
    bDoCollisionTestStr = InProperties.Find(TEXT("bDoCollisionTest"));
    if (bDoCollisionTestStr)
    {
        bDoCollisionTest = (*bDoCollisionTestStr == TEXT("true"));
    }
}

void USpringArmComponent::TickComponent(float DeltaTime) 
{ 
    Super::TickComponent(DeltaTime);

    if (!GetOwner())
        return;

    // Compute world-space origin (target point)
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    // Acotr의 크기가 변하면 적용되도록
    FVector WorldTarget = OwnerLocation + TargetOffset * GetOwner()->GetActorScale();

    // Arm direction (negative forward) in world
    FVector Forward = GetOwner()->GetActorForwardVector();
    FVector DesiredEndNoOffset = WorldTarget - Forward * TargetArmLength;

    FVector FinalEnd = DesiredEndNoOffset;

    FVector RayDirection = Forward * -1.0f;
    float Distance;
    if (bDoCollisionTest && GetWorld())
    { 
        if (GetObstacleDistance(WorldTarget, RayDirection, Distance) && Distance < TargetArmLength)
        {
            // Ray 검사에서 닿은 대상이 TargetArmLength 보다 가까이 있는 경우
            FinalEnd = WorldTarget - Forward * Distance;
        }
    }

    // Apply socket offset in world space
    const FVector WorldSocketOffset = SocketOffset * GetOwner()->GetActorScale();
    SpringArmLocation = FinalEnd + WorldSocketOffset;    
}

bool USpringArmComponent::GetObstacleDistance(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{ 
    const UStaticMeshComponent* Possible = nullptr;
    int maxIntersect = 0;
    float minDistance = FLT_MAX;


    for (const auto iter : TObjectRange<UStaticMeshComponent>())
    {
        if (iter->GetWorld() != GEngine->ActiveWorld)
        {
            continue;
        }

        // 자기 자신 빼기
        if (iter && iter->GetOwner() && iter->GetOwner() == GetOwner())
        {
            continue;
        }

        if (iter && !iter->IsA<UGizmoBaseComponent>())
        {
            float Distance = 0.0f;
            int currentIntersectCount = 0;
            if (iter->CheckRayIntersectionWorld(rayOrigin, rayDirection, Distance) > 0)
            {
                if (Distance < minDistance)
                {
                    minDistance = Distance;
                    maxIntersect = currentIntersectCount;
                    Possible = iter;
                }
                else if (abs(Distance - minDistance) < FLT_EPSILON && currentIntersectCount > maxIntersect)
                {
                    maxIntersect = currentIntersectCount;
                    Possible = iter;
                }
            }
        }
    }

    pfNearHitDistance = minDistance;

    return (maxIntersect == 0);
}
