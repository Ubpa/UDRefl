#include "Vector.h"

#include <UDRefl/UDRefl.h>

void RegisterVector() {
	Ubpa::UDRefl::Mngr->RegisterTypeAuto<Vector>();
	Ubpa::UDRefl::Mngr->AddField<&Vector::data>("data");
}
