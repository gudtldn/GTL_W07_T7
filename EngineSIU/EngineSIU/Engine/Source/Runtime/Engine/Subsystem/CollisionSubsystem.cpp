#include "CollisionSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "Define.h"

void FCollisionSubsystem::RegisterComponent(UPrimitiveComponent* NewComponent)
{
    Components.Add(NewComponent);
}

void FCollisionSubsystem::UnregisterComponent(UPrimitiveComponent* WantsComponent)
{
    Components.Remove(WantsComponent);
}

void FCollisionSubsystem::Tick()
{
    if (Components.IsEmpty()) return;
    
    TArray<TPair<UPrimitiveComponent*, UPrimitiveComponent*>> NewOverlap;

    // Brute-force
    auto Candidates = BroadPhaseCullByBrute(Components);
    for ( auto& [A, B] : Candidates )
    {
        if (A->IntersectCollision(B))
        {
            NewOverlap.Add(MakePair(A, B));
        }
    }

    for (auto& P : NewOverlap)
    {
        if (!CurrentlyOverlapping.Contains(P))
        {
            auto [A, B] = P;
            
            if (A->IsPendingKill() || B->IsPendingKill()) continue;
            
            A->BeginComponentOverlap(B);
            B->BeginComponentOverlap(A);
        }
    }

    for (auto& P : CurrentlyOverlapping)
    {
        if (!NewOverlap.Contains(P))
        {
            auto [A, B] = P;

            if (A->IsPendingKill() || B->IsPendingKill()) continue;
            
            A->EndComponentOverlap(B);
            B->EndComponentOverlap(A);
        }
    }

    CurrentlyOverlapping.Empty();
    CurrentlyOverlapping = NewOverlap;
}

TArray<TPair<UPrimitiveComponent*, UPrimitiveComponent*>> FCollisionSubsystem::BroadPhaseCullByBrute(const TArray<UPrimitiveComponent*>& Comps)
{
    TArray<TPair<UPrimitiveComponent*, UPrimitiveComponent*>> Results;

    for (int32 i = 0; i < Comps.Num(); i++)
    {
        auto* A = Comps[i];
        FBoundingBox AABB = A->GetBoundingBox().TransformWorldIteration(A->GetWorldLocation(), A->GetWorldMatrix());

        for (int32 j = i + 1; j < Comps.Num(); j++)
        {
            auto* B = Comps[j];

            if (AABB.Intersect(B->GetBoundingBox().TransformWorldIteration(B->GetWorldLocation(), B->GetWorldMatrix())))
            {
                Results.Add(MakePair(A, B));
            }
        }
    }
    
    return Results;
}
