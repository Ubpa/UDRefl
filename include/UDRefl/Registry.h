#pragma once

#include "ID.h"

#include <string>
#include <deque>
#include <unordered_map>

namespace Ubpa::UDRefl {
	class Registry {
	public:
		size_t Register(std::string_view name);
		size_t GetID(std::string_view name) const noexcept;
		bool IsRegistered(size_t ID) const noexcept { return ID < names.size(); }
		std::string_view Nameof(size_t ID) const noexcept;

	private:
		std::deque<std::string> names;
		std::unordered_map<std::string_view, size_t> name2id;
	};

	class NameRegistry {
	public:
		NameID Register(std::string_view name) { return NameID{ registry.Register(name) }; }
		NameID GetID(std::string_view name) const noexcept { return NameID{ registry.GetID(name) }; }
		bool IsRegistered(NameID ID) const noexcept { return registry.IsRegistered(ID.GetValue()); }
		std::string_view Nameof(NameID ID) const noexcept { return registry.Nameof(ID.GetValue()); }

	private:
		Registry registry;
	};

	class TypeRegistry {
	public:
		TypeID Register(std::string_view name) { return TypeID{ registry.Register(name) }; }
		TypeID GetID(std::string_view name) const noexcept { return TypeID{ registry.GetID(name) }; }
		bool IsRegistered(TypeID ID) const noexcept { return registry.IsRegistered(ID.GetValue()); }
		std::string_view Nameof(TypeID ID) const noexcept { return registry.Nameof(ID.GetValue()); }

	private:
		Registry registry;
	};
}
