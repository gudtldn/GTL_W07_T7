#include "LuaUserTypes.h"

#include "Engine/World/World.h"
#include "Developer/Lua/LuaActor.h"
#include "Developer/Lua/LuaUtils/LuaBindMacros.h"
#include "Math/Color.h"
#include "Math/Matrix.h"
#include "Math/Quat.h"
#include "Developer/Lua/LuaPlayer.h"
#include "Developer/Lua/LuaCoachActor.h"

void LuaTypes::FBindLua<FColor>::Bind(sol::table& Table)
{
    Table.Lua_NewUserType(
        FColor,

        // 생성자
        LUA_BIND_CONSTRUCTORS(
            FColor(),
            FColor(uint8, uint8, uint8, uint8),
            FColor(uint32)
        ),

        // 멤버 변수
        LUA_BIND_MEMBER(&FColor::R),
        LUA_BIND_MEMBER(&FColor::G),
        LUA_BIND_MEMBER(&FColor::B),
        LUA_BIND_MEMBER(&FColor::A),

        // 멤버 함수
        LUA_BIND_OVERLOAD2(
            &FColor::DWColor,
            uint32_t& (),
            const uint32_t& () const
        ),

        // 정적 상수 (읽기 전용 속성으로 바인딩)
        LUA_BIND_STATIC(FColor::White),
        LUA_BIND_STATIC(FColor::Black),
        LUA_BIND_STATIC(FColor::Transparent),
        LUA_BIND_STATIC(FColor::Red),
        LUA_BIND_STATIC(FColor::Green),
        LUA_BIND_STATIC(FColor::Blue),
        LUA_BIND_STATIC(FColor::Yellow),
        LUA_BIND_STATIC(FColor::Cyan),
        LUA_BIND_STATIC(FColor::Magenta),
        LUA_BIND_STATIC(FColor::Orange),
        LUA_BIND_STATIC(FColor::Purple),
        LUA_BIND_STATIC(FColor::Turquoise),
        LUA_BIND_STATIC(FColor::Silver),
        LUA_BIND_STATIC(FColor::Emerald),

        // 연산자 오버로딩
        sol::meta_function::equal_to, &FColor::operator==,

        // 연산자 메서드
        "AddAssign", [](FColor& A, const FColor& B) { return A += B; }
    );
}

void LuaTypes::FBindLua<FLinearColor>::Bind(sol::table& Table)
{
    Table.Lua_NewUserType(
        FLinearColor,

        // Constructors
        LUA_BIND_CONSTRUCTORS(
            FLinearColor(),
            FLinearColor(float, float, float, float),
            FLinearColor(const FColor&)
        ),

        // Member variables
        LUA_BIND_MEMBER(&FLinearColor::R),
        LUA_BIND_MEMBER(&FLinearColor::G),
        LUA_BIND_MEMBER(&FLinearColor::B),
        LUA_BIND_MEMBER(&FLinearColor::A),

        // Member Method
        LUA_BIND_FUNC(&FLinearColor::GetClamp),
        LUA_BIND_FUNC(&FLinearColor::Equals),
        LUA_BIND_FUNC(&FLinearColor::Lerp),
        LUA_BIND_FUNC(&FLinearColor::GetMax),
        LUA_BIND_FUNC(&FLinearColor::GetMin),
        LUA_BIND_FUNC(&FLinearColor::ToString),

        // Static Method
        LUA_BIND_FUNC(&FLinearColor::FromColor),

        // Static constants
        LUA_BIND_STATIC(FLinearColor::White),
        LUA_BIND_STATIC(FLinearColor::Gray),
        LUA_BIND_STATIC(FLinearColor::Black),
        LUA_BIND_STATIC(FLinearColor::Transparent),
        LUA_BIND_STATIC(FLinearColor::Red),
        LUA_BIND_STATIC(FLinearColor::Green),
        LUA_BIND_STATIC(FLinearColor::Blue),
        LUA_BIND_STATIC(FLinearColor::Yellow),

        // Operators
        sol::meta_function::addition, &FLinearColor::operator+,
        sol::meta_function::subtraction, &FLinearColor::operator-,
        sol::meta_function::multiplication, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
            &FLinearColor::operator*,
            FLinearColor(const FLinearColor&) const,
            FLinearColor(float) const
        ),
        sol::meta_function::division, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
            &FLinearColor::operator/,
            FLinearColor(const FLinearColor&) const,
            FLinearColor(float) const
        ),
        sol::meta_function::equal_to, &FLinearColor::operator==,

        // 연산자 메서드
        "AddAssign", &FLinearColor::operator+=,
        "SubAssign", &FLinearColor::operator-=,
        "MulAssign", sol::overload(
            [](FLinearColor& Self, const FLinearColor& Other) -> FLinearColor& { return Self *= Other; },
            [](FLinearColor& Self, float Scalar) -> FLinearColor& { return Self *= Scalar; }
        ),
        "DivAssign", sol::overload(
            [](FLinearColor& Self, const FLinearColor& Other) -> FLinearColor& { return Self /= Other; },
            [](FLinearColor& Self, float Scalar) -> FLinearColor& { return Self /= Scalar; }
        )
    );
}

