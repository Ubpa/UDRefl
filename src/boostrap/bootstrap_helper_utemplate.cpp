#include "bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::bootstrap_helper_utemplate() {
	// UTemplate
	Mngr.RegisterType<NameID>();
	Mngr.AddConstructor<NameID, std::size_t>();
	Mngr.AddConstructor<NameID, std::string_view>();
	Mngr.AddMethod<&NameID::GetValue>("GetValue");
	Mngr.AddMethod<&NameID::Valid>("Valid");
	Mngr.AddMethod<&NameID::Is>("Is");

	Mngr.RegisterType<TypeID>();
	Mngr.AddConstructor<TypeID, std::size_t>();
	Mngr.AddConstructor<TypeID, std::string_view>();
	Mngr.AddMethod<&TypeID::GetValue>("GetValue");
	Mngr.AddMethod<&TypeID::Valid>("Valid");
	Mngr.AddMethod<MemFuncOf<TypeID, bool(std::string_view)const noexcept>::get(&TypeID::Is)>("Is");

	Mngr.RegisterType<Name>();
	Mngr.AddConstructor<Name, std::string_view>();
	Mngr.AddConstructor<Name, std::string_view, NameID>();
	Mngr.AddMethod<&Name::GetView>("GetView");
	Mngr.AddMethod<&Name::GetID>("GetID");
	Mngr.AddMethod<&Name::Valid>("Valid");
	Mngr.AddMethod<&Name::Is>("Is");

	Mngr.RegisterType<Type>();
	Mngr.AddConstructor<Type, std::string_view>();
	Mngr.AddConstructor<Type, std::string_view, TypeID>();
	Mngr.AddMethod<&Type::GetName>("GetName");
	Mngr.AddMethod<&Type::GetID>("GetID");
	Mngr.AddMethod<&Type::Valid>("Valid");
	Mngr.AddMethod<MemFuncOf<Type, bool(std::string_view)const noexcept>::get(&Type::Is)>("Is");
}
