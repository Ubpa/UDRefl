#include "bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::bootstrap_helper_info_2() {
	Mngr.AddField<&TypeInfo::fieldinfos>("fieldinfos");
}
