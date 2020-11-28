#pragma once

#include "FieldPtr.h"

#include <unordered_map>
#include <any>

namespace Ubpa::UDRefl {
	struct FieldInfo {
		FieldPtr fieldptr;
		std::unordered_map<NameID, std::any> attrs;
	};
}
