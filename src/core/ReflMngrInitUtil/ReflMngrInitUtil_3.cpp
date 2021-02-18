#include "ReflMngrInitUtil.h"

using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::ReflMngrInitUtil_3(ReflMngr& mngr) {
	mngr.RegisterType<bool>();
	mngr.RegisterType<float>();
	mngr.RegisterType<double>();
}
