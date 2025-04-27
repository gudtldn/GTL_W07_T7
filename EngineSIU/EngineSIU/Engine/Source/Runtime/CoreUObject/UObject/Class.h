#pragma once
#include <concepts>
#include "Object.h"
#include "Property.h"


class FArchive;
/**
 * UObject의 RTTI를 가지고 있는 클래스
 */
class UClass : public UObject
{
    using ClassConstructorType = UObject*(*)();

public:
    UClass(
        const char* InClassName,
        uint32 InClassSize,
        uint32 InAlignment,
        UClass* InSuperClass,
        ClassConstructorType InCTOR
    );

    // 복사 & 이동 생성자 제거
    UClass(const UClass&) = delete;
    UClass& operator=(const UClass&) = delete;
    UClass(UClass&&) = delete;
    UClass& operator=(UClass&&) = delete;

    static TMap<FName, UClass*>& GetClassMap()
    {
        static TMap<FName, UClass*> ClassMap;
        return ClassMap;
    }

    static UClass* FindClass(const FName& ClassName)
    {
        auto It = GetClassMap().Find(ClassName);
        if (It)
            return *It;
        return nullptr;
    }

    uint32 GetClassSize() const { return ClassSize; }
    uint32 GetClassAlignment() const { return ClassAlignment; }

    /** SomeBase의 자식 클래스인지 확인합니다. */
    bool IsChildOf(const UClass* SomeBase) const;

    template <typename T>
        requires std::derived_from<T, UObject>
    bool IsChildOf() const;

    /**
     * 부모의 UClass를 가져옵니다.
     *
     * @note AActor::StaticClass()->GetSuperClass() == UObject::StaticClass()
     */
    UClass* GetSuperClass() const { return SuperClass; }

    UObject* GetDefaultObject() const;

    template <typename T>
        requires std::derived_from<T, UObject>
    T* GetDefaultObject() const;

    const TArray<FProperty>& GetProperties() const { return Properties; }

    /**
     * UClass에 Property를 추가합니다
     * @param Prop 추가할 Property
     */
    void RegisterProperty(const FProperty& Prop);

    /** 바이너리 직렬화 함수 */
    void SerializeBin(FArchive& Ar, void* Data);

protected:
    virtual UObject* CreateDefaultObject();


public:
    ClassConstructorType ClassCTOR;

private:
    uint32 ClassSize;
    uint32 ClassAlignment;

    UClass* SuperClass = nullptr;
    UObject* ClassDefaultObject = nullptr;

    TArray<FProperty> Properties;
};

template <typename T>
    requires std::derived_from<T, UObject>
bool UClass::IsChildOf() const
{
    return IsChildOf(T::StaticClass());
}

template <typename T>
    requires std::derived_from<T, UObject>
T* UClass::GetDefaultObject() const
{
    UObject* Ret = GetDefaultObject();
    assert(Ret->IsA<T>());
    return static_cast<T*>(Ret);
}
