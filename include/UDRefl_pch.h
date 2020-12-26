#pragma once

#include <UDRefl/UDRefl.h>

namespace Ubpa::UDRefl {
	inline void RegisterBasic() {
		ReflMngr::Instance().RegisterTypeAuto<bool>();
		ReflMngr::Instance().RegisterTypeAuto<std::uint8_t>();
		ReflMngr::Instance().RegisterTypeAuto<std::uint16_t>();
		ReflMngr::Instance().RegisterTypeAuto<std::uint32_t>();
		ReflMngr::Instance().RegisterTypeAuto<std::uint64_t>();
		ReflMngr::Instance().RegisterTypeAuto<std::int8_t>();
		ReflMngr::Instance().RegisterTypeAuto<std::int16_t>();
		ReflMngr::Instance().RegisterTypeAuto<std::int32_t>();
		ReflMngr::Instance().RegisterTypeAuto<std::int64_t>();
		ReflMngr::Instance().RegisterTypeAuto<float>();
		ReflMngr::Instance().RegisterTypeAuto<double>();
	}
}
