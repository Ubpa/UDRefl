#include "A.h"

#include <UDRefl/ReflMngr.h>

void RegisterA() {
	Ubpa::UDRefl::Mngr->RegisterTypeAuto<A>();
	Ubpa::UDRefl::Mngr->AddField<&A::data>("data");
}
