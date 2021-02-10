#include "bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::bootstrap_helper_container_1() {
	Mngr.RegisterType<std::vector<InfoTypeFieldPair>>();
}
