#pragma once

#include "TypeID.h"

namespace Ubpa::UDRefl {
	class TypeIDRegistry {
	public:
		struct Meta {
			static constexpr char global[] = "UDRefl::global";
		};

		TypeIDRegistry() { registry.Register(TypeID{ Meta::global }.GetValue(), Meta::global); }

		TypeID GetID(std::string_view name) { return TypeID{ registry.GetID(name) }; }
		std::string_view Nameof(TypeID ID) const noexcept { return registry.Nameof(ID.GetValue()); }

		template<typename T>
		TypeID GetID() {
			constexpr auto name = type_name<T>().name;
			constexpr TypeID ID{ name };
			registry.Register(ID.GetValue(), name);
			return ID;
		}

	private:
		IDRegistry registry;
	};
}
