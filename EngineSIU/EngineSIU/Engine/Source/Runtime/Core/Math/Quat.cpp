#include "Quat.h"

#include "Vector.h"
#include "Matrix.h"

const FQuat FQuat::Identity = {0.0f, 0.0f, 0.0f, 1.0f};

FQuat::FQuat(const FVector& Axis, float Angle)
{
    float HalfAngle = Angle * 0.5f;
    float SinHalfAngle = sinf(HalfAngle);
    float CosHalfAngle = cosf(HalfAngle);

    X = Axis.X * SinHalfAngle;
    Y = Axis.Y * SinHalfAngle;
    Z = Axis.Z * SinHalfAngle;
    W = CosHalfAngle;
}

FQuat::FQuat(const FMatrix& InMatrix)
{
    float S;
    // Check diagonal (trace)
    const float trace = InMatrix.M[0][0] + InMatrix.M[1][1] + InMatrix.M[2][2]; // 행렬의 Trace 값 (대각합)

    if (trace > 0.0f) 
    {
        float InvS = FMath::InvSqrt(trace + 1.f);
        this->W = 0.5f * (1.f / InvS);
        S = 0.5f * InvS;

        this->X = ((InMatrix.M[1][2] - InMatrix.M[2][1]) * S);
        this->Y = ((InMatrix.M[2][0] - InMatrix.M[0][2]) * S);
        this->Z = ((InMatrix.M[0][1] - InMatrix.M[1][0]) * S);
    } 
    else 
    {
        // diagonal is negative
        int32 i = 0;

        if (InMatrix.M[1][1] > InMatrix.M[0][0])
            i = 1;

        if (InMatrix.M[2][2] > InMatrix.M[i][i])
            i = 2;

        static constexpr int32 nxt[3] = { 1, 2, 0 };
        const int32 j = nxt[i];
        const int32 k = nxt[j];
 
        S = InMatrix.M[i][i] - InMatrix.M[j][j] - InMatrix.M[k][k] + 1.0f;

        float InvS = FMath::InvSqrt(S);

        float qt[4];
        qt[i] = 0.5f * (1.f / InvS);

        S = 0.5f * InvS;

        qt[3] = (InMatrix.M[j][k] - InMatrix.M[k][j]) * S;
        qt[j] = (InMatrix.M[i][j] + InMatrix.M[j][i]) * S;
        qt[k] = (InMatrix.M[i][k] + InMatrix.M[k][i]) * S;

        this->X = qt[0];
        this->Y = qt[1];
        this->Z = qt[2];
        this->W = qt[3];

    }
}

FQuat FQuat::FindBetween(const FVector& A, const FVector& B)
{
    const float NormAB = FMath::Sqrt(A.SizeSquared() * B.SizeSquared());
    float W = NormAB + FVector::DotProduct(A, B);
    FQuat Result;

    if (W >= 1e-6f * NormAB)
    {
        // Result = FVector::CrossProduct(A, B);
        Result = FQuat(
            A.Y * B.Z - A.Z * B.Y,
            A.Z * B.X - A.X * B.Z,
            A.X * B.Y - A.Y * B.X,
            W
        );
    }
    else
    {
        // A and B point in opposite directions
        W = 0.f;
        const float X = FMath::Abs(A.X);
        const float Y = FMath::Abs(A.Y);
        const float Z = FMath::Abs(A.Z);

        // Find orthogonal basis. 
        const FVector Basis = (X > Y && X > Z) ? FVector::YAxisVector : -FVector::XAxisVector;

        // Result = FVector::CrossProduct(A, Basis);
        Result = FQuat(
            A.Y * Basis.Z - A.Z * Basis.Y,
            A.Z * Basis.X - A.X * Basis.Z,
            A.X * Basis.Y - A.Y * Basis.X,
            W
        );
    }

    Result.Normalize();
    return Result;
}

FQuat FQuat::operator*(const FQuat& Other) const
{
    return FQuat(
            W * Other.W - X * Other.X - Y * Other.Y - Z * Other.Z,
            W * Other.X + X * Other.W + Y * Other.Z - Z * Other.Y,
            W * Other.Y - X * Other.Z + Y * Other.W + Z * Other.X,
            W * Other.Z + X * Other.Y - Y * Other.X + Z * Other.W
        );
}

FQuat FQuat::operator*(float value) const
{
    return FQuat(W * value, X * value, Y * value, Z * value);
}

FQuat FQuat::operator+(const FQuat& Other) const
{
    return FQuat(W + Other.W, X + Other.X, Y + Other.Y, Z + Other.Z);
}

FVector FQuat::RotateVector(const FVector& Vec) const
{
    // 벡터를 쿼터니언으로 변환
    FQuat vecQuat(0.0f, Vec.X, Vec.Y, Vec.Z);
    // 회전 적용 (q * vec * q^-1)
    FQuat conjugate = FQuat(W, -X, -Y, -Z); // 쿼터니언의 켤레
    FQuat result = *this * vecQuat * conjugate;

    return FVector(result.X, result.Y, result.Z); // 회전된 벡터 반환
}

bool FQuat::IsNormalized() const
{
    return fabs(W * W + X * X + Y * Y + Z * Z - 1.0f) < 1e-6f;
}

