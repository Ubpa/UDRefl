#include <UDRefl/IDRegistry.h>

using namespace Ubpa::UDRefl;

StrIDRegistry::StrIDRegistry() {
	//
	// Global
	///////////

	Register(Meta::malloc);
	Register(Meta::free);
	Register(Meta::aligned_malloc);
	Register(Meta::aligned_free);

	//
	// Member
	///////////

	Register(Meta::ctor);
	Register(Meta::dtor);

	Register(Meta::operator_add);
	Register(Meta::operator_minus);
	Register(Meta::operator_mul);
	Register(Meta::operator_div);
	Register(Meta::operator_mod);

	Register(Meta::operator_eq);
	Register(Meta::operator_ne);
	Register(Meta::operator_lt);
	Register(Meta::operator_le);
	Register(Meta::operator_gt);
	Register(Meta::operator_ge);

	Register(Meta::operator_and);
	Register(Meta::operator_or);
	Register(Meta::operator_not);

	Register(Meta::operator_pos);
	Register(Meta::operator_neg);
	Register(Meta::operator_deref);
	Register(Meta::operator_ref);

	Register(Meta::operator_inc);
	Register(Meta::operator_dec);

	Register(Meta::operator_band);
	Register(Meta::operator_bor);
	Register(Meta::operator_bnot);
	Register(Meta::operator_bxor);
	Register(Meta::operator_lshift);
	Register(Meta::operator_rshift);

	Register(Meta::operator_assign);
	Register(Meta::operator_assign_add);
	Register(Meta::operator_assign_minus);
	Register(Meta::operator_assign_mul);
	Register(Meta::operator_assign_div);
	Register(Meta::operator_assign_mod);
	Register(Meta::operator_assign_band);
	Register(Meta::operator_assign_bor);
	Register(Meta::operator_assign_bxor);
	Register(Meta::operator_assign_lshift);
	Register(Meta::operator_assign_rshift);

	Register(Meta::operator_new);
	Register(Meta::operator_new_array);
	Register(Meta::operator_delete);
	Register(Meta::operator_delete_array);

	Register(Meta::operator_member);
	Register(Meta::operator_call);
	Register(Meta::operator_comma);
	Register(Meta::operator_subscript);
}
