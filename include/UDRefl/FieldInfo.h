#pragma once

#include "AttrSet.h"
#include "FieldPtr.h"

namespace Ubpa::UDRefl {
	struct FieldInfo {
		FieldPtr fieldptr;
		AttrSet attrs;
	};
}
