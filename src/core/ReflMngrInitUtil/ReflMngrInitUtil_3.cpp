#include "ReflMngrInitUtil.hpp"

using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::ReflMngrInitUtil_3(ReflMngr& mngr) {
	mngr.RegisterType<bool>();
	mngr.RegisterType<std::nullptr_t>();
	mngr.RegisterType<void*>();
	mngr.RegisterType<float>();
	mngr.RegisterType<double>();
}
