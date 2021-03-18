#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_basic() {
	Mngr.AddField(GlobalType, "MaxArgNum", FieldInfo{ Mngr.GenerateFieldPtr<&MaxArgNum>() });

	Mngr.RegisterType<MethodFlag>();
	Mngr.AddField<MethodFlag::Variable>("Variable");
	Mngr.AddField<MethodFlag::Const>("Const");
	Mngr.AddField<MethodFlag::Static>("Static");
	Mngr.AddField<MethodFlag::None>("None");
	Mngr.AddField<MethodFlag::Member>("Member");
	Mngr.AddField<MethodFlag::Priority>("Priority");
	Mngr.AddField<MethodFlag::All>("All");

	Mngr.RegisterType<FieldFlag>();
	Mngr.AddField<FieldFlag::Basic>("Basic");
	Mngr.AddField<FieldFlag::Virtual>("Virtual");
	Mngr.AddField<FieldFlag::Static>("Static");
	Mngr.AddField<FieldFlag::DynamicShared>("DynamicShared");
	Mngr.AddField<FieldFlag::DynamicBuffer>("DynamicBuffer");
	Mngr.AddField<FieldFlag::None>("None");
	Mngr.AddField<FieldFlag::Owned>("Owned");
	Mngr.AddField<FieldFlag::Unowned>("Unowned");
	Mngr.AddField<FieldFlag::All>("All");
}
