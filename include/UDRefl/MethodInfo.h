#pragma once

#include "Method.h"

namespace Ubpa::UDRefl {
	struct MethodInfo {
		Method method;
		std::unordered_map<size_t, std::any> attrs;
	};
}
