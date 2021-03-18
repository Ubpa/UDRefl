#include "ReflMngrInitUtil.hpp"

using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::ReflMngrInitUtil_2(ReflMngr& mngr) {
	mngr.RegisterType<std::uint8_t>();
	mngr.RegisterType<std::uint16_t>();
	mngr.RegisterType<std::uint32_t>();
	mngr.RegisterType<std::uint64_t>();
}
