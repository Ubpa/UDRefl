#pragma once

namespace Ubpa::UDRefl::details {
	template<typename ArgList>
	struct wrap_function_call;
	template<typename OrigArgList, typename BufferArgList>
	struct wrap_function_call_impl;

	template<typename... Args>
	struct wrap_function_call<TypeList<Args...>> {
		template<typename Obj, auto func_ptr, typename MaybeConstVoidPtr>
		static constexpr decltype(auto) run(MaybeConstVoidPtr ptr, ArgPtrBuffer argptr_buffer) {
			return std::apply(
				[ptr](auto... pointers) -> decltype(auto) {
					return (buffer_as<Obj>(ptr).*func_ptr)(std::forward<Args>(*reinterpret_cast<std::add_const_t<std::add_pointer_t<Args>>>(pointers))...);
				},
				* reinterpret_cast<const std::array<void* const, sizeof...(Args)>*>(argptr_buffer)
			);
		}
		template<auto func_ptr>
		static constexpr decltype(auto) run(ArgPtrBuffer argptr_buffer) {
			return std::apply(
				[](auto... pointers) -> decltype(auto) {
					return func_ptr(std::forward<Args>(*reinterpret_cast<std::add_const_t<std::add_pointer_t<Args>>>(pointers))...);
				},
				* reinterpret_cast<const std::array<void* const, sizeof...(Args)>*>(argptr_buffer)
			);
		}
		template<typename Obj, typename Func, typename MaybeConstVoidPtr>
		static constexpr decltype(auto) run(MaybeConstVoidPtr ptr, Func&& func, ArgPtrBuffer argptr_buffer) {
			return std::apply(
				[ptr, f = std::forward<Func>(func)](auto*... pointers) -> decltype(auto) {
					if constexpr (std::is_member_function_pointer_v<std::decay_t<Func>>)
						return (buffer_as<Obj>(ptr).*f)(std::forward<Args>(*reinterpret_cast<std::add_const_t<std::add_pointer_t<Args>>>(pointers))...);
					else {
						return f(
							buffer_as<Obj>(ptr),
							std::forward<Args>(*reinterpret_cast<std::add_pointer_t<Args>>(pointers))...
						);
					}
				},
				* reinterpret_cast<const std::array<void* const, sizeof...(Args)>*>(argptr_buffer)
			);
		}
		template<typename Func>
		static constexpr decltype(auto) run(Func&& func, ArgPtrBuffer argptr_buffer) {
			return std::apply(
				[f = std::forward<Func>(func)](auto... pointers) -> decltype(auto) {
					return f(std::forward<Args>(*reinterpret_cast<std::add_const_t<std::add_pointer_t<Args>>>(pointers))...);
				},
				* reinterpret_cast<const std::array<void* const, sizeof...(Args)>*>(argptr_buffer)
			);
		}
	};

	template<typename F>
	struct WrapFuncTraits;

	template<typename Func, typename Obj>
	struct WrapFuncTraits<Func Obj::*> : FuncTraits<Func Obj::*> {
	private:
		using Traits = FuncTraits<Func>;
	public:
		using Object = Obj;
		using ArgList = typename Traits::ArgList;
		using Return = typename Traits::Return;
		static constexpr bool is_const = Traits::is_const;
	};

	template<typename F>
	struct WrapFuncTraits {
	private:
		using Traits = FuncTraits<F>;
		using ObjectArgList = typename Traits::ArgList;
		static_assert(!IsEmpty_v<ObjectArgList>);
		using CVObjRef = Front_t<ObjectArgList>;
		using CVObj = std::remove_reference_t<CVObjRef>;
	public:
		using ArgList = PopFront_t<ObjectArgList>;
		using Object = std::remove_cv_t<CVObj>;
		using Return = typename Traits::Return;
		static constexpr bool is_const = std::is_const_v<CVObj>;
		static_assert(is_const || !std::is_rvalue_reference_v<CVObjRef>);
	};
}

