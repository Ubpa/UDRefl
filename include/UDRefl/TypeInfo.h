#pragma once

#include "FieldInfo.h"
#include "MethodInfo.h"
#include "BaseInfo.h"

namespace Ubpa::UDRefl {
	struct TypeInfo {
		size_t size;
		size_t alignment;
		std::unordered_map<Name, FieldInfo> fieldinfos;
		std::unordered_multimap<Name, MethodInfo> methodinfos;
		std::unordered_map<Type, BaseInfo> baseinfos;
		AttrSet attrs;
	};
}
