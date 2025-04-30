#pragma once
#include "Vector.h"
#include "Serialization/Archive.h"


// 4D Vector
struct FVector4
{
public:
    float X, Y, Z, W;

    using FReal = float;

public:
    FVector4() : X(0), Y(0), Z(0), W(0) {}
    FVector4(float InX, float InY, float InZ, float InW)
        : X(InX), Y(InY), Z(InZ), W(InW)
    {}
    FVector4(const FVector& InVector, float InW = 0)
        : X(InVector.X), Y(InVector.Y), Z(InVector.Z)
        , W(InW)
    {}
    FVector4(const FString& SourceString)
        : X(0), Y(0), Z(0), W(0)
    {
        InitFromString(SourceString);
    }

    FVector4 operator+(const FVector4& Other) const;
    FVector4 operator-(const FVector4& Other) const;

    FVector4 operator/(float Scalar) const;

    FString ToString() const;
    bool InitFromString(const FString& InSourceString);
};

inline FVector4 FVector4::operator-(const FVector4& Other) const
{
    return {
        X - Other.X,
        Y - Other.Y,
        Z - Other.Z,
        W - Other.W
    };
}

inline FVector4 FVector4::operator+(const FVector4& Other) const
{
    return {
        X + Other.X,
        Y + Other.Y,
        Z + Other.Z,
        W + Other.W
    };
}

inline FVector4 FVector4::operator/(float Scalar) const
{
    return {
        X / Scalar,
        Y / Scalar,
        Z / Scalar,
        W / Scalar
    };
}



inline FArchive& operator<<(FArchive& Ar, FVector4& V)
{
    return Ar << V.X << V.Y << V.Z << V.W;
}
