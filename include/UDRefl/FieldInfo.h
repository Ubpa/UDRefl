#pragma once

#include "FieldPtr.h"
#include "SharedBlock.h"
#include "NameID.h"

#include <unordered_map>

namespace Ubpa::UDRefl {
	struct FieldInfo {
		FieldPtr fieldptr;
		std::unordered_map<TypeID, SharedBlock> attrs;
	};

	struct Field {
		NameID ID;
		const FieldInfo& info;
	};
}
