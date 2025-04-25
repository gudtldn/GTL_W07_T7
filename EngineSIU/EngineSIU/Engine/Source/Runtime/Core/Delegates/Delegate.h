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
		FDelegateHandle DelegateHandle = FDelegateHandle::CreateHandle();

        DelegateHandles.Add(
            DelegateHandle,
            [Func = std::forward<FunctorType>(InFunctor)](ParamTypes... Params) mutable
            {
                Func(std::forward<ParamTypes>(Params)...);
            }
        );
		return DelegateHandle;
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
			Delegate(std::forward<ParamTypes>(Params)...);  // NOLINT(bugprone-use-after-move)
		}
	}
};
