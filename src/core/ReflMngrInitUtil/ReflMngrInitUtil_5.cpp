#include "ReflMngrInitUtil.h"

using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::ReflMngrInitUtil_5(ReflMngr& mngr) {
	mngr.RegisterType<std::string_view>();
	mngr.AddConstructor<std::string_view, const char* const&>();
	mngr.AddConstructor<std::string_view, const char* const&, const std::string_view::size_type&>();
	mngr.AddConstructor<std::string_view, const std::string&>();
}
