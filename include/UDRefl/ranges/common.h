#pragma once

#include "../Object.h"

#include <small_vector.h>

namespace Ubpa::UDRefl::Ranges {
	struct Derived {
		ObjectView obj;
		TypeInfo* typeinfo; // not nullptr
		std::unordered_map<Type, BaseInfo>::iterator curbase;

		friend bool operator==(const Derived& lhs, const Derived& rhs) {
			return lhs.obj.GetType() == rhs.obj.GetType()
				&& lhs.obj.GetPtr() == rhs.obj.GetPtr()
				&& lhs.curbase == rhs.curbase;
		}
	};
}
