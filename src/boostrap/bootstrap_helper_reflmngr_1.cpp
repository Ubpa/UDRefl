#include "bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::bootstrap_helper_reflmngr_1() {
	Mngr.AddField<&ReflMngr::typeinfos>("typeinfos");
}
