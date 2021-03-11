#include <UDRefl_ext/Bootstrap.h>

#include "Bootstrap_helper.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void Ubpa::UDRefl::ext::Bootstrap() {
	details::Bootstrap_helper_utemplate();
	details::Bootstrap_helper_basic();
	details::Bootstrap_helper_registry();
	details::Bootstrap_helper_info_0();
	details::Bootstrap_helper_info_1();
	details::Bootstrap_helper_info_2();
	details::Bootstrap_helper_info_3();
	details::Bootstrap_helper_info_4();
	details::Bootstrap_helper_object();
	details::Bootstrap_helper_ptr();
	details::Bootstrap_helper_ranges_derived();
	details::Bootstrap_helper_ranges_objecttree();
	details::Bootstrap_helper_ranges_fieldrange();
	details::Bootstrap_helper_ranges_methodrange();
	details::Bootstrap_helper_ranges_varrange();
	details::Bootstrap_helper_reflmngr_0();
	details::Bootstrap_helper_reflmngr_1();
	details::Bootstrap_helper_reflmngr_2();
}