void LuaTypes::FBindLua<FVector>::Bind(sol::table& Table)
{
    Table.Lua_NewUserType(
        FVector,

        // Constructors
        LUA_BIND_CONSTRUCTORS(
            FVector(),
            FVector(float, float, float),
            FVector(float)
        ),

        // Member variables
        LUA_BIND_MEMBER(&FVector::X),
        LUA_BIND_MEMBER(&FVector::Y),
        LUA_BIND_MEMBER(&FVector::Z),

        // Utility functions
        LUA_BIND_FUNC(&FVector::Dot),
        LUA_BIND_FUNC(&FVector::Cross),
        LUA_BIND_FUNC(&FVector::Equals),
        LUA_BIND_FUNC(&FVector::AllComponentsEqual),
        LUA_BIND_FUNC(&FVector::Length),
        LUA_BIND_FUNC(&FVector::SquaredLength),
        LUA_BIND_FUNC(&FVector::SizeSquared),
        LUA_BIND_FUNC(&FVector::Normalize),
        LUA_BIND_FUNC(&FVector::GetUnsafeNormal),
        LUA_BIND_FUNC(&FVector::GetSafeNormal),
        LUA_BIND_FUNC(&FVector::ComponentMin),
        LUA_BIND_FUNC(&FVector::ComponentMax),
        LUA_BIND_FUNC(&FVector::IsNearlyZero),
        LUA_BIND_FUNC(&FVector::IsZero),
        LUA_BIND_FUNC(&FVector::IsNormalized),
        LUA_BIND_FUNC(&FVector::ToString),

        // Static Method
        LUA_BIND_FUNC(&FVector::Zero),
        LUA_BIND_FUNC(&FVector::One),

        LUA_BIND_FUNC(&FVector::UnitX),
        LUA_BIND_FUNC(&FVector::UnitY),
        LUA_BIND_FUNC(&FVector::UnitZ),

        LUA_BIND_FUNC(&FVector::Distance),
        LUA_BIND_FUNC(&FVector::DotProduct),
        LUA_BIND_FUNC(&FVector::CrossProduct),

        // Static properties
        LUA_BIND_STATIC(FVector::ZeroVector),
        LUA_BIND_STATIC(FVector::OneVector),
        LUA_BIND_STATIC(FVector::UpVector),
        LUA_BIND_STATIC(FVector::DownVector),
        LUA_BIND_STATIC(FVector::ForwardVector),
        LUA_BIND_STATIC(FVector::BackwardVector),
        LUA_BIND_STATIC(FVector::RightVector),
        LUA_BIND_STATIC(FVector::LeftVector),
        LUA_BIND_STATIC(FVector::XAxisVector),
        LUA_BIND_STATIC(FVector::YAxisVector),
        LUA_BIND_STATIC(FVector::ZAxisVector),

        // Operators
        sol::meta_function::addition, LUA_BIND_OVERLOAD_WITHOUT_NAME2(\
            &FVector::operator+,
            FVector(const FVector&) const,
            FVector(float) const
        ),
        sol::meta_function::subtraction, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
            &FVector::operator-,
            FVector(const FVector&) const,
            FVector(float) const
        ),
        sol::meta_function::multiplication, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
            &FVector::operator*,
            FVector(const FVector&) const,
            FVector(float) const
        ),
        sol::meta_function::division, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
            &FVector::operator/,
            FVector(const FVector&) const,
            FVector(float) const
        ),
        sol::meta_function::equal_to, &FVector::operator==,

        // 연산자 메서드
        "AddAssign", [](FVector& Self, const FVector& Other) { return Self += Other; },
        "SubAssign", [](FVector& Self, const FVector& Other) { return Self -= Other; },
        "MulAssign", [](FVector& Self, float Scalar) { return Self *= Scalar; },
        "DivAssign", [](FVector& Self, float Scalar) { return Self /= Scalar; }
    );
}

