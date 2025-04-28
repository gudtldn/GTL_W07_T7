#include "Vector.h"
#include "Misc/Parse.h"

const FVector2D FVector2D::ZeroVector = FVector2D(0, 0);
const FVector2D FVector2D::OneVector = FVector2D(1, 1);

const FVector FVector::ZeroVector = FVector(0, 0, 0);
const FVector FVector::OneVector = FVector(1, 1, 1);

const FVector FVector::UpVector = FVector(0, 0, 1);
const FVector FVector::DownVector = FVector(0, 0, -1);
const FVector FVector::ForwardVector = FVector(1, 0, 0);
const FVector FVector::BackwardVector = FVector(-1, 0, 0);
const FVector FVector::RightVector = FVector(0, 1, 0);
const FVector FVector::LeftVector = FVector(0, -1, 0);

const FVector FVector::XAxisVector = FVector(1, 0, 0);
const FVector FVector::YAxisVector = FVector(0, 1, 0);
const FVector FVector::ZAxisVector = FVector(0, 0, 1);


FString FVector2D::ToString() const
{
    return FString::Printf(TEXT("X=%3.3f Y=%3.3f"), X, Y);
}

bool FVector2D::InitFromString(const FString& InSourceString)
{
    X = Y = 0;

    // The initialization is only successful if the X and Y values can all be parsed from the string
    const bool bSuccessful = FParse::Value(*InSourceString, TEXT("X=") , X) && FParse::Value(*InSourceString, TEXT("Y="), Y) ;

    return bSuccessful;
}


FVector FVector::operator+(float Scalar) const
{
    return FVector{
        X + Scalar,
        Y + Scalar,
        Z + Scalar
    };
}

FVector FVector::operator-(float Scalar) const
{
    return FVector{
        X - Scalar,
        Y - Scalar,
        Z - Scalar
    };
}

float FVector::SizeSquared() const
{
    return SquaredLength();
}

bool FVector::IsNormalized() const
{
    constexpr float ThreshVectorNormalized = 0.01f;
    return (FMath::Abs(1.f - SizeSquared()) < ThreshVectorNormalized);
}

FString FVector::ToString() const
{
    // FString::Printf를 사용하여 포맷팅된 문자열 생성
    // TEXT() 매크로는 리터럴 문자열을 TCHAR 타입으로 만들어줍니다.
    return FString::Printf(TEXT("X=%3.3f Y=%3.3f Z=%3.3f"), X, Y, Z);

    // 필요에 따라 소수점 정밀도 지정 가능: 예) "X=%.2f Y=%.2f Z=%.2f"
    // return FString::Printf(TEXT("X=%.2f Y=%.2f Z=%.2f"), x, y, z);
}


bool FVector::InitFromString(const FString& InSourceString)
{
    // The initialization is only successful if the X, Y, and Z values can all be parsed from the string
    const bool bSuccessful = FParse::Value(*InSourceString, TEXT("X=") , X) && FParse::Value(*InSourceString, TEXT("Y="), Y) && FParse::Value(*InSourceString, TEXT("Z="), Z);

    return bSuccessful;
}
