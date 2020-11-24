#pragma once

#include "FieldPtr.h"

#include <unordered_map>
#include <any>

namespace Ubpa::UDRefl {
	struct FieldInfo {
		FieldPtr fieldptr;
		std::unordered_map<size_t, std::any> attrs;
	};

	struct ConstFieldInfo {
		ConstFieldPtr fieldptr;
		std::unordered_map<size_t, std::any> attrs;
	};

	struct StaticFieldInfo {
		ObjectPtr objptr;
		std::unordered_map<size_t, std::any> attrs;
	};

	struct StaticConstFieldInfo {
		ConstObjectPtr objptr;
		std::unordered_map<size_t, std::any> attrs;
	};
}
