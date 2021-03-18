#include "Vector.hpp"

#include <UDRefl/UDRefl.hpp>

void RegisterVector() {
	Ubpa::UDRefl::Mngr.RegisterType<Vector>();
	Ubpa::UDRefl::Mngr.AddField<&Vector::data>("data");
}
