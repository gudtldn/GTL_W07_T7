#pragma once
#include <functional>
#include "Core/Container/Map.h"
#include "UObject/SafeObjectPtr.h"

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
	    Func = [Func = std::forward<FunctorType>(InFunctor)](ParamTypes... Params) mutable
	    {
	        return Func(std::forward<ParamTypes>(Params)...);
	    };
	}

    template <typename UserClass, typename FunctorType>
        requires std::derived_from<UserClass, UObject>
    void BindWeakLambda(UserClass* InUserObject, FunctorType&& InFunctor)
    {
        Func = [
            SafeObject = TSafeObjectPtr<UserClass>(InUserObject),
            Func = std::forward<FunctorType>(InFunctor)
        ](ParamTypes... Params) mutable
        {
            if (SafeObject.IsValid())
            {
                return Func(std::forward<ParamTypes>(Params)...);
            }

            UE_LOG(ELogLevel::Warning, "TDelegate executing on invalid object. Returning default value.");
            return ReturnType{};
        };
    }

    template <typename UserClass, typename MethodType>
        requires std::derived_from<UserClass, UObject> && std::is_member_function_pointer_v<MethodType>
    void BindUObject(UserClass* Obj, MethodType InMethod)
    {
        Func = [SafeObj = TSafeObjectPtr<UserClass>(Obj), InMethod](ParamTypes... Params) mutable
        {
            if (SafeObj.IsValid())
            {
                return (SafeObj->*InMethod)(std::forward<ParamTypes>(Params)...);
            }

            UE_LOG(ELogLevel::Warning, "TDelegate executing on invalid object. Returning default value.");
            return ReturnType{};
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

public:
    template <typename FunctorType>
    FDelegateHandle AddLambda(FunctorType&& InFunctor)
    {
        FDelegateHandle Handle = FDelegateHandle::CreateHandle();
        DelegateHandles.Add(
            Handle,
            [Func = std::forward<FunctorType>(InFunctor)](ParamTypes... Params) mutable
            {
                Func(std::forward<ParamTypes>(Params)...);
            }
        );
        return Handle;
    }

    template <typename UserClass, typename FunctorType>
        requires std::derived_from<UserClass, UObject>
    FDelegateHandle AddWeakLambda(UserClass* InUserObject, FunctorType&& InFunctor)
    {
        FDelegateHandle Handle = FDelegateHandle::CreateHandle();
        DelegateHandles.Add(
            Handle,
            [
                this, Handle,
                SafeObject = TSafeObjectPtr<UserClass>(InUserObject),
                Func = std::forward<FunctorType>(InFunctor)
            ](ParamTypes... Params) mutable
            {
                if (SafeObject.IsValid())
                {
                    return Func(std::forward<ParamTypes>(Params)...);
                }

                Remove(Handle); // 유효한 포인터가 사라지면 델리게이트에서 제거
                return ReturnType{};
            }
        );
        return Handle;
    }

    template <typename UserClass, typename MethodType>
        requires std::derived_from<UserClass, UObject> && std::is_member_function_pointer_v<MethodType>
    FDelegateHandle AddUObject(UserClass* InUserObject, MethodType InMethod)
    {
        FDelegateHandle Handle = FDelegateHandle::CreateHandle();
        DelegateHandles.Add(
            Handle,
            [
                this, Handle,
                SafeObj = TSafeObjectPtr<UserClass>(InUserObject),
                InMethod
            ](ParamTypes... Params)
            {
                if (SafeObj.IsValid())
                {
                    return (SafeObj->*InMethod)(std::forward<ParamTypes>(Params)...);
                }
    
                Remove(Handle); // 유효한 포인터가 사라지면 델리게이트에서 제거
                return ReturnType{};
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

	void Broadcast(ParamTypes... Params) const
	{
		auto CopyDelegates = DelegateHandles;
		for (const auto& [Handle, Delegate] : CopyDelegates)
		{
			Delegate(std::forward<ParamTypes>(Params)...);
		}
	}
};
