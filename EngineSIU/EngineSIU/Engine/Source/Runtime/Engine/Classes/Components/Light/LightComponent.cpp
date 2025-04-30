#include "LightComponent.h"
#include "UObject/Casts.h"

ULightComponentBase::ULightComponentBase()
{
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };
}

UObject* ULightComponentBase::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->AABB = AABB;

    return NewComponent;
}

void ULightComponentBase::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("AABB_Min"), AABB.min.ToString());
    OutProperties.Add(TEXT("AABB_Max"), AABB.max.ToString());
}

void ULightComponentBase::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    auto SetPropertyHelper = [&InProperties] <typename T, typename Fn>(const FString& Key, T& MemberVariable, const Fn& ConversionFunc)
    {
        if (const FString* TempStr = InProperties.Find(Key))
        {
            MemberVariable = ConversionFunc(*TempStr);
        }
    };

    SetPropertyHelper(TEXT("AABB_Min"), AABB.min, [](const FString& Str) { FVector Pos; Pos.InitFromString(Str); return Pos; });
    SetPropertyHelper(TEXT("AABB_Max"), AABB.max, [](const FString& Str) { FVector Pos; Pos.InitFromString(Str); return Pos; });
}

void ULightComponentBase::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int ULightComponentBase::CheckRayIntersectionLocal(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    bool res = AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
    return res;
}

