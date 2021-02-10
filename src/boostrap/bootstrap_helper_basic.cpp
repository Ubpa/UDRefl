#include "bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::bootstrap_helper_basic() {
	Mngr.AddField(GlobalType, "MaxArgNum", FieldInfo{ Mngr.GenerateFieldPtr<&MaxArgNum>() });

	Mngr.RegisterType<MethodFlag>();
	Mngr.AddField<MethodFlag::Variable>("Variable");
	Mngr.AddField<MethodFlag::Const>("Const");
	Mngr.AddField<MethodFlag::Static>("Static");
	Mngr.AddField<MethodFlag::None>("None");
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

	Mngr.RegisterType<ResultDesc>();
	Mngr.AddField<&ResultDesc::type>("type");
	Mngr.AddField<&ResultDesc::size>("size");
	Mngr.AddField<&ResultDesc::alignment>("alignment");

	Mngr.RegisterType<InvokeResult>();
	Mngr.AddField<&InvokeResult::success>("success");
	Mngr.AddField<&InvokeResult::type>("type");
	Mngr.AddField<&InvokeResult::destructor>("destructor");

	Mngr.RegisterType<InvocableResult>();
	Mngr.AddField<&InvocableResult::success>("success");
	Mngr.AddField<&InvocableResult::result_desc>("result_desc");

	Mngr.RegisterType<InfoTypePair>();
	Mngr.AddField<&InfoTypePair::type>("type");
	Mngr.AddField<&InfoTypePair::info>("info");

	Mngr.RegisterType<InfoFieldPair>();
	Mngr.AddField<&InfoFieldPair::name>("name");
	Mngr.AddField<&InfoFieldPair::info>("info");

	Mngr.RegisterType<InfoMethodPair>();
	Mngr.AddField<&InfoMethodPair::name>("name");
	Mngr.AddField<&InfoMethodPair::info>("info");

	Mngr.RegisterType<InfoTypeFieldPair>();
	Mngr.AddField<&InfoTypeFieldPair::type>("type");
	Mngr.AddField<&InfoTypeFieldPair::field>("field");

	Mngr.RegisterType<InfoTypeMethodPair>();
	Mngr.AddField<&InfoTypeMethodPair::type>("type");
	Mngr.AddField<&InfoTypeMethodPair::method>("method");
}
