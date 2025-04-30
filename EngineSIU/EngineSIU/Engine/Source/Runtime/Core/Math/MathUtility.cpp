#include "MathUtility.h"
#include "Color.h"
#include "Quat.h"
#include "Vector.h"
#include "Rotator.h"


FVector FMath::VInterpNormalRotationTo(const FVector& Current, const FVector& Target, float DeltaTime, float RotationSpeedDegrees)
{
    // Find delta rotation between both normals.
    FQuat DeltaQuat = FQuat::FindBetween(Current, Target);

    // Decompose into an axis and angle for rotation
    FVector DeltaAxis(0.f);
    FQuat::FReal DeltaAngle = 0.f;
    DeltaQuat.ToAxisAndAngle(DeltaAxis, DeltaAngle);

    // Find rotation step for this frame
    const float RotationStepRadians = RotationSpeedDegrees * (PI / 180) * DeltaTime;

    if (FMath::Abs(DeltaAngle) > RotationStepRadians)
    {
        DeltaAngle = FMath::Clamp<FQuat::FReal>(DeltaAngle, -RotationStepRadians, RotationStepRadians);
        DeltaQuat = FQuat(DeltaAxis, DeltaAngle);
        return DeltaQuat.RotateVector(Current);
    }
    return Target;
}

FVector FMath::VInterpConstantTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed)
{
    const FVector Delta = Target - Current;
    const FVector::FReal DeltaM = Delta.Size();
    const FVector::FReal MaxStep = InterpSpeed * DeltaTime;

    if (DeltaM > MaxStep)
    {
        if (MaxStep > 0.f)
        {
            const FVector DeltaN = Delta / DeltaM;
            return Current + DeltaN * MaxStep;
        }
        else
        {
            return Current;
        }
    }

    return Target;
}

FVector FMath::VInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed)
{
    // If no interp speed, jump to target value
    if (InterpSpeed <= 0.f)
    {
        return Target;
    }

    // Distance to reach
    const FVector Dist = Target - Current;

    // If distance is too small, just set the desired location
    if (Dist.SizeSquared() < KINDA_SMALL_NUMBER)
    {
        return Target;
    }

    // Delta Move, Clamp so we do not over shoot.
    const FVector DeltaMove = Dist * FMath::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);

    return Current + DeltaMove;
}

FVector2D FMath::Vector2DInterpConstantTo(const FVector2D& Current, const FVector2D& Target, float DeltaTime, float InterpSpeed)
{
    const FVector2D Delta = Target - Current;
    const FVector2D::FReal DeltaM = Delta.Size();
    const float MaxStep = InterpSpeed * DeltaTime;

    if (DeltaM > MaxStep)
    {
        if (MaxStep > 0.f)
        {
            const FVector2D DeltaN = Delta / DeltaM;
            return Current + DeltaN * MaxStep;
        }
        else
        {
            return Current;
        }
    }

    return Target;
}

FVector2D FMath::Vector2DInterpTo(const FVector2D& Current, const FVector2D& Target, float DeltaTime, float InterpSpeed)
{
    if (InterpSpeed <= 0.f)
    {
        return Target;
    }

    const FVector2D Dist = Target - Current;
    if (Dist.SizeSquared() < KINDA_SMALL_NUMBER)
    {
        return Target;
    }

    const FVector2D DeltaMove = Dist * FMath::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);
    return Current + DeltaMove;
}

FRotator FMath::RInterpConstantTo(const FRotator& Current, const FRotator& Target, float DeltaTime, float InterpSpeed)
{
    // if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
    if (DeltaTime == 0.f || Current == Target)
    {
        return Current;
    }

    // If no interp speed, jump to target value
    if (InterpSpeed <= 0.f)
    {
        return Target;
    }

    const float DeltaInterpSpeed = InterpSpeed * DeltaTime;

    const FRotator DeltaMove = (Target - Current).GetNormalized();
    FRotator Result = Current;
    Result.Pitch += FMath::Clamp(DeltaMove.Pitch, -DeltaInterpSpeed, DeltaInterpSpeed);
    Result.Yaw += FMath::Clamp(DeltaMove.Yaw, -DeltaInterpSpeed, DeltaInterpSpeed);
    Result.Roll += FMath::Clamp(DeltaMove.Roll, -DeltaInterpSpeed, DeltaInterpSpeed);
    return Result.GetNormalized();
}

FRotator FMath::RInterpTo(const FRotator& Current, const FRotator& Target, float DeltaTime, float InterpSpeed)
{
    // if DeltaTime is 0, do not perform any interpolation (Location was already calculated for that frame)
    if (DeltaTime == 0.f || Current == Target)
    {
        return Current;
    }

    // If no interp speed, jump to target value
    if (InterpSpeed <= 0.f)
    {
        return Target;
    }

    const float DeltaInterpSpeed = InterpSpeed * DeltaTime;

    const FRotator Delta = (Target - Current).GetNormalized();

    // If steps are too small, just return Target and assume we have reached our destination.
    if (Delta.IsNearlyZero())
    {
        return Target;
    }

    // Delta Move, Clamp so we do not over shoot.
    const FRotator DeltaMove = Delta * FMath::Clamp<float>(DeltaInterpSpeed, 0.f, 1.f);
    return (Current + DeltaMove).GetNormalized();
}

/** Interpolate Linear Color from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
FLinearColor FMath::CInterpTo(const FLinearColor& Current, const FLinearColor& Target, float DeltaTime, float InterpSpeed)
{
    // If no interp speed, jump to target value
    if (InterpSpeed <= 0.f)
    {
        return Target;
    }

    // Difference between colors
    const float Dist = FLinearColor::Dist(Target, Current);

    // If distance is too small, just set the desired color
    if (Dist < KINDA_SMALL_NUMBER)
    {
        return Target;
    }

    // Delta change, Clamp so we do not over shoot.
    const FLinearColor DeltaMove = (Target - Current) * FMath::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);

    return Current + DeltaMove;
}

FQuat FMath::QInterpConstantTo(const FQuat& Current, const FQuat& Target, float DeltaTime, float InterpSpeed)
{
	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	// If the values are nearly equal, just return Target and assume we have reached our destination.
	if (Current.Equals(Target))
	{
		return Target;
	}

	float DeltaInterpSpeed = FMath::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);
	float AngularDistance = FMath::Max<float>(SMALL_NUMBER, (float)(Target.AngularDistance(Current)));
	float Alpha = FMath::Clamp<float>(DeltaInterpSpeed / AngularDistance, 0.f, 1.f);

	return FQuat::Slerp(Current, Target, Alpha);
}

FQuat FMath::QInterpTo(const FQuat& Current, const FQuat& Target, float DeltaTime, float InterpSpeed)
{
	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
	{
		return Target;
	}

	// If the values are nearly equal, just return Target and assume we have reached our destination.
	if (Current.Equals(Target))
	{
		return Target;
	}

	return FQuat::Slerp(Current, Target, FMath::Clamp<float>(InterpSpeed * DeltaTime, 0.f, 1.f));
}
