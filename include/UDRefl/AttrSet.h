#pragma once

#include "Object.h"

#include <set>

namespace Ubpa::UDRefl {
	using Attr = SharedObject;

	struct AttrLess {
		using is_transparent = int;

		bool operator()(const Attr&   lhs, const Attr&   rhs) const noexcept { return lhs.GetTypeID() < rhs.GetTypeID(); }
		bool operator()(const Attr&   lhs, const TypeID& rhs) const noexcept { return lhs.GetTypeID() < rhs;             }
		bool operator()(const TypeID& lhs, const Attr&   rhs) const noexcept { return lhs             < rhs.GetTypeID(); }
	};

	using AttrSet = std::set<Attr, AttrLess>;
}
