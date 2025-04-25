#include "LuaMathTypes.h"

#include "Developer/Lua/LuaUtils/LuaBindMacros.h"
#include "Math/Color.h"
#include "Math/Matrix.h"
#include "Math/Quat.h"


void LuaTypes::Math::BindFColorToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FColor,

        // 생성자
        sol::constructors<FColor(), FColor(uint8, uint8, uint8, uint8)>(),

        // 멤버 변수
        "R", &FColor::R,
        "G", &FColor::G,
        "B", &FColor::B,
        "A", &FColor::A,

        // 연산자 오버로딩
        sol::meta_function::equal_to, &FColor::operator==,

        // 정적 상수 (읽기 전용 속성으로 바인딩)
        "White", sol::var(FColor::White),
        "Black", sol::var(FColor::Black),
        "Transparent", sol::var(FColor::Transparent),
        "Red", sol::var(FColor::Red),
        "Green", sol::var(FColor::Green),
        "Blue", sol::var(FColor::Blue),
        "Yellow", sol::var(FColor::Yellow)
    );
}

void LuaTypes::Math::BindFLinearColorToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FLinearColor,

        // Constructors
        sol::constructors<FLinearColor(), FLinearColor(float, float, float, float)>(),

        // Member variables
        "R", &FLinearColor::R,
        "G", &FLinearColor::G,
        "B", &FLinearColor::B,
        "A", &FLinearColor::A,

        // Operators
        sol::meta_function::equal_to, &FLinearColor::operator==,
        sol::meta_function::multiplication, [](const FLinearColor& A, const FLinearColor& B) { return A * B; },
        sol::meta_function::addition, &FLinearColor::operator+,

        // Static constants
        "White", sol::var(FLinearColor::White),
        "Black", sol::var(FLinearColor::Black),
        "Transparent", sol::var(FLinearColor::Transparent),
        "Red", sol::var(FLinearColor::Red),
        "Green", sol::var(FLinearColor::Green),
        "Blue", sol::var(FLinearColor::Blue),
        "Yellow", sol::var(FLinearColor::Yellow)
    );
}

void LuaTypes::Math::BindFVectorToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FVector,

        // Constructors
        sol::constructors<FVector(), FVector(float, float, float)>(),

        // Member variables
        "X", &FVector::X,
        "Y", &FVector::Y,
        "Z", &FVector::Z,

        // Operators
        sol::meta_function::equal_to, &FVector::operator==,
        sol::meta_function::addition, &FVector::operator+,
        sol::meta_function::subtraction, [](const FVector& A, const FVector& B) { return A - B; },
        sol::meta_function::multiplication, [](const FVector& A, const FVector& B) { return A * B; },
        sol::meta_function::division, [](const FVector& A, const FVector& B) { return A / B; },

        // Utility functions
        "Length", &FVector::Length,
        "SquaredLength", &FVector::SquaredLength,
        "Normalize", &FVector::Normalize,
        "IsNormalized", &FVector::IsNormalized,
        "DotProduct", &FVector::DotProduct,
        "CrossProduct", &FVector::CrossProduct,

        // Static properties
        "ZeroVector", sol::var(FVector::ZeroVector),
        "OneVector", sol::var(FVector::OneVector),
        "UpVector", sol::var(FVector::UpVector),
        "ForwardVector", sol::var(FVector::ForwardVector),
        "RightVector", sol::var(FVector::RightVector)
    );
}

void LuaTypes::Math::BindFVector2DToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FVector2D,

        // Constructors
        sol::constructors<FVector2D(), FVector2D(float, float)>(),

        // Member variables
        "X", &FVector2D::X,
        "Y", &FVector2D::Y,

        // Operators
        sol::meta_function::equal_to, &FVector2D::operator==,
        sol::meta_function::addition, &FVector2D::operator+,
        sol::meta_function::subtraction, &FVector2D::operator-,
        sol::meta_function::multiplication, &FVector2D::operator*,
        sol::meta_function::division, &FVector2D::operator/,

        // Utility functions
        // "Length", &FVector2D::Length,
        // "SquaredLength", &FVector2D::LengthSquared,
        // "Normalize", &FVector2D::Normalize,
        // "IsNormalized", &FVector2D::IsNormalized,
        // "DotProduct", &FVector2D::DotProduct,

        // Static properties
        "ZeroVector", sol::var(FVector2D::ZeroVector)
        // "UnitVector", sol::var(FVector2D::UnitVector)
    );
}

void LuaTypes::Math::BindFVector4ToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FVector4,

        // Constructors
        sol::constructors<FVector4(), FVector4(float, float, float, float)>(),

        // Member variables
        "X", &FVector4::X,
        "Y", &FVector4::Y,
        "Z", &FVector4::Z,
        "W", &FVector4::W
    );
}

void LuaTypes::Math::BindFRotatorToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FRotator,

        // Constructors
        sol::constructors<FRotator(), FRotator(float, float, float)>(),

        // Member variables
        "Pitch", &FRotator::Pitch,
        "Yaw", &FRotator::Yaw,
        "Roll", &FRotator::Roll,

        // Operators
        sol::meta_function::equal_to, &FRotator::operator==,
        sol::meta_function::addition, &FRotator::operator+,
        sol::meta_function::subtraction, [](const FRotator& A, const FRotator& B) { return A - B; },
        sol::meta_function::multiplication, &FRotator::operator*,

        // Utility functions
        "Normalize", &FRotator::Normalize,
        "GetNormalized", &FRotator::GetNormalized
        // "GetInverse", &FRotator::GetInverse,

        // Static properties
        // "ZeroRotator", sol::var(FRotator::ZeroRotator)
    );
}

void LuaTypes::Math::BindFQuatToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FQuat,

        // Constructors
        sol::constructors<FQuat(), FQuat(float, float, float, float)>(),

        // Member variables
        "X", &FQuat::X,
        "Y", &FQuat::Y,
        "Z", &FQuat::Z,
        "W", &FQuat::W,

        // Utility functions
        "Normalize", &FQuat::Normalize,
        "IsNormalized", &FQuat::IsNormalized,
        "RotateVector", &FQuat::RotateVector
        // "GetAxisX", &FQuat::GetAxisX,
        // "GetAxisY", &FQuat::GetAxisY,
        // "GetAxisZ", &FQuat::GetAxisZ,

        // Static functions
        // "Identity", sol::var(FQuat::Identity)
    );
}

void LuaTypes::Math::BindFMatrixToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FMatrix,

        // Constructors
        sol::constructors<FMatrix()>(),

        // Matrix operations
        // "Determinant", &FMatrix::Determinant,
        // "Inverse", &FMatrix::Inverse,
        // "Transpose", &FMatrix::Transpose,
        // "TransformVector3", &FMatrix::TransformVector,
        // "TransformPosition", &FMatrix::TransformPosition,

        // Static functions
        "Identity", sol::var(FMatrix::Identity)
    );
}
