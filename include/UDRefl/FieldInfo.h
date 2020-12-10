#pragma once

#include "FieldPtr.h"
#include "SharedBlock.h"

#include <unordered_map>

namespace Ubpa::UDRefl {
	struct FieldInfo {
		FieldPtr fieldptr;
		std::unordered_map<TypeID, SharedBlock> attrs;
	};

	struct Field {
		StrID ID;
		const FieldInfo& info;
	};
}
