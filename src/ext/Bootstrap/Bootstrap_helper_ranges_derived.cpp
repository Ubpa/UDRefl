#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_ranges_derived() {
	Mngr.RegisterType<Ranges::Derived>();
	Mngr.AddField<&Ranges::Derived::obj>("obj");
	Mngr.AddField<&Ranges::Derived::typeinfo>("typeinfo");
	Mngr.AddField<&Ranges::Derived::curbase>("curbase");
}
