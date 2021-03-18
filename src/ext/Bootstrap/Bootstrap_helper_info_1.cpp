#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_info_1() {
	Mngr.RegisterType<AttrSet>();

	Mngr.AddField<&FieldInfo::attrs>("attrs");

	Mngr.AddField<&MethodInfo::attrs>("attrs");

	Mngr.AddField<&TypeInfo::attrs>("attrs");
}
