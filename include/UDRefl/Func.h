#pragma once

#include "ArgList.h"

#include <functional>
#include <cassert>

namespace Ubpa::UDRefl {
	struct Func {
		std::function<AnyWrapper(ArgList)> func;
		FuncSig signature;

		template<typename T>
		Func(T&& func);

		template<typename... Args>
		bool SignatureIs() const noexcept {
			return signature.Is<Args...>();
		}

		AnyWrapper Call(ArgList arglist) {
			assert(signature == arglist.GetFuncSig());
			return func(std::move(arglist));
		}

		template<typename Ret, typename... Args>
		Ret Call(Args... args);
	};
}

#include "detail/Func.inl"
