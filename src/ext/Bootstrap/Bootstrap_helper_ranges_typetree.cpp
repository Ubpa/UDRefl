#include "Bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::details::Bootstrap_helper_ranges_typetree() {
	Mngr.RegisterType<TypeTree>();
	Mngr.AddConstructor<TypeTree, Type>();
	Mngr.AddMethod<&MethodRange::GetType>("GetType");
	Mngr.AddConstructor<TypeTree::iterator, Type, bool>();
	Mngr.AddMethod<&TypeTree::iterator::Valid>("Valid");
	Mngr.AddMethod<&TypeTree::iterator::GetDeriveds>("GetDeriveds");
}
