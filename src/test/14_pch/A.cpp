#include "A.h"

#include <UDRefl_pch.h>

void RegisterA() {
	Ubpa::UDRefl::Mngr->RegisterTypeAuto<A>();
	Ubpa::UDRefl::Mngr->AddField<&A::data>("data");
}
