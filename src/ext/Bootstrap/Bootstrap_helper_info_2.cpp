#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_info_2() {
	Mngr.AddField<&TypeInfo::fieldinfos>("fieldinfos");
}
