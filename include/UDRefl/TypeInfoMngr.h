#pragma once

#include "Export.h"
#include "TypeInfo.h"
#include <unordered_map>

namespace Ubpa::UDRefl {
	class UDREFL_DESC TypeInfoMngr {
	public:
		static TypeInfoMngr& Instance() {
			static TypeInfoMngr instance;
			return instance;
		}

		TypeInfo* GetTypeInfo(size_t id);

	private:
		std::unordered_map<size_t, TypeInfo*> id2typeinfo;

		TypeInfoMngr() = default;
		~TypeInfoMngr();
	};
}
