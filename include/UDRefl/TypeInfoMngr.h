#pragma once

#include "TypeInfo.h"

namespace Ubpa::UDRefl {
	class TypeInfoMngr {
	public:
		static TypeInfoMngr& Instance() noexcept {
			static TypeInfoMngr instance;
			return instance;
		}

		std::unordered_map<size_t, TypeInfo> typeinfos;

	private:
		TypeInfoMngr() = default;
	};
}
