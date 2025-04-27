#include "FCollision.h"

#include <sstream>

#include "Quat.h"
#include "Components/Collision/BoxComponent.h"
#include "Components/Collision/CapsuleComponent.h"
#include "Components/Collision/SphereComponent.h"

bool FCollision::CheckOverlapBoxToBox(const UBoxComponent& Box1, const UBoxComponent& Box2)
{
    const FVector Box1Min = Box1.GetWorldLocation() - Box1.GetScaledBoxExtent();
    const FVector Box1Max = Box1.GetWorldLocation() + Box1.GetScaledBoxExtent();

    const FVector Box2Min = Box2.GetWorldLocation() - Box2.GetScaledBoxExtent();
    const FVector Box2Max = Box2.GetWorldLocation() + Box2.GetScaledBoxExtent();

    if (Box1Max.X < Box2Min.X || Box1Min.X > Box2Max.X) return false;
    if (Box1Max.Y < Box2Min.Y || Box1Min.Y > Box2Max.Y) return false;
    if (Box1Max.Z < Box2Min.Z || Box1Min.Z > Box2Max.Z) return false;

    return true;
}

bool FCollision::CheckOverlapSphereToSphere(const USphereComponent& Sphere1, const USphereComponent& Sphere2)
{
    const float Dist = (Sphere1.GetWorldLocation() - Sphere2.GetWorldLocation()).SquaredLength();
    const float Rad = Sphere1.GetScaledSphereRadius() + Sphere2.GetScaledSphereRadius();
    return Dist <= Rad * Rad; // distance of between s1 to s2 <= (r1 + r2)
}

bool FCollision::CheckOverlapCapsuleToCapsule(const UCapsuleComponent& Capsule1, const UCapsuleComponent& Capsule2)
{
    // Calculate capsule1 P0, P1
    FVector C1P0 = Capsule1.GetWorldLocation() + Capsule1.GetWorldRotation().ToQuaternion().RotateVector(FVector::UpVector) * Capsule1.GetScaledCapsuleHalfHeight();
    FVector C1P1 = Capsule1.GetWorldLocation() - Capsule1.GetWorldRotation().ToQuaternion().RotateVector(FVector::UpVector) * Capsule1.GetScaledCapsuleHalfHeight();

    // Calculate capsule2 P0, P1
    FVector C2P0 = Capsule2.GetWorldLocation() + Capsule2.GetWorldRotation().ToQuaternion().RotateVector(FVector::UpVector) * Capsule2.GetScaledCapsuleHalfHeight();
    FVector C2P1 = Capsule2.GetWorldLocation() - Capsule2.GetWorldRotation().ToQuaternion().RotateVector(FVector::UpVector) * Capsule2.GetScaledCapsuleHalfHeight();

    FVector u = C1P1 - C1P0;
    FVector v = C2P1 - C2P0;
    FVector w = C1P0 - C2P0;

    float a = u.Dot(u);
    float b = u.Dot(v);
    float c = v.Dot(v);
    float d = u.Dot(w);
    float e = v.Dot(w);
    float den = a * c - b * b;

    float s = 0.0f, t = 0.0f;

    if (den > 1e-6f)
    {
        s = (b*e - c*d) / den;
        s = FMath::Clamp(s, 0.0f, 1.0f);
    }

    t = (b*s + e) / c;

    if ( t < 0.0f )
    {
        t = 0.0f;
        s = FMath::Clamp(-d / a, 0.0f, 1.0f);
    }
    else if ( t > 1.0f )
    {
        t = 1.0f;
        s = FMath::Clamp((b - d) / a, 0.0f, 1.0f);
    }

    FVector cP = C1P0 + u * s;
    FVector cQ = C2P0 + v * t;
    
    float RSum = Capsule1.GetScaledCapsuleRadius() + Capsule2.GetScaledCapsuleRadius();
    float Dist2 = (cP - cQ).SquaredLength();
    return Dist2 <= RSum * RSum;
}

bool FCollision::CheckOverlapBoxToSphere(const UBoxComponent& Box1, const USphereComponent& Sphere1)
{
    const FVector BoxMin = Box1.GetWorldLocation() - Box1.GetScaledBoxExtent();
    const FVector BoxMax = Box1.GetWorldLocation() + Box1.GetScaledBoxExtent();
    
    FVector Closet;
    Closet.X = FMath::Max(BoxMin.X, FMath::Min(Sphere1.GetWorldLocation().X, BoxMax.X));
    Closet.Y = FMath::Max(BoxMin.Y, FMath::Min(Sphere1.GetWorldLocation().Y, BoxMax.Y));
    Closet.Z = FMath::Max(BoxMin.Z, FMath::Min(Sphere1.GetWorldLocation().Z, BoxMax.Z));

    float Dist = (Closet - Sphere1.GetWorldLocation()).SquaredLength();
    return Dist <= Sphere1.GetScaledSphereRadius() * Sphere1.GetScaledSphereRadius();
}

