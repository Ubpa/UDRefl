#include "ReflMngrInitUtil.hpp"

using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::details::ReflMngrInitUtil_7(ReflMngr& mngr) {
	mngr.RegisterType<std::pmr::string>();
	mngr.AddConstructor<std::pmr::string, const std::string_view&>();
	mngr.AddConstructor<std::pmr::string, const std::string&>();
	mngr.AddConstructor<std::pmr::string, const char* const&>();
	mngr.AddConstructor<std::pmr::string, const char* const&, const std::pmr::string::size_type&>();
	mngr.AddConstructor<std::pmr::string, const std::pmr::string::size_type&, const std::pmr::string::value_type&>();
}
