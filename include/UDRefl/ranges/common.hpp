#pragma once

#include "../Object.hpp"

#include "../Info.hpp" 

#include <USmallFlat/small_vector.hpp>

namespace Ubpa::UDRefl::Ranges {
	struct UDRefl_core_API Derived {
		ObjectView obj;
		TypeInfo* typeinfo; // not nullptr
		std::unordered_map<Type, BaseInfo>::iterator curbase;

		UDRefl_core_API friend bool operator==(const Derived& lhs, const Derived& rhs) {
			return lhs.obj.GetType() == rhs.obj.GetType()
				&& lhs.obj.GetPtr() == rhs.obj.GetPtr()
				&& lhs.curbase == rhs.curbase;
		}
	};
}
