#pragma once

#include "NameRegistry.h"
#include "TypeInfo.h"
#include "EnumInfo.h"

namespace Ubpa::UDRefl {
	class ReflMngr {
	public:
		static ReflMngr& Instance() noexcept {
			static ReflMngr instance;
			return instance;
		}

		NameRegistry registry;
		std::unordered_map<size_t, TypeInfo> typeinfos;
		std::unordered_map<size_t, EnumInfo> enuminfos;

	private:
		ReflMngr() = default;
	};
}
