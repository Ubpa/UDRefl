#pragma once

#include "../FuncTransformer.h"

namespace Ubpa::UDRefl {
	template<typename T>
	Func::Func(T&& func) :
		func{ FuncTransformer<std::decay_t<T>>::template GetFunc(std::forward<T>(func)) },
		signature{ FuncTransformer<std::decay_t<T>>::GetFuncSig() }
	{}

	template<typename Ret, typename... Args>
	Ret Func::Call(Args... args) {
		assert(signature.Is<Args...>());
		if constexpr (std::is_void_v<Ret>)
			func({ std::forward<Args>(args)... });
		else
			return func({ std::forward<Args>(args)... }).Cast<Ret>();
	}
}
