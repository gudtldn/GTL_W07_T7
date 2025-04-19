#pragma once
#include "Object.h"
//
// #pragma region Define Property Types
// enum class EPropertyType : uint8
// {
//     Unknown,
//
//     // Primitive Types
//     Bool,
//     Int8,  UInt8,
//     Int16, UInt16,
//     Int32, UInt32,
//     Int64, UInt64,
//     Float, Double,
//
//     // Structs
//     Vector2D,    // FVector2D
//     Vector,      // FVector
//     Vector4,     // FVector4
//     Rotator,     // FRotator
//     Quat,        // FQuat
//     Matrix,      // FMatrix
//     Color,       // FColor
//     LinearColor, // FLinearColor
//
//     // Containers
//     String,      // FString
//     Name,        // FName
//     Array,       // TArray
//     Map,         // TMap
//     Set,         // TSet
//
//     Object,      // UObject
// };
//
// template <typename>
// struct GetPropertyTypeHelper { static constexpr EPropertyType Value = EPropertyType::Unknown; };
//
// template <> struct GetPropertyTypeHelper<bool>         { static constexpr EPropertyType Value = EPropertyType::Bool;        };
// template <> struct GetPropertyTypeHelper<int8>         { static constexpr EPropertyType Value = EPropertyType::Int8;        };
// template <> struct GetPropertyTypeHelper<uint8>        { static constexpr EPropertyType Value = EPropertyType::UInt8;       };
// template <> struct GetPropertyTypeHelper<int16>        { static constexpr EPropertyType Value = EPropertyType::Int16;       };
// template <> struct GetPropertyTypeHelper<uint16>       { static constexpr EPropertyType Value = EPropertyType::UInt16;      };
// template <> struct GetPropertyTypeHelper<int32>        { static constexpr EPropertyType Value = EPropertyType::Int32;       };
// template <> struct GetPropertyTypeHelper<uint32>       { static constexpr EPropertyType Value = EPropertyType::UInt32;      };
// template <> struct GetPropertyTypeHelper<int64>        { static constexpr EPropertyType Value = EPropertyType::Int64;       };
// template <> struct GetPropertyTypeHelper<uint64>       { static constexpr EPropertyType Value = EPropertyType::UInt64;      };
// template <> struct GetPropertyTypeHelper<float>        { static constexpr EPropertyType Value = EPropertyType::Float;       };
// template <> struct GetPropertyTypeHelper<double>       { static constexpr EPropertyType Value = EPropertyType::Double;      };
// template <> struct GetPropertyTypeHelper<FVector2D>    { static constexpr EPropertyType Value = EPropertyType::Vector2D;    };
// template <> struct GetPropertyTypeHelper<FVector>      { static constexpr EPropertyType Value = EPropertyType::Vector;      };
// template <> struct GetPropertyTypeHelper<FVector4>     { static constexpr EPropertyType Value = EPropertyType::Vector4;     };
// template <> struct GetPropertyTypeHelper<FRotator>     { static constexpr EPropertyType Value = EPropertyType::Rotator;     };
// template <> struct GetPropertyTypeHelper<FQuat>        { static constexpr EPropertyType Value = EPropertyType::Quat;        };
// template <> struct GetPropertyTypeHelper<FMatrix>      { static constexpr EPropertyType Value = EPropertyType::Matrix;      };
// template <> struct GetPropertyTypeHelper<FColor>       { static constexpr EPropertyType Value = EPropertyType::Color;       };
// template <> struct GetPropertyTypeHelper<FLinearColor> { static constexpr EPropertyType Value = EPropertyType::LinearColor; };
// template <> struct GetPropertyTypeHelper<FString>      { static constexpr EPropertyType Value = EPropertyType::String;      };
// template <> struct GetPropertyTypeHelper<FName>        { static constexpr EPropertyType Value = EPropertyType::Name;        };
//
// template <typename ElementType, typename Allocator>
// struct GetPropertyTypeHelper<TArray<ElementType, Allocator>>
// {
//     static constexpr EPropertyType Value = EPropertyType::Array;
// };
//
// template <typename KeyType, typename ValueType, typename Allocator>
// struct GetPropertyTypeHelper<TMap<KeyType, ValueType, Allocator>>
// {
//     static constexpr EPropertyType Value = EPropertyType::Map;
// };
//
// template <typename ElementType, typename Hasher, typename Allocator>
// struct GetPropertyTypeHelper<TSet<ElementType, Hasher, Allocator>>
// {
//     static constexpr EPropertyType Value = EPropertyType::Set;
// };
//
// template <typename Obj>
//     requires std::derived_from<Obj, UObject>
// struct GetPropertyTypeHelper<Obj>
// {
//     static constexpr EPropertyType Value = EPropertyType::Object;
// };
//
// template <typename T>
// constexpr EPropertyType GetPropertyType()
// {
//     using DecayedT = std::remove_cvref_t<T>;
//     return GetPropertyTypeHelper<DecayedT>::Value;
// }
// #pragma endregion


struct FProperty
{
    FProperty(const char* InName, int32 InSize, int32 InOffset)
        : Name(InName)
        , Size(InSize)
        , Offset(InOffset)
    {}

    virtual ~FProperty() = default;

    virtual void Serialize(FArchive& Ar) {}

    const char* Name;
    int64 Size;
    int64 Offset;
};

template <typename NumericType>
    requires std::is_arithmetic_v<NumericType>
struct TNumericProperty : public FProperty
{
    using ValueType = NumericType;

    TNumericProperty(const char* InName, int32 InSize, int32 InOffset)
        : FProperty(InName, InSize, InOffset)
    {}

    // virtual void Serialize(FArchive& Ar) override
    // {
    //     Ar << 
    // }
};

using FInt8Property = TNumericProperty<int8>;
using FUInt8Property = TNumericProperty<uint8>;
using FInt16Property = TNumericProperty<int16>;
using FUInt16Property = TNumericProperty<uint16>;
using FInt32Property = TNumericProperty<int32>;
using FUInt32Property = TNumericProperty<uint32>;
using FInt64Property = TNumericProperty<int64>;
using FUInt64Property = TNumericProperty<uint64>;
using FFloatProperty = TNumericProperty<float>;
using FDoubleProperty = TNumericProperty<double>;
