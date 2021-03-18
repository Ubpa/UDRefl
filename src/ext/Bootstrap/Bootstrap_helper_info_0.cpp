#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_info_0() {
	Mngr.RegisterType<BaseInfo>();
	Mngr.AddMethod<&BaseInfo::IsVirtual>("IsVirtual");
	Mngr.AddMethod<&BaseInfo::IsPolymorphic>("IsPolymorphic");
	Mngr.AddMethod<&BaseInfo::StaticCast_DerivedToBase>("StaticCast_DerivedToBase");
	Mngr.AddMethod<&BaseInfo::StaticCast_BaseToDerived>("StaticCast_BaseToDerived");
	Mngr.AddMethod<&BaseInfo::DynamicCast_BaseToDerived>("DynamicCast_BaseToDerived");

	Mngr.RegisterType<FieldInfo>();
	Mngr.AddField<&FieldInfo::fieldptr>("fieldptr");

	Mngr.RegisterType<MethodInfo>();
	Mngr.AddField<&MethodInfo::methodptr>("methodptr");

	Mngr.RegisterType<TypeInfo>();
	Mngr.AddField<&TypeInfo::size>("size");
	Mngr.AddField<&TypeInfo::alignment>("alignment");
	Mngr.AddField<&TypeInfo::size>("size");
}
