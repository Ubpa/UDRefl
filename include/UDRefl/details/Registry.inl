#pragma once

#include <cassert>

namespace Ubpa::UDRefl {
	inline size_t Registry::GetID(std::string_view name) {
		const size_t ID = DirectGetID(name);
		Register(ID, name);
		return ID;
	}

	inline void Registry::Register(size_t ID, std::string_view name) {
		auto target = id2name.find(ID);
		if (target == id2name.end())
			id2name.emplace_hint(target, std::pair{ ID, std::string{name} });
		else
			assert(target->second == name);
	}

	inline std::string_view Registry::Nameof(size_t ID) const noexcept {
		auto target = id2name.find(ID);
		if (target != id2name.end())
			return target->second;
		else
			return {};
	}
}
