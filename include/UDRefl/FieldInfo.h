#pragma once

#include "FieldPtr.h"
#include "Attr.h"

#include <unordered_map>

namespace Ubpa::UDRefl {
	struct FieldInfo {
		FieldPtr fieldptr;
		std::unordered_map<TypeID, Attr> attrs;
	};

	struct Field {
		NameID ID;
		const FieldInfo& info;
	};
}
