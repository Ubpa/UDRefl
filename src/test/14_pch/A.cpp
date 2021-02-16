#include "A.h"

#include <UDRefl/UDRefl.h>

void RegisterA() {
	Ubpa::UDRefl::Mngr->RegisterType<A>();
	Ubpa::UDRefl::Mngr->AddField<&A::data>("data");
}
