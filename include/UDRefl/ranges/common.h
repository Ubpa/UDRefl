#pragma once

#include "../Basic.h"

#include <small_vector.h>

namespace Ubpa::UDRefl::Ranges {
	struct Derived {
		Type type;
		TypeInfo* typeinfo; // not nullptr
		std::unordered_map<Type, BaseInfo>::iterator curbase;

		void* ptr{ nullptr }; // for VarRange

		friend bool operator==(const Derived& lhs, const Derived& rhs) {
			return lhs.type == rhs.type && lhs.curbase == rhs.curbase;
		}
	};
}