void LuaTypes::FBindLua<FVector2D>::Bind(sol::table& Table)
{
    Table.Lua_NewUserType(
        FVector2D,

        // Constructors
        LUA_BIND_CONSTRUCTORS(
            FVector2D(),
            FVector2D(float, float),
            FVector2D(float)
        ),

        // Member variables
        LUA_BIND_MEMBER(&FVector2D::X),
        LUA_BIND_MEMBER(&FVector2D::Y),

        // Utility functions
        // LUA_BIND_FUNC(&FVector2D::Length),
        // LUA_BIND_FUNC(&FVector2D::LengthSquared),
        // LUA_BIND_FUNC(&FVector2D::Normalize),
        // LUA_BIND_FUNC(&FVector2D::IsNormalized),
        // LUA_BIND_FUNC(&FVector2D::DotProduct),

        LUA_BIND_FUNC(&FVector2D::ToString),

        // Static properties
        LUA_BIND_STATIC(FVector2D::ZeroVector),
        LUA_BIND_STATIC(FVector2D::OneVector),

        // Operators
        sol::meta_function::addition, &FVector2D::operator+,
        sol::meta_function::subtraction, &FVector2D::operator-,
        sol::meta_function::multiplication, &FVector2D::operator*,
        sol::meta_function::division, &FVector2D::operator/,
        sol::meta_function::equal_to, &FVector2D::operator==,

        // 연산자 메서드
        "AddAssign", &FVector2D::operator+=
    );
}

void LuaTypes::FBindLua<FVector4>::Bind(sol::table& Table)
{
    Table.Lua_NewUserType(
        FVector4,

        // Constructors
        LUA_BIND_CONSTRUCTORS(
            FVector4(),
            FVector4(float, float, float, float),
            FVector4(const FVector&, float)
        ),

        // Member variables
        LUA_BIND_MEMBER(&FVector4::X),
        LUA_BIND_MEMBER(&FVector4::Y),
        LUA_BIND_MEMBER(&FVector4::Z),
        LUA_BIND_MEMBER(&FVector4::W),

        // Member Method
        LUA_BIND_FUNC(&FVector4::ToString),

        // Operators
        sol::meta_function::addition, &FVector4::operator+,
        sol::meta_function::subtraction, &FVector4::operator-,
        sol::meta_function::division, &FVector4::operator/ // Scalar
    );
}

