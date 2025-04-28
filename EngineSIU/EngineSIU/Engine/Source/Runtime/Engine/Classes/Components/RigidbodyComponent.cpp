#include "RigidbodyComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "UObject/UObjectIterator.h"
#include "Engine/EditorEngine.h"

URigidbodyComponent::URigidbodyComponent()
    :Velocity(FVector::ZeroVector),
    AngularVelocity(FVector::ZeroVector),
    Mass(1.0f),
    ForceAccum(FVector::ZeroVector),
    TorqueAccum(FVector::ZeroVector),
    Restituation(0.7f),
    Friction(0.025f),
    Radius(1.0f),
    RollingFriction(0.1f)
{
    // TODO 크기에 대한 설정 가능하게끔 변경해야함
    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;

    InvInertiaBody = FMatrix::ComputeInvInertiaBox(Mass, width, height, depth);
    Gravity = FVector(0.0f, 0.0f, -7.5f);


    // x,y ∈ [-10,10] 범위를 둘러싼 네 벽
    // 1) 뒤쪽 벽 (y = -10)
    TestBoxes[0] = FBoundingBox(
        /*min=*/FVector(-10.0f, -10.0f, 0.0f),
        /*max=*/FVector(10.0f, -9.0f, 20.0f)
    );
    // 2) 앞쪽 벽 (y = +10)
    TestBoxes[1] = FBoundingBox(
        FVector(-10.0f, 9.0f, 0.0f),
        FVector(10.0f, 10.0f, 20.0f)
    );
    // 3) 왼쪽 벽 (x = -10)
    TestBoxes[2] = FBoundingBox(
        FVector(-10.0f, -10.0f, 0.0f),
        FVector(-9.0f, 10.0f, 20.0f)
    );
    // 4) 오른쪽 벽 (x = +10)
    TestBoxes[3] = FBoundingBox(
        FVector(9.0f, -10.0f, 0.0f),
        FVector(10.0f, 10.0f, 20.0f)
    );
}

URigidbodyComponent::~URigidbodyComponent()
{
}

UObject* URigidbodyComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->Velocity = Velocity;
    NewComponent->AngularVelocity = AngularVelocity;
    NewComponent->Mass = Mass;
    //NewComponent->Orientation = Orientation;
    NewComponent->ForceAccum = ForceAccum;
    NewComponent->TorqueAccum = TorqueAccum;

    return NewComponent;
}

void URigidbodyComponent::ApplyForceAtPoint(const FVector& force, const FVector& localPoint)
{
    ForceAccum += force;

    // 2) 씬 컴포넌트가 실제로 갖는 ‘월드 회전’을 구해서…
    USceneComponent* root = GetOwner()->GetRootComponent();
    FRotator worldRot = root->GetWorldRotation();              // 부모까지 합쳐진 회전
    FQuat    worldQuat = worldRot.ToQuaternion();              // 쿼터니언으로
    FMatrix  R = FMatrix::GetRotationMatrix(worldQuat);// 회전 행렬로

    // 3) 로컬 포인트(localPoint)가 월드 좌표에서 얼마만큼 떨어진 벡터인지 계산
    //    (즉, r = R * localPoint)
    FVector r;
    r.X = R.M[0][0] * localPoint.X + R.M[1][0] * localPoint.Y + R.M[2][0] * localPoint.Z;
    r.Y = R.M[0][1] * localPoint.X + R.M[1][1] * localPoint.Y + R.M[2][1] * localPoint.Z;
    r.Z = R.M[0][2] * localPoint.X + R.M[1][2] * localPoint.Y + R.M[2][2] * localPoint.Z;

    // 4) 무게중심이 컴포넌트 위치(root 위치)에 있다고 가정했다면,
    //    실제 토크 계산은 r 벡터 그대로 cross
    TorqueAccum += r ^ force;  // τ = r × F
}

