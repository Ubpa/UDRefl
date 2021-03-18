#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_reflmngr_1() {
	Mngr.AddField<&ReflMngr::typeinfos>("typeinfos");
}
