#pragma once

#include "MethodPtr.h"
#include "Attr.h"

namespace Ubpa::UDRefl {
	struct MethodInfo {
		MethodPtr methodptr;
		std::unordered_map<TypeID, Attr> attrs;
	};

	struct Method {
		NameID ID;
		MethodInfo info;
	};
}
