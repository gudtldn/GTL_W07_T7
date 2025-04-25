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
        LUA_BIND_MEMBER(&FColor::R),
        LUA_BIND_MEMBER(&FColor::G),
        LUA_BIND_MEMBER(&FColor::B),
        LUA_BIND_MEMBER(&FColor::A),

        // 연산자 오버로딩
        sol::meta_function::equal_to, &FColor::operator==,

        // 정적 상수 (읽기 전용 속성으로 바인딩)
        LUA_BIND_VAR(FColor::White),
        LUA_BIND_VAR(FColor::Black),
        LUA_BIND_VAR(FColor::Transparent),
        LUA_BIND_VAR(FColor::Red),
        LUA_BIND_VAR(FColor::Green),
        LUA_BIND_VAR(FColor::Blue),
        LUA_BIND_VAR(FColor::Yellow)
    );
}

void LuaTypes::Math::BindFLinearColorToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FLinearColor,

        // Constructors
        sol::constructors<FLinearColor(), FLinearColor(float, float, float, float)>(),

        // Member variables
        LUA_BIND_MEMBER(&FLinearColor::R),
        LUA_BIND_MEMBER(&FLinearColor::G),
        LUA_BIND_MEMBER(&FLinearColor::B),
        LUA_BIND_MEMBER(&FLinearColor::A),

        // Operators
        sol::meta_function::equal_to, &FLinearColor::operator==,
        sol::meta_function::multiplication, [](const FLinearColor& A, const FLinearColor& B) { return A * B; },
        sol::meta_function::addition, &FLinearColor::operator+,

        // Static constants
        LUA_BIND_VAR(FLinearColor::White),
        LUA_BIND_VAR(FLinearColor::Black),
        LUA_BIND_VAR(FLinearColor::Transparent),
        LUA_BIND_VAR(FLinearColor::Red),
        LUA_BIND_VAR(FLinearColor::Green),
        LUA_BIND_VAR(FLinearColor::Blue),
        LUA_BIND_VAR(FLinearColor::Yellow)
    );
}

void LuaTypes::Math::BindFVectorToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FVector,

        // Constructors
        sol::constructors<FVector(), FVector(float, float, float)>(),

        // Member variables
        LUA_BIND_MEMBER(&FVector::X),
        LUA_BIND_MEMBER(&FVector::Y),
        LUA_BIND_MEMBER(&FVector::Z),

        // Operators
        sol::meta_function::equal_to, &FVector::operator==,
        sol::meta_function::addition, &FVector::operator+,
        sol::meta_function::subtraction, [](const FVector& A, const FVector& B) { return A - B; },
        sol::meta_function::multiplication, [](const FVector& A, const FVector& B) { return A * B; },
        sol::meta_function::division, [](const FVector& A, const FVector& B) { return A / B; },

        // Utility functions
        LUA_BIND_MEMBER(&FVector::Length),
        LUA_BIND_MEMBER(&FVector::SquaredLength),
        LUA_BIND_MEMBER(&FVector::Normalize),
        LUA_BIND_MEMBER(&FVector::IsNormalized),
        LUA_BIND_MEMBER(&FVector::DotProduct),
        LUA_BIND_MEMBER(&FVector::CrossProduct),

        // Static properties
        LUA_BIND_VAR(FVector::ZeroVector),
        LUA_BIND_VAR(FVector::OneVector),
        LUA_BIND_VAR(FVector::UpVector),
        LUA_BIND_VAR(FVector::ForwardVector),
        LUA_BIND_VAR(FVector::RightVector)
    );
}

