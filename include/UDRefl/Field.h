#pragma once

#include "Var.h"
#include "StaticVar.h"
#include "Func.h"
#include "VariantWrapper.h"
#include "AttrList.h"

namespace Ubpa::UDRefl {
	struct Field {
		VariantWrapper<Var, StaticVar, Func> value;
		AttrList attrs;

		bool operator<(const Field& rhs) const {
			if (!value.TypeIs<Func>() || !rhs.value.TypeIs<Func>())
				return false;
			return value.Cast<Func>().signature < rhs.value.Cast<Func>().signature;
		}
	};
}