template<auto func_ptr>
constexpr auto Ubpa::UDRefl::wrap_member_function() noexcept {
	using FuncPtr = decltype(func_ptr);
	static_assert(std::is_member_function_pointer_v<FuncPtr>);
	using Traits = FuncTraits<FuncPtr>;
	static_assert(!(Traits::ref == ReferenceMode::Right && !Traits::is_const));
	using Obj = typename Traits::Object;
	using Return = typename Traits::Return;
	using ArgList = typename Traits::ArgList;
	using MaybeConstVoidPtr = std::conditional_t<Traits::is_const, const void*, void*>;
	constexpr auto wrapped_function = [](MaybeConstVoidPtr obj, void* result_buffer, ArgPtrBuffer argptr_buffer) {
		if constexpr (!std::is_void_v<Return>) {
			using NonCVReturn = std::remove_cv_t<Return>;
			NonCVReturn rst = details::wrap_function_call<ArgList>::template run<Obj, func_ptr>(obj, argptr_buffer);
			if (result_buffer) {
				if constexpr (std::is_reference_v<Return>)
					buffer_as<std::add_pointer_t<Return>>(result_buffer) = &rst;
				else
					buffer_as<NonCVReturn>(result_buffer) = std::move(rst);
			}
		}
		else
			details::wrap_function_call<ArgList>::template run<Obj, func_ptr>(obj, argptr_buffer);
	};
	return wrapped_function;
}

template<typename Func>
constexpr auto Ubpa::UDRefl::wrap_member_function(Func&& func) noexcept {
	using Traits = details::WrapFuncTraits<std::decay_t<Func>>;
	using Return = typename Traits::Return;
	using Obj = typename Traits::Object;
	using ArgList = typename Traits::ArgList;
	using MaybeConstVoidPtr = std::conditional_t<Traits::is_const, const void*, void*>;
	/*constexpr*/ auto wrapped_function =
		[f = std::forward<Func>(func)](MaybeConstVoidPtr obj, void* result_buffer, ArgPtrBuffer argptr_buffer) mutable {
		if constexpr (!std::is_void_v<Return>) {
			using NonCVReturn = std::remove_cv_t<Return>;
			NonCVReturn rst = details::wrap_function_call<ArgList>::template run<Obj>(obj, std::forward<Func>(f), argptr_buffer);
			if (result_buffer) {
				if constexpr (std::is_reference_v<Return>)
					buffer_as<std::add_pointer_t<Return>>(result_buffer) = &rst;
				else
					new(result_buffer)NonCVReturn{ std::move(rst) };
			}
		}
		else
			details::wrap_function_call<ArgList>::template run<Obj>(obj, std::forward<Func>(f), argptr_buffer);
	};
	return wrapped_function;
}