void LuaTypes::Math::BindFVector2DToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FVector2D,

        // Constructors
        sol::constructors<FVector2D(), FVector2D(float, float)>(),

        // Member variables
        LUA_BIND_MEMBER(&FVector2D::X),
        LUA_BIND_MEMBER(&FVector2D::Y),

        // Operators
        sol::meta_function::equal_to, &FVector2D::operator==,
        sol::meta_function::addition, &FVector2D::operator+,
        sol::meta_function::subtraction, &FVector2D::operator-,
        sol::meta_function::multiplication, &FVector2D::operator*,
        sol::meta_function::division, &FVector2D::operator/,

        // Utility functions
        // LUA_BIND_MEMBER(&FVector2D::Length),
        // LUA_BIND_MEMBER(&FVector2D::LengthSquared),
        // LUA_BIND_MEMBER(&FVector2D::Normalize),
        // LUA_BIND_MEMBER(&FVector2D::IsNormalized),
        // LUA_BIND_MEMBER(&FVector2D::DotProduct),

        // Static properties
        LUA_BIND_VAR(FVector2D::ZeroVector)
        // LUA_BIND_VAR(FVector2D::UnitVector)
    );
}

void LuaTypes::Math::BindFVector4ToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FVector4,

        // Constructors
        sol::constructors<FVector4(), FVector4(float, float, float, float)>(),

        // Member variables
        LUA_BIND_MEMBER(&FVector4::X),
        LUA_BIND_MEMBER(&FVector4::Y),
        LUA_BIND_MEMBER(&FVector4::Z),
        LUA_BIND_MEMBER(&FVector4::W)
    );
}

void LuaTypes::Math::BindFRotatorToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FRotator,

        // Constructors
        sol::constructors<FRotator(), FRotator(float, float, float)>(),

        // Member variables
        LUA_BIND_MEMBER(&FRotator::Pitch),
        LUA_BIND_MEMBER(&FRotator::Yaw),
        LUA_BIND_MEMBER(&FRotator::Roll),

        // Operators
        sol::meta_function::equal_to, &FRotator::operator==,
        sol::meta_function::addition, &FRotator::operator+,
        sol::meta_function::subtraction, [](const FRotator& A, const FRotator& B) { return A - B; },
        sol::meta_function::multiplication, &FRotator::operator*,

        // Utility functions
        LUA_BIND_MEMBER(&FRotator::Normalize),
        LUA_BIND_MEMBER(&FRotator::GetNormalized)
        // LUA_BIND_MEMBER(&FRotator::GetInverse),

        // Static properties
        // LUA_BIND_VAR(FRotator::ZeroRotator)
    );
}

void LuaTypes::Math::BindFQuatToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FQuat,

        // Constructors
        sol::constructors<FQuat(), FQuat(float, float, float, float)>(),

        // Member variables
        LUA_BIND_MEMBER(&FQuat::X),
        LUA_BIND_MEMBER(&FQuat::Y),
        LUA_BIND_MEMBER(&FQuat::Z),
        LUA_BIND_MEMBER(&FQuat::W),

        // Utility functions
        LUA_BIND_MEMBER(&FQuat::Normalize),
        LUA_BIND_MEMBER(&FQuat::IsNormalized),
        LUA_BIND_MEMBER(&FQuat::RotateVector)
        // LUA_BIND_MEMBER(&FQuat::GetAxisX),
        // LUA_BIND_MEMBER(&FQuat::GetAxisY),
        // LUA_BIND_MEMBER(&FQuat::GetAxisZ),

        // Static functions
        // LUA_BIND_VAR(FQuat::Identity)
    );
}

void LuaTypes::Math::BindFMatrixToLua(sol::table& Table)
{
    Table.Lua_New_UserType(
        FMatrix,

        // Constructors
        sol::constructors<FMatrix()>(),

        // Matrix operations
        // LUA_BIND_MEMBER(&FMatrix::Determinant),
        // LUA_BIND_MEMBER(&FMatrix::Inverse),
        // LUA_BIND_MEMBER(&FMatrix::Transpose),
        // LUA_BIND_MEMBER(&FMatrix::TransformVector),
        // LUA_BIND_MEMBER(&FMatrix::TransformPosition),

        // Static functions
        LUA_BIND_VAR(FMatrix::Identity)
    );
}
