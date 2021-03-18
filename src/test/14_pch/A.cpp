#include "A.hpp"

#include <UDRefl/UDRefl.hpp>

void RegisterA() {
	Ubpa::UDRefl::Mngr.RegisterType<A>();
	Ubpa::UDRefl::Mngr.AddField<&A::data>("data");
}