template<auto func_ptr>
constexpr auto Ubpa::UDRefl::wrap_static_function() noexcept {
	using FuncPtr = decltype(func_ptr);
	static_assert(is_function_pointer_v<FuncPtr>);
	using Traits = FuncTraits<FuncPtr>;
	using Return = typename Traits::Return;
	using ArgList = typename Traits::ArgList;
	constexpr auto wrapped_function = [](void* result_buffer, ArgPtrBuffer argptr_buffer) {
		if constexpr (!std::is_void_v<Return>) {
			using NonCVReturn = std::remove_cv_t<Return>;
			NonCVReturn rst = details::wrap_function_call<ArgList>::template run<func_ptr>(argptr_buffer);
			if (result_buffer) {
				if constexpr (std::is_reference_v<Return>)
					buffer_as<std::add_pointer_t<Return>>(result_buffer) = &rst;
				else
					new(result_buffer)NonCVReturn{ std::move(rst) };
			}
		}
		else
			details::wrap_function_call<ArgList>::template run<func_ptr>(argptr_buffer);
	};
	return wrapped_function;
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr decltype(auto) Ubpa::UDRefl::enum_cast(Enum&& e) noexcept {
	using E = decltype(e);
	using T = std::underlying_type_t<std::remove_cvref_t<Enum>>;
	if constexpr (std::is_reference_v<E>) {
		using UnrefE = std::remove_reference_t<E>;
		if constexpr (std::is_lvalue_reference_v<E>) {
			if constexpr (std::is_const_v<UnrefE>)
				return static_cast<const T&>(e);
			else
				return static_cast<T&>(e);
		}
		else if constexpr (std::is_rvalue_reference_v<E>) {
			if constexpr (std::is_const_v<UnrefE>)
				return static_cast<const T&&>(e);
			else
				return static_cast<T&&>(e);
		}
		else {
			if constexpr (std::is_const_v<UnrefE>)
				return static_cast<const T>(e);
			else
				return static_cast<T>(e);
		}
	}

}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr bool Ubpa::UDRefl::enum_empty(const Enum& e) noexcept {
	using T = std::underlying_type_t<Enum>;
	return static_cast<T>(e);
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr bool Ubpa::UDRefl::enum_contain_any(const Enum& e, const Enum& flag) noexcept {
	using T = std::underlying_type_t<Enum>;
	return static_cast<T>(e) & static_cast<T>(flag);
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr bool Ubpa::UDRefl::enum_contain(const Enum& e, const Enum& flag) noexcept {
	using T = std::underlying_type_t<Enum>;
	const auto flag_T = static_cast<T>(flag);
	return (static_cast<T>(e) & flag_T) == flag_T;
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr Enum Ubpa::UDRefl::enum_combine(std::initializer_list<Enum> flags) noexcept {
	using T = std::underlying_type_t<Enum>;
	T rst = 0;
	for (const auto& flag : flags)
		rst |= static_cast<T>(flag);
	return static_cast<Enum>(rst);
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr Enum Ubpa::UDRefl::enum_remove(const Enum& e, const Enum& flag) noexcept {
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<T>(e) & (~static_cast<T>(flag)));
}

template<typename Enum> requires std::is_enum_v<Enum>
constexpr Enum Ubpa::UDRefl::enum_within(const Enum& e, const Enum& flag) noexcept {
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<T>(e) & (static_cast<T>(flag)));
}

template<typename Func>
constexpr auto Ubpa::UDRefl::wrap_static_function(Func&& func) noexcept {
	using Traits = FuncTraits<std::decay_t<Func>>;
	using Return = typename Traits::Return;
	using ArgList = typename Traits::ArgList;
	/*constexpr*/ auto wrapped_function =
		[f = std::forward<Func>(func)](void* result_buffer, ArgPtrBuffer argptr_buffer) mutable {
			if constexpr (!std::is_void_v<Return>) {
				using NonCVReturn = std::remove_cv_t<Return>;
				NonCVReturn rst = details::wrap_function_call<ArgList>::template run(std::forward<Func>(f), argptr_buffer);
				if (result_buffer) {
					if constexpr (std::is_reference_v<Return>)
						buffer_as<std::add_pointer_t<Return>>(result_buffer) = &rst;
					else
						new(result_buffer)NonCVReturn{ std::move(rst) };
				}
			}
			else
				details::wrap_function_call<ArgList>::template run(std::forward<Func>(f), argptr_buffer);
		};
	return wrapped_function;
}

template<auto func_ptr>
constexpr auto Ubpa::UDRefl::wrap_function() noexcept {
	using FuncPtr = decltype(func_ptr);
	if constexpr (is_function_pointer_v<FuncPtr>)
		return wrap_static_function<func_ptr>();
	else if constexpr (std::is_member_function_pointer_v<FuncPtr>)
		return wrap_member_function<func_ptr>();
	else
		static_assert(always_false<decltype(func_ptr)>);
}
