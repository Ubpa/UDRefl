#include <UDRefl_bootstrap.h>

#include "bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::bootstrap() {
	details::bootstrap_helper_utemplate();
	details::bootstrap_helper_basic();
	details::bootstrap_helper_registry();
	details::bootstrap_helper_container_0();
	details::bootstrap_helper_container_1();
	details::bootstrap_helper_container_2();
	details::bootstrap_helper_container_3();
	details::bootstrap_helper_container_4();
	details::bootstrap_helper_container_5();
	details::bootstrap_helper_container_6();
	details::bootstrap_helper_info_0();
	details::bootstrap_helper_info_1();
	details::bootstrap_helper_info_2();
	details::bootstrap_helper_info_3();
	details::bootstrap_helper_info_4();
	details::bootstrap_helper_object();
	details::bootstrap_helper_ptr();
	details::bootstrap_helper_reflmngr_0();
	details::bootstrap_helper_reflmngr_1();
}
