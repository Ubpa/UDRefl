#pragma once

#include "Attr.h"

#include <map>
#include <string>
#include <cassert>

namespace Ubpa::UDRefl {
	struct AttrList {
		std::map<std::string, Attr, std::less<>> data;

		bool Contains(std::string_view name) const {
			return data.find(name) != data.end();
		}

		template<typename T>
		T& Get(std::string_view name) {
			assert(Contains(name));
			return data.find(name)->second.Cast<T>();
		}
	};
}
