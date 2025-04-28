#pragma once
#include <concepts>

#include "ObjectUtils.h"
#include "HAL/PlatformType.h"

/**
 * Object가 유효할때만 값을 반환하는 포인터, Object가 유효하지 않다면 nullptr
 * @tparam T UObject를 상속받은 Class
 *
 * @note TSafeObjectPtr의 소멸자가 호출이 되어도 Object는 따로 삭제를 하지 않습니다.
 */
template <typename T>
    requires std::derived_from<T, UObject>
struct TSafeObjectPtr
{
public:
    using ElementType = T;

    TSafeObjectPtr() = default;
    ~TSafeObjectPtr() = default;

    TSafeObjectPtr(const TSafeObjectPtr&) = default;
    TSafeObjectPtr& operator=(const TSafeObjectPtr&) = default;
    TSafeObjectPtr(TSafeObjectPtr&&) = default;
    TSafeObjectPtr& operator=(TSafeObjectPtr&&) = default;

    TSafeObjectPtr(nullptr_t)
        : ObjectPtr(nullptr)
    {
    }

    TSafeObjectPtr(ElementType* InPtr)
        : ObjectPtr(InPtr)
    {
    }

    TSafeObjectPtr& operator=(nullptr_t)
    {
        ObjectPtr = nullptr;
        return *this;
    }

    TSafeObjectPtr& operator=(ElementType* InPtr)
    {
        ObjectPtr = InPtr;
        return *this;
    }

public:
    FORCEINLINE ElementType* Get() const
    {
        if (ObjectPtr)
        {
            if (::IsValid(ObjectPtr))
            {
                return ObjectPtr;
            }
            ObjectPtr = nullptr;
        }
        return nullptr;
    }

    FORCEINLINE ElementType* operator->() const { return Get(); }
    FORCEINLINE ElementType& operator*() const { return *Get(); }
    FORCEINLINE operator ElementType*() const { return Get(); }
    FORCEINLINE explicit operator bool() const { return Get() != nullptr; }

public:
    bool IsValid() const { return Get() != nullptr; }

private:
    mutable ElementType* ObjectPtr = nullptr;
};
