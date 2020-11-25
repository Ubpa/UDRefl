#pragma once

#include "FieldInfo.h"
#include "MethodInfo.h"

namespace Ubpa::UDRefl {
	struct TypeInfo {
		std::unordered_map<size_t, std::any> attrs;
		std::unordered_map<size_t, FieldInfo> fieldinfos;
		std::unordered_map<size_t, ConstFieldInfo> cfieldinfos;
		std::unordered_map<size_t, StaticFieldInfo> sfieldinfos;
		std::unordered_map<size_t, StaticConstFieldInfo> scfieldinfos;
		std::unordered_multimap<size_t, MethodInfo> methodinfos;
		std::unordered_multimap<size_t, ConstMethodInfo> cmethodinfos;
	};
}
