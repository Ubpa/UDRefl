#include "B.hpp"

#include <UDRefl/UDRefl.hpp>

void RegisterB() {
	Ubpa::UDRefl::Mngr.RegisterType<B>();
	Ubpa::UDRefl::Mngr.AddField<&B::v0>("v0");
	Ubpa::UDRefl::Mngr.AddField<&B::v1>("v1");
	Ubpa::UDRefl::Mngr.AddField<&B::v2>("v2");
	Ubpa::UDRefl::Mngr.AddField<&B::v3>("v3");
	Ubpa::UDRefl::Mngr.AddField<&B::v4>("v4");
	Ubpa::UDRefl::Mngr.AddField<&B::v5>("v5");
}
