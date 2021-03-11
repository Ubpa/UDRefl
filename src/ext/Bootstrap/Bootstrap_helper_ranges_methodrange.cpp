#include "Bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_ranges_methodrange() {
	Mngr.RegisterType<MethodRange>();
	Mngr.AddConstructor<MethodRange, Type, MethodFlag>();
	Mngr.AddConstructor<MethodRange, Type>();
	Mngr.AddMethod<&MethodRange::GetType>("GetType");
	Mngr.AddConstructor<MethodRange::iterator, TypeTree::iterator>();
	Mngr.AddConstructor<MethodRange::iterator, TypeTree::iterator, MethodFlag>();
	Mngr.AddMethod<&MethodRange::iterator::Valid>("Valid");
	Mngr.AddMethod<&MethodRange::iterator::GetDeriveds>("GetDeriveds");
}
