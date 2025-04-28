#pragma once
#include <functional>
#include "Core/Container/Map.h"

#define FUNC_DECLARE_DELEGATE(DelegateName, ReturnType, ...) \
	using DelegateName = TDelegate<ReturnType(__VA_ARGS__)>;

#define FUNC_DECLARE_MULTICAST_DELEGATE(MulticastDelegateName, ReturnType, ...) \
	using MulticastDelegateName = TMulticastDelegate<ReturnType(__VA_ARGS__)>;

class FDelegateHandle
{
	friend struct std::hash<FDelegateHandle>;

	uint64 HandleId;
	explicit FDelegateHandle() : HandleId(0) {}
	explicit FDelegateHandle(uint64 HandleId) : HandleId(HandleId) {}

	static uint64 GenerateNewID()
	{
		static std::atomic<uint64> NextHandleId = 1;
		uint64 Result = NextHandleId.fetch_add(1, std::memory_order_relaxed);

		// Overflow가 발생했다면
		if (Result == 0)
		{
			// 한번 더 더하기
			Result = NextHandleId.fetch_add(1, std::memory_order_relaxed);
		}

		return Result;
	}

public:
	static FDelegateHandle CreateHandle()
	{
		return FDelegateHandle{GenerateNewID()};
	}

	bool IsValid() const { return HandleId != 0; }
	void Invalidate() { HandleId = 0; }

	bool operator==(const FDelegateHandle& Other) const
	{
		return HandleId == Other.HandleId;
	}

	bool operator!=(const FDelegateHandle& Other) const
	{
		return HandleId != Other.HandleId;
	}
};

template <>
struct std::hash<FDelegateHandle>
{
    size_t operator()(const FDelegateHandle& InHandle) const noexcept
    {
        return std::hash<uint64>()(InHandle.HandleId);
    }
};

template <typename Signature>
class TDelegate;

template <typename ReturnType, typename... ParamTypes>
class TDelegate<ReturnType(ParamTypes...)>
{
    // TODO: std::function 사용 안하고 직접 TFunction 구현하기
	using FuncType = std::function<ReturnType(ParamTypes...)>;
	FuncType Func;

public:
	template <typename FunctorType>
	void BindLambda(FunctorType&& InFunctor)
	{
	    Func = [Func = std::forward<FunctorType>(InFunctor)](ParamTypes... Params)
	    {
	        return Func(std::forward<ParamTypes>(Params)...);
	    };
	}

    // 비-const 멤버 함수용 AddDynamic
    template <typename UserClass>
     void AddDynamic(UserClass* Obj, ReturnType(UserClass::* InMethod)(ParamTypes...))
	{
	    Func = [Obj, InMethod](ParamTypes... args) -> ReturnType {
	        return (Obj->*InMethod)(std::forward<ParamTypes>(args)...);
	    };
	}

    // const 멤버 함수용 AddDynamic
    template <typename UserClass>
    void AddDynamic(UserClass* Obj, ReturnType(UserClass::* InMethod)(ParamTypes...) const)
	{
	    Func = [Obj, InMethod](ParamTypes... args) -> ReturnType {
	        return (Obj->*InMethod)(std::forward<ParamTypes>(args)...);
	    };
	}

    template <typename T>
    void BindDynamic(T* Instance, void (T::*Func)(ParamTypes...))
	{
	    this->Func = [Instance, Func](ParamTypes... args)
	    {
	        (Instance->*Func)(std::forward<ParamTypes>(args)...);
	    };
	}

	void UnBind()
	{
		Func = nullptr;
	}

	bool IsBound() const
	{
	    return static_cast<bool>(Func);
	}

	ReturnType Execute(ParamTypes... InArgs) const
	{
		return Func(std::forward<ParamTypes>(InArgs)...);
	}

	bool ExecuteIfBound(ParamTypes... InArgs) const
	{
		if (IsBound())
		{
			Execute(std::forward<ParamTypes>(InArgs)...);
			return true;
		}
		return false;
	}
};

template <typename Signature>
class TMulticastDelegate;

template <typename ReturnType, typename... ParamTypes>
class TMulticastDelegate<ReturnType(ParamTypes...)>
{
    // TODO: std::function 사용 안하고 직접 TFunction 구현하기
	using FuncType = std::function<ReturnType(ParamTypes...)>;
    TMap<FDelegateHandle, FuncType> DelegateHandles;
    TMap<const char*, FDelegateHandle> DelegateHandlesByName;
    
public:
	template <typename FunctorType>
	FDelegateHandle AddLambda(const char* FuncName, FunctorType&& InFunctor)
	{
		FDelegateHandle DelegateHandle = FDelegateHandle::CreateHandle();
        
        DelegateHandles.Add(
            DelegateHandle,
            [Func = std::forward<FunctorType>(InFunctor)](ParamTypes... Params) mutable
            {
                Func(std::forward<ParamTypes>(Params)...);
            }
        );
	    DelegateHandlesByName.Add(FuncName, DelegateHandle);
		return DelegateHandle;
	}

    // 비-const 멤버 함수 바인딩
    template <typename UserClass>
    FDelegateHandle AddDynamic(UserClass* Obj, ReturnType(UserClass::* InMethod)(ParamTypes...))
    {
        // 새로운 핸들 생성
        FDelegateHandle Handle = FDelegateHandle::CreateHandle();
        // 멤버 함수 호출 람다 저장
        DelegateHandles.Add(
            Handle,
            [Obj, InMethod](ParamTypes... args) -> ReturnType {
                return (Obj->*InMethod)(std::forward<ParamTypes>(args)...);
            }
        );
        return Handle;
    }

    // const 멤버 함수 바인딩
    template <typename UserClass>
    FDelegateHandle AddDynamic(UserClass* Obj, ReturnType(UserClass::* InMethod)(ParamTypes...) const)
    {
        FDelegateHandle Handle = FDelegateHandle::CreateHandle();
        DelegateHandles.Add(
            Handle,
            [Obj, InMethod](ParamTypes... args) -> ReturnType {
                return (Obj->*InMethod)(std::forward<ParamTypes>(args)...);
            }
        );
        return Handle;
    }

	bool Remove(FDelegateHandle Handle)
	{
	    if (Handle.IsValid())
	    {
	        DelegateHandles.Remove(Handle);
	        return true;
	    }
	    return false;
	}

    void RemoveByName(const char* Name)
	{
	    if (const FDelegateHandle* HandlePtr = DelegateHandlesByName.Find(Name))
	    {
	        DelegateHandles.Remove(*HandlePtr);
	        DelegateHandlesByName.Remove(Name);
	    }
	}

	void Broadcast(ParamTypes... Params) const
	{
		auto CopyDelegates = DelegateHandles;
		for (const auto& [Handle, Delegate] : CopyDelegates)
		{
			Delegate(std::forward<ParamTypes>(Params)...);  // NOLINT(bugprone-use-after-move)
		}
	}
};
