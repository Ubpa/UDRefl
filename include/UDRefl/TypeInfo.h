#pragma once

#include "FieldInfo.h"

namespace Ubpa::UDRefl {
	struct TypeInfo {
		std::unordered_map<size_t, std::any> attrs;
		std::unordered_map<size_t, FieldInfo> fieldinfos;
		std::unordered_map<size_t, StaticFieldInfo> staticfieldinfos;
	};
}
