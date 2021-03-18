#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_ranges_span_derived() {
	Mngr.RegisterType<std::span<const Ranges::Derived>>();
}
