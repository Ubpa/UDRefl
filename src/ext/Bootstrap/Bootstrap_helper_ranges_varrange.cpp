#include "Bootstrap_helper.hpp"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_ranges_varrange() {
	Mngr.RegisterType<VarRange>();
	Mngr.AddConstructor<VarRange, ObjectView, FieldFlag>();
	Mngr.AddConstructor<VarRange, ObjectView>();
	Mngr.AddConstructor<VarRange, Type>();
	Mngr.AddConstructor<VarRange::iterator, ObjectTree::iterator>();
	Mngr.AddConstructor<VarRange::iterator, ObjectTree::iterator, CVRefMode>();
	Mngr.AddConstructor<VarRange::iterator, ObjectTree::iterator, CVRefMode, FieldFlag>();
	Mngr.AddMethod<&VarRange::iterator::Valid>("Valid");
	Mngr.AddMethod<&VarRange::iterator::GetDeriveds>("GetDeriveds");
	Mngr.AddMethod<&VarRange::iterator::GetObjectView>("GetObjectView");
	Mngr.AddMethod<&VarRange::iterator::GetTypeInfo>("GetTypeInfo");
	Mngr.AddMethod<&VarRange::iterator::GetFieldInfo>("GetFieldInfo");
}
