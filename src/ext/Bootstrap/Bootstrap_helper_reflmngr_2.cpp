#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_reflmngr_2() {
	Mngr.RegisterType<std::span<const Type>>();
}