void LuaTypes::FBindLua<FRotator>::Bind(sol::table& Table)
{
    Table.Lua_NewUserType(
        FRotator,

        // Constructors
        LUA_BIND_CONSTRUCTORS(
            FRotator(),
            FRotator(float, float, float)
        ),

        // Member variables
        LUA_BIND_MEMBER(&FRotator::Pitch),
        LUA_BIND_MEMBER(&FRotator::Yaw),
        LUA_BIND_MEMBER(&FRotator::Roll),

        // Utility functions
        LUA_BIND_FUNC(&FRotator::IsNearlyZero),
        LUA_BIND_FUNC(&FRotator::IsZero),
        LUA_BIND_FUNC(&FRotator::Equals),
        LUA_BIND_FUNC(&FRotator::Add),
        LUA_BIND_FUNC(&FRotator::FromQuaternion),
        LUA_BIND_FUNC(&FRotator::ToQuaternion),
        LUA_BIND_FUNC(&FRotator::ToVector),
        LUA_BIND_FUNC(&FRotator::ToMatrix),
        LUA_BIND_FUNC(&FRotator::Clamp),
        LUA_BIND_FUNC(&FRotator::GetNormalized),
        LUA_BIND_FUNC(&FRotator::Normalize),
        LUA_BIND_FUNC(&FRotator::ToString),
        LUA_BIND_FUNC(&FRotator::InitFromString),

        // Static properties
        // LUA_BIND_STATIC(FRotator::ZeroRotator),

        // Operators
        sol::meta_function::addition, &FRotator::operator+,
        sol::meta_function::subtraction, [](const FRotator& A, const FRotator& B) { return A - B; },
        sol::meta_function::multiplication, &FRotator::operator*,
        sol::meta_function::division, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
            &FRotator::operator/,
            FRotator(const FRotator&) const,
            FRotator(float) const
        ),
        sol::meta_function::unary_minus, [](const FRotator& A) { return -A; },
        sol::meta_function::equal_to, &FRotator::operator==,

        // 연산자 메서드
        "AddAssign", &FRotator::operator+=,
        "SubAssign", &FRotator::operator-=,
        "MulAssign", &FRotator::operator*=,
        "DivAssign", &FRotator::operator/= // Scalar
    );
}

void LuaTypes::FBindLua<FQuat>::Bind(sol::table& Table)
{
    Table.Lua_NewUserType(
        FQuat,

        // Constructors
        LUA_BIND_CONSTRUCTORS(
            FQuat(),
            FQuat(const FVector&, float),
            FQuat(float, float, float, float),
            FQuat(const FMatrix&)
        ),

        // Member variables
        LUA_BIND_MEMBER(&FQuat::W),
        LUA_BIND_MEMBER(&FQuat::X),
        LUA_BIND_MEMBER(&FQuat::Y),
        LUA_BIND_MEMBER(&FQuat::Z),

        // Utility functions
        LUA_BIND_FUNC(&FQuat::RotateVector),
        LUA_BIND_FUNC(&FQuat::IsNormalized),
        LUA_BIND_FUNC(&FQuat::Normalize),
        LUA_BIND_FUNC(&FQuat::ToMatrix),

        // Static functions
        LUA_BIND_FUNC(&FQuat::FromAxisAngle),
        LUA_BIND_FUNC(&FQuat::CreateRotation),

        // LUA_BIND_STATIC(FQuat::Identity)

        // Operators
        sol::meta_function::multiplication, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
            &FQuat::operator*,
            FQuat(const FQuat&) const,
            FQuat(float) const
        )
    );
}

