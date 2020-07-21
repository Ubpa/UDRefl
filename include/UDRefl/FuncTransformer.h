#pragma once

#include "FuncSig.h"

namespace Ubpa::UDRefl {
	// [Usage]
	// FuncTransformer<decltype(func)>::GetFunc(std::forward<decltype(func)>(func))
	// FuncTransformer<decltype(func)>::GetFuncSig()
	template<typename T>
	struct FuncTransformer;

	// kernel case
	template<typename Ret, typename... Args>
	struct FuncTransformer<Ret(Args...)> {
		template<typename Func>
		static auto GetFunc(Func&& func);

		static FuncSig GetFuncSig() {
			return { typeid(Args).hash_code()... };
		}
	};
}

#include "detail/FuncTransformer.inl"
