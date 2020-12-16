#include <UDRefl/IDRegistry.h>

using namespace Ubpa::UDRefl;

StrIDRegistry::StrIDRegistry() {
	//
	// Global
	///////////

	RegisterUnmanaged(Meta::malloc);
	RegisterUnmanaged(Meta::free);
	RegisterUnmanaged(Meta::aligned_malloc);
	RegisterUnmanaged(Meta::aligned_free);

	//
	// Member
	///////////

	RegisterUnmanaged(Meta::ctor);
	RegisterUnmanaged(Meta::dtor);

	RegisterUnmanaged(Meta::operator_plus);
	RegisterUnmanaged(Meta::operator_minus);

	RegisterUnmanaged(Meta::operator_add);
	RegisterUnmanaged(Meta::operator_sub);
	RegisterUnmanaged(Meta::operator_mul);
	RegisterUnmanaged(Meta::operator_div);
	RegisterUnmanaged(Meta::operator_mod);

	RegisterUnmanaged(Meta::operator_bnot);
	RegisterUnmanaged(Meta::operator_band);
	RegisterUnmanaged(Meta::operator_bor);
	RegisterUnmanaged(Meta::operator_bxor);
	RegisterUnmanaged(Meta::operator_lshift);
	RegisterUnmanaged(Meta::operator_rshift);

	RegisterUnmanaged(Meta::operator_pre_inc);
	RegisterUnmanaged(Meta::operator_pre_dec);
	RegisterUnmanaged(Meta::operator_post_inc);
	RegisterUnmanaged(Meta::operator_post_dec);

	RegisterUnmanaged(Meta::operator_assign);
	RegisterUnmanaged(Meta::operator_assign_add);
	RegisterUnmanaged(Meta::operator_assign_sub);
	RegisterUnmanaged(Meta::operator_assign_mul);
	RegisterUnmanaged(Meta::operator_assign_div);
	RegisterUnmanaged(Meta::operator_assign_mod);
	RegisterUnmanaged(Meta::operator_assign_band);
	RegisterUnmanaged(Meta::operator_assign_bor);
	RegisterUnmanaged(Meta::operator_assign_bxor);
	RegisterUnmanaged(Meta::operator_assign_lshift);
	RegisterUnmanaged(Meta::operator_assign_rshift);

	RegisterUnmanaged(Meta::operator_eq);
	RegisterUnmanaged(Meta::operator_ne);
	RegisterUnmanaged(Meta::operator_lt);
	RegisterUnmanaged(Meta::operator_le);
	RegisterUnmanaged(Meta::operator_gt);
	RegisterUnmanaged(Meta::operator_ge);

	RegisterUnmanaged(Meta::operator_and);
	RegisterUnmanaged(Meta::operator_or);
	RegisterUnmanaged(Meta::operator_not);

	RegisterUnmanaged(Meta::operator_subscript);
	RegisterUnmanaged(Meta::operator_deref);
	RegisterUnmanaged(Meta::operator_ref);
	RegisterUnmanaged(Meta::operator_member);
	RegisterUnmanaged(Meta::operator_member_of_pointer);

	RegisterUnmanaged(Meta::operator_call);
	RegisterUnmanaged(Meta::operator_comma);
}
