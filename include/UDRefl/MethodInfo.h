#pragma once

#include "Method.h"
#include "Attr.h"

namespace Ubpa::UDRefl {
	struct MethodInfo {
		Method method;
		std::unordered_map<TypeID, Attr> attrs;
	};
}
