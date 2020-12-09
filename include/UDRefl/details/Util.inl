#pragma once

namespace Ubpa::UDRefl::details {
	template<typename ArgList>
	struct wrap_function_call;
	template<typename OrigArgList, typename BufferArgList>
	struct wrap_function_call_impl;

	template<typename... Args>
	struct wrap_function_call<TypeList<Args...>>
		: wrap_function_call_impl<TypeList<Args...>, TypeList<decltype(remove_lref<Args>(std::declval<Args>()))...>> {};

	template<typename... OrigArgs, typename... BufferArgs>
	struct wrap_function_call_impl<TypeList<OrigArgs...>, TypeList<BufferArgs...>> {
		template<typename Obj, auto func_ptr, typename ObjPtr>
		static constexpr decltype(auto) run(ObjPtr ptr, void* args_buffer) {
			return std::apply(
				[ptr](auto&&... bufferArgs) -> decltype(auto) {
					return (buffer_as<Obj>(ptr).*func_ptr)(add_lref<OrigArgs...>(std::forward<decltype(bufferArgs)>(bufferArgs))...);
				},
				std::move(*reinterpret_cast<std::tuple<BufferArgs...>*>(args_buffer))
			);
		}
		template<auto func_ptr>
		static constexpr decltype(auto) run(void* args_buffer) {
			return std::apply(
				[](auto&&... bufferArgs) -> decltype(auto) {
					return func_ptr(add_lref<OrigArgs...>(std::forward<decltype(bufferArgs)>(bufferArgs))...);
				},
				std::move(*reinterpret_cast<std::tuple<BufferArgs...>*>(args_buffer))
			);
		}
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
	constexpr auto wrapped_function = [](ObjPtr obj, void* args_buffer, void* result_buffer) {
		if constexpr (!std::is_void_v<Return>) {
			Return rst = details::wrap_function_call<ArgList>::template run<Obj, func_ptr>(obj, args_buffer);
			auto transformed_rst = remove_lref<Return>(std::forward<Return>(rst));
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

template<auto func_ptr>
constexpr auto Ubpa::UDRefl::wrap_non_member_function() noexcept {
	using FuncPtr = decltype(func_ptr);
	static_assert(is_function_pointer_v<FuncPtr>);
	using Traits = FuncTraits<FuncPtr>;
	using Return = typename Traits::Return;
	using ArgList = typename Traits::ArgList;
	constexpr auto wrapped_function = [](void* args_buffer, void* result_buffer) {
		if constexpr (!std::is_void_v<Return>) {
			Return rst = details::wrap_function_call<ArgList>::template run<func_ptr>(args_buffer);
			auto transformed_rst = remove_lref<Return>(std::forward<Return>(rst));
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

template<auto func_ptr>
constexpr auto Ubpa::UDRefl::wrap_function() noexcept {
	using FuncPtr = decltype(func_ptr);
	if constexpr (is_function_pointer_v<FuncPtr>)
		return wrap_non_member_function<func_ptr>();
	else if constexpr (std::is_member_function_pointer_v<FuncPtr>)
		return wrap_member_function<func_ptr>();
	else
		static_assert(false);
}
