#include "Bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_ranges_derived() {
	Mngr.RegisterType<Ranges::Derived>();
	Mngr.AddField<&Ranges::Derived::obj>("obj");
	Mngr.AddField<&Ranges::Derived::typeinfo>("typeinfo");
	Mngr.AddField<&Ranges::Derived::curbase>("curbase");
	Mngr.RegisterType<std::span<const Ranges::Derived>>();
}