void URigidbodyComponent::ApplyImpulseAtPoint(const FVector& impulse, const FVector& localPoint)
{
    Velocity += impulse / Mass;
    UpdateInertiaTensor();

    // 3) 동일하게 월드 회전부터 가져와 R 계산
    FQuat    worldQuat = GetOwner()->GetRootComponent()->GetWorldRotation()
        .ToQuaternion();
    FMatrix  R = FMatrix::GetRotationMatrix(worldQuat);

    // 4) r = R * localPoint
    FVector r;
    r.X = R.M[0][0] * localPoint.X + R.M[1][0] * localPoint.Y + R.M[2][0] * localPoint.Z;
    r.Y = R.M[0][1] * localPoint.X + R.M[1][1] * localPoint.Y + R.M[2][1] * localPoint.Z;
    r.Z = R.M[0][2] * localPoint.X + R.M[1][2] * localPoint.Y + R.M[2][2] * localPoint.Z;

    // 5) ΔL = r × J, Δω = I⁻¹_world * ΔL
    FVector dL = r ^ impulse;
    const FMatrix& Iw = InvInertiaWorld;
    FVector deltaOmega(
        Iw.M[0][0] * dL.X + Iw.M[0][1] * dL.Y + Iw.M[0][2] * dL.Z,
        Iw.M[1][0] * dL.X + Iw.M[1][1] * dL.Y + Iw.M[1][2] * dL.Z,
        Iw.M[2][0] * dL.X + Iw.M[2][1] * dL.Y + Iw.M[2][2] * dL.Z
    );
    AngularVelocity += deltaOmega;
}

void URigidbodyComponent::UpdateInertiaTensor()
{
    // 1. 씬 컴포넌트가 실제로 갖는 월드 회전(부모 포함)
    FRotator worldRot = GetOwner()->GetRootComponent()->GetWorldRotation();
    FQuat    worldQuat = worldRot.ToQuaternion();

    // 2. 월드 회전 → 행렬
    FMatrix R = FMatrix::GetRotationMatrix(worldQuat);

    // 3. I⁻¹_world = R · I⁻¹_body · R^T
    InvInertiaWorld = R * InvInertiaBody * FMatrix::Transpose(R);
}

void URigidbodyComponent::Integrate(float dt)
{
    // --- 0) 중력 가속도를 외력에 추가 ---
    //    F_gravity = m * g
    ForceAccum += Gravity * Mass;

    // --- 선형 ---
    FVector accel = ForceAccum / Mass;
    Velocity += accel * dt;
    FVector newLoc = GetOwner()->GetActorLocation() + Velocity * dt;
    GetOwner()->SetActorLocation(newLoc);

    // --- (B) 회전 적분 — 여기서 WorldRotation 사용 ---
    UpdateInertiaTensor();

    // (B1) 각가속도 α = I⁻¹_world · τ
    const FMatrix& Iw = InvInertiaWorld;
    FVector angAccel(
        Iw.M[0][0] * TorqueAccum.X + Iw.M[0][1] * TorqueAccum.Y + Iw.M[0][2] * TorqueAccum.Z,
        Iw.M[1][0] * TorqueAccum.X + Iw.M[1][1] * TorqueAccum.Y + Iw.M[1][2] * TorqueAccum.Z,
        Iw.M[2][0] * TorqueAccum.X + Iw.M[2][1] * TorqueAccum.Y + Iw.M[2][2] * TorqueAccum.Z
    );
    AngularVelocity += angAccel * dt;

    // (B2) 씬 상의 현재 월드 회전 얻기
    FRotator worldRot = GetOwner()->GetRootComponent()->GetWorldRotation();
    FQuat    worldQuat = worldRot.ToQuaternion();

    // (B3) 쿼터니언 미분: q̇ = ½ · q_world · Ω
    FQuat omegaQuat(0.0f,
        AngularVelocity.X,
        AngularVelocity.Y,
        AngularVelocity.Z);
    FQuat qDot = worldQuat * omegaQuat * 0.5f;

    // (B4) q_new = q_world + q̇·dt, 그리고 정규화
    FQuat newWorldQuat = worldQuat + qDot * dt;
    newWorldQuat.Normalize();

    // (B6) 씬 컴포넌트에도 새 회전 적용
    FRotator newRot(newWorldQuat);
    GetOwner()->GetRootComponent()->SetRelativeRotation(newRot);

    CheckAndResolveGroundCollision();

    // 굴림 운동에 대한 이해 부족으로 단순히 각속도 감쇠 로직 적용
    // 굴림 운동 구현하기엔 시간이 없어요 ㅠ
    FVector worldLoc = GetOwner()->GetActorLocation();
    if (worldLoc.Z <= 0.0f  && Velocity.Length() < 0.1f)
    {
        // 1) 각속도 댐핑
        float damping = 0.4f;
        AngularVelocity *= FMath::Clamp(1.0f - damping * dt, 0.0f, 1.0f);
    }

    // --- (C) 누적값 초기화 ---
    ForceAccum = FVector::ZeroVector;
    TorqueAccum = FVector::ZeroVector;
}

void URigidbodyComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    Integrate(DeltaTime);

    
    // StaticMeshComp의 AABB에 대해서 충돌 체크
    for (auto* Comp : TObjectRange<UStaticMeshComponent>())
    {
        // 자기 자신에 대한 검사인 경우 패스
        if (Comp->GetOwner() == this->GetOwner()) 
        {
            continue;
        }
        if (Comp->GetWorld() == GEngine->ActiveWorld)
        {
            ResolveSphereAABB(Comp->GetWorldAABB(), 1.0f, Restituation, Friction);
        }
    }
}

void URigidbodyComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Velocity"), FString::Printf(TEXT("%f"), Velocity));
    OutProperties.Add(TEXT("AngularVelocity"), FString::Printf(TEXT("%f"), AngularVelocity));
}

void URigidbodyComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("Velocity"));
    if (TempStr)
    {
        Velocity.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("AngularVelocity"));
    if (TempStr)
    {
        AngularVelocity.InitFromString(*TempStr);
    }
}

void URigidbodyComponent::ResetRigidbody()
{
    Velocity = FVector::ZeroVector;
    AngularVelocity = FVector::ZeroVector;
    Mass = 1;
    //Orientation = FQuat();
    ForceAccum = FVector::ZeroVector;
    TorqueAccum = FVector::ZeroVector;
}

void URigidbodyComponent::ResolveStaticCollision(const FVector& worldPoint, const FVector& normal, float restitution, float friction)
{
    // 1) 로컬 포인트로 변환 (root 위치 기준)
    FVector center = GetOwner()->GetActorLocation();
    FVector r_world = worldPoint - center;

    // 2) 접촉점에서의 상대 속도
    //    v_contact = v_linear + ω × r
    FVector v_contact = Velocity + (AngularVelocity ^ r_world);

    // 3) 노멀 성분
    float v_n = v_contact.Dot(normal);
    if (v_n >= 0.0f) {
        // 분리 중이거나 정지 상태라면 충돌 해결 불필요
        return;
    }

    // 4) 노멀 임펄스 크기 j
    //    j = -(1+e) * v_n / (1/m + normal · (I⁻¹ (r×normal) × r))
    UpdateInertiaTensor();  // InvInertiaWorld 갱신
    FVector r_cross_n = r_world ^ normal;
    // I⁻¹ * (r×n)
    FVector Iinv_r_cross_n(
        InvInertiaWorld.M[0][0] * r_cross_n.X + InvInertiaWorld.M[0][1] * r_cross_n.Y + InvInertiaWorld.M[0][2] * r_cross_n.Z,
        InvInertiaWorld.M[1][0] * r_cross_n.X + InvInertiaWorld.M[1][1] * r_cross_n.Y + InvInertiaWorld.M[1][2] * r_cross_n.Z,
        InvInertiaWorld.M[2][0] * r_cross_n.X + InvInertiaWorld.M[2][1] * r_cross_n.Y + InvInertiaWorld.M[2][2] * r_cross_n.Z
    );
    // normal·(Iinv_r_cross_n × r)
    FVector temp = Iinv_r_cross_n ^ r_world;
    float k_rot = normal.Dot(temp);
    float k_lin = 1.0f / Mass;
    float j = -(1.0f + restitution) * v_n / (k_lin + k_rot);

    // 5) 노멀 방향 임펄스 적용
    FVector impulseN = normal * j;
    ApplyImpulseAtPoint(impulseN, r_world);

    // 6) kinetic friction 임펄스만
    FVector v_tangent = v_contact - normal * v_contact.Dot(normal);
    float mag_vt = v_tangent.Length();
    if (mag_vt > SMALL_NUMBER)
    {
        FVector tangent = v_tangent / mag_vt;  // 단위 탄젠트
        // kinetic friction: 항상 μ * j 크기만큼 감속
        float jt = friction * j;               // + 방향? j<0 이니 jt<0
        // 실제 임펄스 방향은 -sign(v_tangent) 이므로
        FVector impulseT = tangent * (-jt);
        ApplyImpulseAtPoint(impulseT, r_world);
    }
}