void FQuat::Normalize(float Tolerance)
{
    // TODO: 추후에 SIMD 사용
    const float SquareSum = X * X + Y * Y + Z * Z + W * W;

    if (SquareSum >= Tolerance)
    {
        const float Scale = FMath::InvSqrt(SquareSum);

        X *= Scale; 
        Y *= Scale; 
        Z *= Scale;
        W *= Scale;
    }
    else
    {
        *this = Identity;
    }
}

void FQuat::ToAxisAndAngle(FVector& Axis, float& Angle) const
{
    Angle = (float)GetAngle();  // 각도 추출
    Axis = GetRotationAxis();   // 축 벡터 계산
}

float FQuat::GetAngle() const
{
    // W 성분의 Acos 기반 각도 계산
    return 2.0f * FMath::Acos(W);
}

FVector FQuat::GetRotationAxis() const
{
    // TODO: 추후에 SIMD 사용

    // 벡터 성분의 제곱합 계산
    const float SquareSum = X * X + Y * Y + Z * Z;
    if (SquareSum < SMALL_NUMBER)
    {
        return FVector::XAxisVector;
    }

    // 벡터 정규화
    const float Scale = FMath::InvSqrt(SquareSum);
    return FVector{X * Scale, Y * Scale, Z * Scale};
}

FQuat FQuat::Slerp_NotNormalized(const FQuat& Quat1, const FQuat& Quat2, float Slerp)
{
    // Get cosine of angle between quats.
    float RawCosom =
        Quat1.X * Quat2.X +
        Quat1.Y * Quat2.Y +
        Quat1.Z * Quat2.Z +
        Quat1.W * Quat2.W;

    // Unaligned quats - compensate, results in taking shorter route.
    const float Sign = FMath::FloatSelect(RawCosom, 1.0f, -1.0f);
    RawCosom *= Sign;
		
    float Scale0 = 1.0f - Slerp;
    float Scale1 = Slerp * Sign;
		
    if (RawCosom < 0.9999)
    {
        const float Omega = FMath::Acos(RawCosom);
        const float InvSin = 1.0f / FMath::Sin(Omega);
        Scale0 = FMath::Sin(Scale0 * Omega) * InvSin;
        Scale1 = FMath::Sin(Scale1 * Omega) * InvSin;
    }
		
    return FQuat{
        Scale0 * Quat1.X + Scale1 * Quat2.X,
        Scale0 * Quat1.Y + Scale1 * Quat2.Y,
        Scale0 * Quat1.Z + Scale1 * Quat2.Z,
        Scale0 * Quat1.W + Scale1 * Quat2.W
    };
}

FQuat FQuat::FromAxisAngle(const FVector& Axis, float Angle)
{
    float halfAngle = Angle * 0.5f;
    float sinHalfAngle = sinf(halfAngle);
    return FQuat(cosf(halfAngle), Axis.X * sinHalfAngle, Axis.Y * sinHalfAngle, Axis.Z * sinHalfAngle);
}

FQuat FQuat::CreateRotation(float roll, float pitch, float yaw)
{
    // 각도를 라디안으로 변환
    float radRoll = roll * (PI / 180.0f);
    float radPitch = pitch * (PI / 180.0f);
    float radYaw = yaw * (PI / 180.0f);

    // 각 축에 대한 회전 쿼터니언 계산
    FQuat qRoll = FQuat(FVector(1.0f, 0.0f, 0.0f), radRoll); // X축 회전
    FQuat qPitch = FQuat(FVector(0.0f, 1.0f, 0.0f), radPitch); // Y축 회전
    FQuat qYaw = FQuat(FVector(0.0f, 0.0f, 1.0f), radYaw); // Z축 회전

    // 회전 순서대로 쿼터니언 결합 (Y -> X -> Z)
    return qRoll * qPitch * qYaw;
}

FMatrix FQuat::ToMatrix() const
{
    FMatrix RotationMatrix;
    RotationMatrix.M[0][0] = 1.0f - 2.0f * (Y * Y + Z * Z);
    RotationMatrix.M[0][1] = 2.0f * (X * Y - W * Z);
    RotationMatrix.M[0][2] = 2.0f * (X * Z + W * Y);
    RotationMatrix.M[0][3] = 0.0f;


    RotationMatrix.M[1][0] = 2.0f * (X * Y + W * Z);
    RotationMatrix.M[1][1] = 1.0f - 2.0f * (X * X + Z * Z);
    RotationMatrix.M[1][2] = 2.0f * (Y * Z - W * X);
    RotationMatrix.M[1][3] = 0.0f;

    RotationMatrix.M[2][0] = 2.0f * (X * Z - W * Y);
    RotationMatrix.M[2][1] = 2.0f * (Y * Z + W * X);
    RotationMatrix.M[2][2] = 1.0f - 2.0f * (X * X + Y * Y);
    RotationMatrix.M[2][3] = 0.0f;

    RotationMatrix.M[3][0] = RotationMatrix.M[3][1] = RotationMatrix.M[3][2] = 0.0f;
    RotationMatrix.M[3][3] = 1.0f;

    return RotationMatrix;
}
