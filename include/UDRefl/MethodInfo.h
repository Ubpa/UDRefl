#pragma once

#include "MethodPtr.h"
#include "SharedBlock.h"

namespace Ubpa::UDRefl {
	struct MethodInfo {
		MethodPtr methodptr;
		std::unordered_map<TypeID, SharedBlock> attrs;
	};

	struct Method {
		StrID ID;
		MethodInfo info;
	};
}
