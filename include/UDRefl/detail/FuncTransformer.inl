#pragma once

namespace Ubpa::UDRefl::detail {
	template<typename Ret, typename... Args, typename Func, size_t... Ns>
	static auto GetFunc(Func&& func, std::index_sequence<Ns...>) {
		return [&](ArgList args) -> AnyWrapper {
			using ArgTuple = std::tuple<Args...>;
			if constexpr (std::is_void_v<Ret>) {
				std::forward<Func>(func)(args.GetArg<std::tuple_element_t<Ns, ArgTuple>>(Ns)...);
				return {};
			}
			else
				return std::forward<Func>(func)(args.GetArg<std::tuple_element_t<Ns, ArgTuple>>(Ns)...);
		};
	}
}

namespace Ubpa::UDRefl {
	template<typename Ret, typename... Args>
	template<typename Func>
	auto FuncTransformer<Ret(Args...)>::GetFunc(Func&& func) {
		return detail::GetFunc<Ret,Args...>(std::forward<Func>(func), std::make_index_sequence<sizeof...(Args)>{});
	}

	template<typename Ret, typename... Args>
	struct FuncTransformer<Ret(*)(Args...)> : FuncTransformer<Ret(Args...)> {};
	template<typename Ret, typename... Args>
	struct FuncTransformer<Ret(Args...)const> : FuncTransformer<Ret(Args...)> {};
	template<typename Obj, typename Func>
	struct FuncTransformer<Func Obj::*> : FuncTransformer<Func> {};
	template<typename T>
	struct FuncTransformer : FuncTransformer<decltype(&std::decay_t<T>::operator())> {};
}
