#pragma once

#include "FieldInfo.h"
#include "MethodInfo.h"
#include "BaseInfo.h"

namespace Ubpa::UDRefl {
	struct TypeInfo {
		//
		// Data
		/////////

		size_t size;
		size_t alignment;
		std::unordered_map<StrID, FieldInfo> fieldinfos;
		std::unordered_multimap<StrID, MethodInfo> methodinfos;
		std::unordered_map<TypeID, BaseInfo> baseinfos;
		AttrSet attrs;
	};
}
