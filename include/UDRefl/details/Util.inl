#pragma once

namespace Ubpa::UDRefl::details {
	template<typename ArgList>
	struct wrap_function_call;
	template<typename OrigArgList, typename BufferArgList>
	struct wrap_function_call_impl;

	template<typename... Args>
	struct wrap_function_call<TypeList<Args...>>
		: wrap_function_call_impl<TypeList<Args...>, TypeList<decltype(type_buffer_decay<Args>(std::declval<Args>()))...>> {};

	template<typename... OrigArgs, typename... BufferArgs>
	struct wrap_function_call_impl<TypeList<OrigArgs...>, TypeList<BufferArgs...>> {
		template<typename Obj, auto func_ptr, typename ObjPtr>
		static constexpr decltype(auto) run(ObjPtr ptr, void* args_buffer) {
			return std::apply(
				[ptr](auto&&... bufferArgs) -> decltype(auto) {
					return (buffer_as<Obj>(ptr).*func_ptr)(type_buffer_recover<OrigArgs>(std::forward<decltype(bufferArgs)>(bufferArgs))...);
				},
				std::move(*reinterpret_cast<std::tuple<BufferArgs...>*>(args_buffer))
			);
		}
		template<auto func_ptr>
		static constexpr decltype(auto) run(void* args_buffer) {
			return std::apply(
				[](auto&&... bufferArgs) -> decltype(auto) {
					return func_ptr(type_buffer_recover<OrigArgs>(std::forward<decltype(bufferArgs)>(bufferArgs))...);
				},
				std::move(*reinterpret_cast<std::tuple<BufferArgs...>*>(args_buffer))
			);
		}
		template<typename Obj, typename Func, typename ObjPtr>
		static constexpr decltype(auto) run(ObjPtr ptr, Func&& func, void* args_buffer) {
			return std::apply(
				[ptr, f = std::forward<Func>(func)](auto&&... bufferArgs) mutable -> decltype(auto) {
					if constexpr (std::is_member_function_pointer_v<std::decay_t<Func>>)
						return (buffer_as<Obj>(ptr).*f)(type_buffer_recover<OrigArgs>(std::forward<decltype(bufferArgs)>(bufferArgs))...);
					else {
						return std::forward<Func>(f)(
							buffer_as<Obj>(ptr),
							type_buffer_recover<OrigArgs>(std::forward<decltype(bufferArgs)>(bufferArgs))...
						);
					}
				},
				std::move(*reinterpret_cast<std::tuple<BufferArgs...>*>(args_buffer))
			);
		}
		template<typename Func>
		static constexpr decltype(auto) run(Func&& func, void* args_buffer) {
			return std::apply(
				[f = std::forward<Func>(func)](auto&&... bufferArgs) mutable -> decltype(auto) {
					return std::forward<Func>(f)(type_buffer_recover<OrigArgs>(std::forward<decltype(bufferArgs)>(bufferArgs))...);
				},
				std::move(*reinterpret_cast<std::tuple<BufferArgs...>*>(args_buffer))
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
		using CVObjectRef = Front_t<ObjectArgList>;
		static_assert(std::is_reference_v<CVObjectRef>);
		using CVObject = std::remove_reference_t<CVObjectRef>;
	public:
		using ArgList = PopFront_t<ObjectArgList>;
		using Object = std::remove_cv_t<CVObject>;
		using Return = typename Traits::Return;
		static constexpr bool is_const = std::is_const_v<CVObject>;
	};
}

template<auto func_ptr>
constexpr auto Ubpa::UDRefl::wrap_member_function() noexcept {
	using FuncPtr = decltype(func_ptr);
	static_assert(std::is_member_function_pointer_v<FuncPtr>);
	using Traits = FuncTraits<FuncPtr>;
	using Obj = typename Traits::Object;
	using Return = typename Traits::Return;
	using ArgList = typename Traits::ArgList;
	using ObjPtr = std::conditional_t<Traits::is_const, const void*, void*>;
	static_assert(std::is_void_v<Return> || !std::is_const_v<Return> && !std::is_volatile_v<Return>);
	constexpr auto wrapped_function = [](ObjPtr obj, void* result_buffer, void* args_buffer) -> Destructor {
		if constexpr (!std::is_void_v<Return>) {
			Return rst = details::wrap_function_call<ArgList>::template run<Obj, func_ptr>(obj, args_buffer);
			auto transformed_rst = type_buffer_decay<Return>(std::forward<Return>(rst));
			using BReturn = decltype(transformed_rst);
			buffer_as<BReturn>(result_buffer) = std::forward<BReturn>(transformed_rst);
			return destructor<BReturn>();
		}
		else {
			details::wrap_function_call<ArgList>::template run<Obj, func_ptr>(obj, args_buffer);
			return destructor<void>();
		}
	};
	return wrapped_function;
}

template<typename Func>
constexpr auto Ubpa::UDRefl::wrap_member_function(Func&& func) noexcept {
	using Traits = details::WrapFuncTraits<std::decay_t<Func>>;
	using Return = typename Traits::Return;
	using Obj = typename Traits::Object;
	using ArgList = typename Traits::ArgList;
	using ObjPtr = std::conditional_t<Traits::is_const, const void*, void*>;
	static_assert(std::is_void_v<Return> || !std::is_const_v<Return> && !std::is_volatile_v<Return>);
	/*constexpr*/ auto wrapped_function =
		[f = std::forward<Func>(func)](ObjPtr obj, void* result_buffer, void* args_buffer) mutable -> Destructor {
			if constexpr (!std::is_void_v<Return>) {
				Return rst = details::wrap_function_call<ArgList>::template run<Obj>(obj, std::forward<Func>(f), args_buffer);
				auto transformed_rst = type_buffer_decay<Return>(std::forward<Return>(rst));
				using BReturn = decltype(transformed_rst);
				buffer_as<BReturn>(result_buffer) = std::forward<BReturn>(transformed_rst);
				return destructor<BReturn>();
			}
			else {
				details::wrap_function_call<ArgList>::template run<Obj>(obj, std::forward<Func>(f), args_buffer);
				return destructor<void>();
			}
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
	static_assert(std::is_void_v<Return> || !std::is_const_v<Return> && !std::is_volatile_v<Return>);
	constexpr auto wrapped_function = [](void* result_buffer, void* args_buffer) -> Destructor {
		if constexpr (!std::is_void_v<Return>) {
			Return rst = details::wrap_function_call<ArgList>::template run<func_ptr>(args_buffer);
			auto transformed_rst = type_buffer_decay<Return>(std::forward<Return>(rst));
			using BReturn = decltype(transformed_rst);
			buffer_as<BReturn>(result_buffer) = std::forward<BReturn>(transformed_rst);
			return destructor<BReturn>();
		}
		else {
			details::wrap_function_call<ArgList>::template run<func_ptr>(args_buffer);
			return destructor<void>();
		}
	};
	return wrapped_function;
}

template<typename Func>
constexpr auto Ubpa::UDRefl::wrap_static_function(Func&& func) noexcept {
	using Traits = FuncTraits<std::decay_t<Func>>;
	using Return = typename Traits::Return;
	using ArgList = typename Traits::ArgList;
	static_assert(std::is_void_v<Return> || !std::is_const_v<Return> && !std::is_volatile_v<Return>);
	/*constexpr*/ auto wrapped_function =
		[f = std::forward<Func>(func)](void* result_buffer, void* args_buffer) mutable -> Destructor {
			if constexpr (!std::is_void_v<Return>) {
				Return rst = details::wrap_function_call<ArgList>::template run(std::forward<Func>(f), args_buffer);
				auto transformed_rst = type_buffer_decay<Return>(std::forward<Return>(rst));
				using BReturn = decltype(transformed_rst);
				buffer_as<BReturn>(result_buffer) = std::forward<BReturn>(transformed_rst);
				return destructor<BReturn>();
			}
			else {
				details::wrap_function_call<ArgList>::template run(std::forward<Func>(f), args_buffer);
				return destructor<void>();
			}
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
		static_assert(false);
}
