#include "PrimitiveComponent.h"

#include "UObject/Casts.h"


UObject* UPrimitiveComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->AABB = AABB;

    return NewComponent;
}

void UPrimitiveComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UPrimitiveComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
}

void UPrimitiveComponent::SetType(const FString& InType)
{
    Type = InType;
}

int UPrimitiveComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    //if (!AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance)) return 0;
    int nIntersections = 0;
    //if (staticMesh == nullptr) return 0;
    //FVertexSimple* vertices = staticMesh->vertices.get();
    //int vCount = staticMesh->numVertices;
    //UINT* indices = staticMesh->indices.get();
    //int iCount = staticMesh->numIndices;

    //if (!vertices) return 0;
    //BYTE* pbPositions = reinterpret_cast<BYTE*>(staticMesh->vertices.get());
    //
    //int nPrimitives = (!indices) ? (vCount / 3) : (iCount / 3);
    //float fNearHitDistance = FLT_MAX;
    //for (int i = 0; i < nPrimitives; i++) {
    //    int idx0, idx1, idx2;
    //    if (!indices) {
    //        idx0 = i * 3;
    //        idx1 = i * 3 + 1;
    //        idx2 = i * 3 + 2;
    //    }
    //    else {
    //        idx0 = indices[i * 3];
    //        idx2 = indices[i * 3 + 1];
    //        idx1 = indices[i * 3 + 2];
    //    }

    //    // 각 삼각형의 버텍스 위치를 FVector로 불러옵니다.
    //    uint32 stride = sizeof(FVertexSimple);
    //    FVector v0 = *reinterpret_cast<FVector*>(pbPositions + idx0 * stride);
    //    FVector v1 = *reinterpret_cast<FVector*>(pbPositions + idx1 * stride);
    //    FVector v2 = *reinterpret_cast<FVector*>(pbPositions + idx2 * stride);

    //    float fHitDistance;
    //    if (IntersectRayTriangle(rayOrigin, rayDirection, v0, v1, v2, fHitDistance)) {
    //        if (fHitDistance < fNearHitDistance) {
    //            pfNearHitDistance =  fNearHitDistance = fHitDistance;
    //        }
    //        nIntersections++;
    //    }
    //   
    //}
    return nIntersections;
}

bool UPrimitiveComponent::IntersectRayTriangle(const FVector& rayOrigin, const FVector& rayDirection, const FVector& v0, const FVector& v1, const FVector& v2, float& hitDistance) const
{
    constexpr float epsilon = 1e-6f;
    FVector edge1 = v1 - v0;
    const FVector edge2 = v2 - v0;
    FVector FrayDirection = rayDirection;
    FVector h = FrayDirection.Cross(edge2);
    float a = edge1.Dot(h);

    if (fabs(a) < epsilon)
        return false; // Ray와 삼각형이 평행한 경우

    float f = 1.0f / a;
    FVector s = rayOrigin - v0;
    float u = f * s.Dot(h);
    if (u < 0.0f || u > 1.0f)
        return false;

    FVector q = s.Cross(edge1);
    float v = f * FrayDirection.Dot(q);
    if (v < 0.0f || (u + v) > 1.0f)
        return false;

    float t = f * edge2.Dot(q);
    if (t > epsilon) {

        hitDistance = t;
        return true;
    }

    return false;
}

void UPrimitiveComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Type"), Type);
    OutProperties.Add(TEXT("AABB_min"), AABB.min.ToString());
    OutProperties.Add(TEXT("AABB_max"), AABB.max.ToString());
}


void UPrimitiveComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    const FString* TempStr = nullptr;

    // --- PrimitiveComponent 고유 속성 복원 ---

    TempStr = InProperties.Find(TEXT("Type"));
    if (TempStr)
    {
        this->Type = *TempStr; // Type이 FString이라고 가정
        // 만약 Type이 enum이라면 문자열로부터 enum 값을 파싱하는 로직 필요
    }

    const FString* AABBminStr = InProperties.Find(TEXT("AABB_min"));
    if (AABBminStr) AABB.min.InitFromString(*AABBminStr); 

    
    const FString* AABBmaxStr = InProperties.Find(TEXT("AABB_max"));
    if (AABBmaxStr) AABB.max.InitFromString(*AABBmaxStr); 
}

bool UPrimitiveComponent::IsZeroExtent() const
{
    return false;
}

bool UPrimitiveComponent::IntersectCollision(const UPrimitiveComponent* Other)
{
    return false;
}

void UPrimitiveComponent::BeginComponentOverlap(UPrimitiveComponent* OverlappedComponent)
{
    if (OverlappedComponent)
    {
        if (IsOverlappingComponent(OverlappedComponent))
        {
            // Already processed component.
            return;
        }

        OverlappingComponents.Add(OverlappedComponent);

        AActor* const MyActor = GetOwner();
        
        AActor* const OtherActor = OverlappedComponent->GetOwner();

        if (!IsPendingKill())
        {
            UE_LOG(ELogLevel::Display, "%s: BeginOverlap", *this->GetName());
            OnComponentBeginOverlap.Broadcast(OtherActor, OverlappedComponent);
        }
    }
}

void UPrimitiveComponent::EndComponentOverlap(UPrimitiveComponent* OverlappedComponent)
{
    AActor* const OtherActor = OverlappedComponent ? OverlappedComponent->GetOwner() : nullptr;
    AActor* const MyActor = GetOwner();

    if ( (OtherActor != nullptr) && IsOverlappingComponent(OverlappedComponent) && (MyActor != nullptr))
    {
        if (IsActive())
        {
            OnComponentEndOverlap.Broadcast(OtherActor, OverlappedComponent);
        }

        if (OverlappedComponent->IsActive())
        {
            UE_LOG(ELogLevel::Display, "%s: EndOverlap", *this->GetName());
            OverlappedComponent->OnComponentEndOverlap.Broadcast(MyActor, this);
        }
    }

    int32 OverlapIndex = OverlappingComponents.Find(OverlappedComponent);
    if (OverlapIndex != -1)
    {
        OverlappingComponents.RemoveAt(OverlapIndex);
    }

    if (OverlappedComponent)        
    {
        int32 OtherOverlapIndex = OverlappedComponent->OverlappingComponents.Find(this);
        if (OtherOverlapIndex != -1)
        {
            OverlappedComponent->OverlappingComponents.RemoveAt(OtherOverlapIndex);
        }
    }
}

bool UPrimitiveComponent::IsOverlappingActor(AActor* Other) const
{
    return false;
}

bool UPrimitiveComponent::IsOverlappingComponent(UPrimitiveComponent* Other) const
{
    for (int32 i = 0; i < OverlappingComponents.Num(); i++)
    {
        if (OverlappingComponents[i] == Other) { return true; }
    }
    return false;
}

const TArray<UPrimitiveComponent*>& UPrimitiveComponent::GetOverlappingComponents() const
{
    return OverlappingComponents;
}