void LuaTypes::FBindLua<FMatrix>::Bind(sol::table& Table)
{
    Table.Lua_NewUserType(
        FMatrix,

        // Constructors
        LUA_BIND_CONSTRUCTORS(
            FMatrix()
        ),

        // Methods
        LUA_BIND_FUNC(&FMatrix::TransformFVector4),
        LUA_BIND_FUNC(&FMatrix::TransformPosition),
        LUA_BIND_FUNC(&FMatrix::ToQuat),

        // Static functions
        LUA_BIND_FUNC(&FMatrix::Transpose),
        LUA_BIND_FUNC(&FMatrix::Inverse),
        LUA_BIND_FUNC(&FMatrix::CreateRotationMatrix),
        LUA_BIND_FUNC(&FMatrix::CreateScaleMatrix),
        LUA_BIND_OVERLOAD2(
            &FMatrix::TransformVector,
            FVector(const FVector&, const FMatrix&),
            FVector4(const FVector4&, const FMatrix&)
        ),
        LUA_BIND_FUNC(&FMatrix::CreateTranslationMatrix),
        LUA_BIND_FUNC(&FMatrix::GetScaleMatrix),
        LUA_BIND_FUNC(&FMatrix::GetTranslationMatrix),
        LUA_BIND_OVERLOAD2(
            &FMatrix::GetRotationMatrix,
            FMatrix(const FRotator&),
            FMatrix(const FQuat&)
        ),

        // Static variable
        LUA_BIND_STATIC(FMatrix::Identity),

        // Operators
        sol::meta_function::addition, &FMatrix::operator+,
        sol::meta_function::subtraction, &FMatrix::operator-,
        sol::meta_function::multiplication, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
            &FMatrix::operator*,
            FMatrix(const FMatrix&) const,
            FMatrix(float) const
        ),
        sol::meta_function::division, &FMatrix::operator/, // Scalar
        sol::meta_function::index, LUA_BIND_OVERLOAD_WITHOUT_NAME2(
            &FMatrix::operator[],
            float*(int32),
            const float*(int32) const,
        )
    );
}

void LuaTypes::FBindLua<ALuaActor>::Bind(sol::table& Table)
{
    Table.Lua_NewUserType(
        ALuaActor,

        // UObject 메서드
        LUA_BIND_FUNC(&ALuaActor::Duplicate),
        LUA_BIND_FUNC(&ALuaActor::GetFName),
        LUA_BIND_FUNC(&ALuaActor::GetName),
        LUA_BIND_FUNC(&ALuaActor::GetNameStr),
        "GetClassName", [](const ALuaActor& Self) { return Self.GetClass()->GetNameStr(); },
        LUA_BIND_FUNC(&ALuaActor::GetOuter),
        LUA_BIND_FUNC(&ALuaActor::GetWorld),
        LUA_BIND_FUNC(&ALuaActor::GetUUID),
        LUA_BIND_FUNC(&ALuaActor::GetClass),

        // AActor 메서드
        LUA_BIND_FUNC(&ALuaActor::GetActorLocation),
        LUA_BIND_FUNC(&ALuaActor::GetActorRotation),
        LUA_BIND_FUNC(&ALuaActor::GetActorScale),

        LUA_BIND_FUNC(&ALuaActor::GetActorForwardVector),
        LUA_BIND_FUNC(&ALuaActor::GetActorRightVector),
        LUA_BIND_FUNC(&ALuaActor::GetActorUpVector),

        LUA_BIND_FUNC(&ALuaActor::SetActorLocation),
        LUA_BIND_FUNC(&ALuaActor::SetActorRotation),
        LUA_BIND_FUNC(&ALuaActor::SetActorScale),

        LUA_BIND_FUNC(&ALuaActor::GetRootComponent),
        LUA_BIND_FUNC(&ALuaActor::SetRootComponent),
        LUA_BIND_FUNC(&ALuaActor::GetOwner),
        LUA_BIND_FUNC(&ALuaActor::SetOwner),

        LUA_BIND_FUNC(&ALuaActor::Destroy),
        LUA_BIND_FUNC(&ALuaActor::IsActorBeingDestroyed)
    );
}

void LuaTypes::FBindLua<ALuaPlayer>::Bind(sol::table& Table)
{
    Table.Lua_NewUserType(
        ALuaPlayer,
        sol::base_classes, sol::bases<ALuaActor>(),
        LUA_BIND_FUNC(&ALuaPlayer::GetAimDirection),
        LUA_BIND_FUNC(&ALuaPlayer::SpawnHeart)
    );
}

void LuaTypes::FBindLua<ALuaCoachActor>::Bind(sol::table& Table)
{
    Table.Lua_NewUserType(
        ALuaCoachActor,
        sol::base_classes, sol::bases<ALuaActor>(),
        LUA_BIND_FUNC(&ALuaCoachActor::GetAffection),
        LUA_BIND_FUNC(&ALuaCoachActor::SetAffection)
    );
}
