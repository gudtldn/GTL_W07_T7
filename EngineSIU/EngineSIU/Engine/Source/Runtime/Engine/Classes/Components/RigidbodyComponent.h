#pragma once
#include "Components/SceneComponent.h"
#include "Math/JungleMath.h"


class URigidbodyComponent : public USceneComponent 
{
    DECLARE_CLASS(URigidbodyComponent, USceneComponent)
public:
    URigidbodyComponent();
    virtual ~URigidbodyComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    // 점(point)에 힘(force) 가하기
    void ApplyForceAtPoint(const FVector& force, const FVector& point);

    // 점(point)에 임펄스(impulse) 가하기
    void ApplyImpulseAtPoint(const FVector& impulse, const FVector& point);

    // orientation에 따라 invInertiaWorld = R·invInertiaBody·R^T 계산
    void UpdateInertiaTensor();

    // dt 시간만큼 물리 통합 (반드시 매 프레임 호출)
    void Integrate(float dt);

    virtual void TickComponent(float DeltaTime) override;

    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;

    FVector GetVelocity() { return Velocity; }
    void SetVelocity(const FVector& InVelocity) { Velocity = InVelocity; }

    FVector GetAngularVelocity() { return AngularVelocity; }
    void SetAngularVelocity(const FVector& InAngularVelocity) { AngularVelocity = InAngularVelocity; }

    FVector GetAccumulatedForce() { return ForceAccum; }
    FVector GetAccumulatedTorque() { return TorqueAccum; }

    void ResetRigidbody();

    void ResolveStaticCollision(const FVector& worldPoint, const FVector& normal, float restitution, float friction);

    void CheckAndResolveGroundCollision();

    void ResolveSphereAABB(const FBoundingBox& box, float radius, float restitution, float friction);

private:
    //FQuat Orientation;  // 회전 쿼터니언
    FVector Velocity;   // 선형 속도
    FVector AngularVelocity;    // 각속도

    // 물리 속성
    float Mass;     // 질량
    FMatrix InvInertiaBody;    // 바디 공간 역관성 텐서 (4x4, 상위 3x3 사용)
    FMatrix InvInertiaWorld;   // 월드 공간 역관성 텐서

    // 누적 용량
    FVector ForceAccum;        // 누적 외력
    FVector TorqueAccum;       // 누적 토크

    float Restituation;
    float Friction;

    FVector Gravity;

    float Radius;
    float RollingFriction;

    // 테스트용 AABB 4개
    FBoundingBox TestBoxes[4];
};
