#pragma once

#include "MethodPtr.h"
#include "AttrSet.h"

namespace Ubpa::UDRefl {
	struct MethodInfo {
		MethodPtr methodptr;
		AttrSet attrs;
	};

	struct Method {
		StrID ID;
		MethodInfo info;
	};
}