bool FCollision::CheckOverlapBoxToCapsule(const UBoxComponent& Box1, const UCapsuleComponent& Capsule1)
{
    // 1) 캡슐 축 끝점 P0, P1 계산 (월드 공간)
    const FVector center = Capsule1.GetWorldLocation();
    const FQuat   q = Capsule1.GetWorldRotation().ToQuaternion();
    // UpVector은 (0,0,1) 이라고 가정
    const FVector up = q.RotateVector(FVector::UpVector);
    const float   halfH = Capsule1.GetScaledCapsuleHalfHeight();
    const float   radius = Capsule1.GetScaledCapsuleRadius();

    const FVector P0 = center + up * halfH;
    const FVector P1 = center - up * halfH;

    // 2) AABB 구하기 (월드 공간) 및 반지름만큼 확장
    const FVector boxCenter = Box1.GetWorldLocation();
    const FVector extents = Box1.GetScaledBoxExtent();

    const FVector boxMin = {
        boxCenter.X - extents.X - radius,
        boxCenter.Y - extents.Y - radius,
        boxCenter.Z - extents.Z - radius
    };
    const FVector boxMax = {
        boxCenter.X + extents.X + radius,
        boxCenter.Y + extents.Y + radius,
        boxCenter.Z + extents.Z + radius
    };

    // 3) 선분 P0→P1 에 대한 방향 벡터 d 와 파라메터 t 범위
    const FVector d = { P1.X - P0.X, P1.Y - P0.Y, P1.Z - P0.Z };
    float tmin = 0.0f, tmax = 1.0f;
    const float eps = 1e-6f;

    // 4) 슬래브 기법: X, Y, Z 축별로 교차 파라미터 범위를 좁혀간다
    for (int i = 0; i < 3; ++i)
    {
        // i 번째 축 성분을 꺼낸다
        float a, mn, mx, di;
        if (i == 0) { a = P0.X; mn = boxMin.X; mx = boxMax.X; di = d.X; }
        else if (i == 1) { a = P0.Y; mn = boxMin.Y; mx = boxMax.Y; di = d.Y; }
        else { a = P0.Z; mn = boxMin.Z; mx = boxMax.Z; di = d.Z; }

        // 평행(거의 움직임 없음) 판정
        if (std::fabs(di) < eps)
        {
            // 시작점 a 가 슬래브 밖이면 교차 없음
            if (a < mn || a > mx)
                return false;
        }
        else
        {
            // t1, t2 계산: a + di * t = mn 또는 mx
            float ood = 1.0f / di;
            float t1 = (mn - a) * ood;
            float t2 = (mx - a) * ood;
            // t1 ≤ t2 보장
            if (t1 > t2) std::swap(t1, t2);

            // 기존 [tmin, tmax] 와 교차(intersect)
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);

            // 교차 구간이 없으면 실패
            if (tmin > tmax)
                return false;
        }
    }

    // 모든 축에서 교차 구간이 존재 → true
    return true;
}

bool FCollision::CheckOverlapSphereToCapsule(const USphereComponent& Sphere1, const UCapsuleComponent& Capsule1)
{
    // Calculate capsule P0, P1
    FVector P0 = Capsule1.GetWorldLocation() + Capsule1.GetWorldRotation().ToQuaternion().RotateVector(FVector::UpVector) * Capsule1.GetScaledCapsuleHalfHeight();
    FVector P1 = Capsule1.GetWorldLocation() - Capsule1.GetWorldRotation().ToQuaternion().RotateVector(FVector::UpVector) * Capsule1.GetScaledCapsuleHalfHeight();

    FVector Closet;
    
    FVector AB = P1 - P0;
    float ABLengthSq = AB.SquaredLength();
    if (ABLengthSq == 0.0f)
    {
        Closet = P0;
    }
    else
    {
        float t = AB.Dot(Sphere1.GetWorldLocation() - P0) / ABLengthSq;
        t = FMath::Clamp(t, 0.0f, 1.0f);
        Closet = P0 + AB * t;
    }

    return (Closet - Sphere1.GetWorldLocation()).SquaredLength() <= Capsule1.GetScaledCapsuleRadius() * Capsule1.GetScaledCapsuleRadius();
}