void URigidbodyComponent::CheckAndResolveGroundCollision()
{
    // 1. 현재 월드 위치
    FVector worldLoc = GetOwner()->GetActorLocation();

    // 2. 땅(Plane z=0) 아래로 내려갔으면 충돌 처리
    if (worldLoc.Z < 0.0f)
    {
        // 충돌점: 수직 투영
        FVector contactPoint(worldLoc.X, worldLoc.Y, 0.0f);

        // 땅 노멀은 +Z
        FVector groundNormal(0.0f, 0.0f, 1.0f);

        // ResolveStaticCollision 내부에서 반발계수와 마찰계수를 사용
        ResolveStaticCollision(contactPoint, groundNormal, Restituation, Friction);

        // 충돌 후 위치 보정: 바로 땅 위로 올려두면 파이프 관통 방지
        worldLoc.Z = 0.0f;
        GetOwner()->SetActorLocation(worldLoc);
    }
}


void URigidbodyComponent::ResolveSphereAABB(const FBoundingBox& box, float radius, float restitution, float friction)
{
    // 1) 구 중심
    FVector center = GetOwner()->GetActorLocation();

    // 2) AABB 위의 가장 가까운 점 찾기 (클램프)
    FVector closest;
    closest.X = FMath::Clamp(center.X, box.min.X, box.max.X);
    closest.Y = FMath::Clamp(center.Y, box.min.Y, box.max.Y);
    closest.Z = FMath::Clamp(center.Z, box.min.Z, box.max.Z);

    // 3) 중심↔closest 벡터, 거리 제곱
    FVector delta = center - closest;
    float dist2 = delta.SizeSquared();

    // 4) 충돌 여부: dist² ≤ r²
    if (dist2 > radius * radius)
        return;

    // 5) 실제 거리와 penetration 깊이
    float dist = FMath::Sqrt(dist2);
    float penetration = radius - dist;

    // 6) 충돌 normal (거리 0일 땐 임의축)
    FVector normal = (dist > KINDA_SMALL_NUMBER)
        ? delta / dist
        : FVector(1.0f, 0.0f, 0.0f);

    // 7) 위치 보정: penetration 만큼 평면 밖으로 이동
    center += normal * penetration;
    GetOwner()->SetActorLocation(center);

    // 8) 접촉점 (AABB 위의 closest)
    FVector contactPoint = closest;

    // 9) r_world: COM→접촉점
    FVector r_world = contactPoint - center;

    // 10) 접촉점 상대 속도: v + ω×r
    FVector v_contact = Velocity + (AngularVelocity ^ r_world);

    // 11) 노멀 성분
    float v_n = v_contact.Dot(normal);
    if (v_n >= 0.0f)
        return;  // 분리 중이므로 처리 안 함

    // 12) 충돌 임펄스 j 계산
    UpdateInertiaTensor();  // invInertiaWorld 갱신
    FVector r_cross_n = r_world ^ normal;

    // I⁻¹ · (r×n)
    FVector Iinv_rn(
        InvInertiaWorld.M[0][0] * r_cross_n.X + InvInertiaWorld.M[0][1] * r_cross_n.Y + InvInertiaWorld.M[0][2] * r_cross_n.Z,
        InvInertiaWorld.M[1][0] * r_cross_n.X + InvInertiaWorld.M[1][1] * r_cross_n.Y + InvInertiaWorld.M[1][2] * r_cross_n.Z,
        InvInertiaWorld.M[2][0] * r_cross_n.X + InvInertiaWorld.M[2][1] * r_cross_n.Y + InvInertiaWorld.M[2][2] * r_cross_n.Z
    );

    float k_rot = normal.Dot(Iinv_rn ^ r_world); // 회전 저항
    float k_lin = 1.0f / Mass;                            // 선형 관성
    float j = -(1.0f + restitution) * v_n / (k_lin + k_rot);

    // 13) 노멀 임펄스 적용
    FVector impulseN = normal * j;
    ApplyImpulseAtPoint(impulseN, r_world);

    // 14) 마찰 임펄스 (kinetic)
    FVector v_tangent = v_contact - normal * v_n;
    float vt = v_tangent.Length();
    if (vt > KINDA_SMALL_NUMBER)
    {
        FVector tangent = v_tangent / vt;
        float jt = friction * FMath::Abs(j);
        FVector impulseT = -tangent * jt;
        ApplyImpulseAtPoint(impulseT, r_world);
    }
}
