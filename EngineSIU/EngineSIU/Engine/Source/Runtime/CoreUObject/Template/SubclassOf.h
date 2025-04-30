#pragma once
#include <concepts>

#include "HAL/PlatformType.h"
#include "UObject/Class.h"


template <typename T>
struct TSubclassOf
{
    using ElementType = T;

public:
    TSubclassOf() = default;
    TSubclassOf(const TSubclassOf&) = default;
    TSubclassOf& operator=(const TSubclassOf&) = default;
    TSubclassOf(TSubclassOf&&) = default;
    TSubclassOf& operator=(TSubclassOf&&) = default;
    ~TSubclassOf() = default;

    TSubclassOf(UClass* InClass) :
        Class(InClass)
    {
    }

    FORCEINLINE TSubclassOf& operator=(UClass* From)
    {
        Class = From;
        return *this;
    }

    FORCEINLINE UClass* operator*() const
    {
        if (!Class || !Class->IsChildOf(T::StaticClass()))
        {
            return nullptr;
        }
        return Class;
    }

    /** UClass* 로 역참조하여 런타임 형식 검사를 수행합니다. */
    FORCEINLINE UClass* Get() const
    {
        return **this;
    }

    /** UClass* 로 역참조하여 런타임 형식 검사를 수행합니다. */
    FORCEINLINE UClass* operator->() const
    {
        return **this;
    }

    /** UClass*로의 암시적 변환으로, 런타임 형식 검사를 수행합니다. */
    FORCEINLINE operator UClass*() const
    {
        return **this;
    }

    /**
     * 유효한 클래스를 참조하는 경우 CDO를 가져옵니다.
     *
     * @return CDO 또는 클래스가 null이면 nullptr
     */
    FORCEINLINE T* GetDefaultObject() const
    {
        UObject* Result = nullptr;
        if (Class)
        {
            Result = Class->GetDefaultObject();
            assert(Result && Result->IsA(T::StaticClass()));
        }
        return static_cast<T*>(Result);
    }

    UClass* GetClassPtr() const
    {
        return Class;
    }

private:
    UClass* Class = nullptr;
};
