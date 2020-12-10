#include <UDRefl/NameIDRegistry.h>

#include <cassert>

using namespace Ubpa::UDRefl;

NameIDRegistry::NameIDRegistry() {
	//
	// Global
	///////////

	registry.Register(string_hash(Meta::malloc), Meta::malloc);
	registry.Register(string_hash(Meta::free), Meta::free);

	//
	// Member
	///////////

	registry.Register(string_hash(Meta::ctor), Meta::ctor);
	registry.Register(string_hash(Meta::dtor), Meta::dtor);

	registry.Register(string_hash(Meta::operator_add), Meta::operator_add);
	registry.Register(string_hash(Meta::operator_minus), Meta::operator_minus);
	registry.Register(string_hash(Meta::operator_mul), Meta::operator_mul);
	registry.Register(string_hash(Meta::operator_div), Meta::operator_div);
	registry.Register(string_hash(Meta::operator_mod), Meta::operator_mod);

	registry.Register(string_hash(Meta::operator_eq), Meta::operator_eq);
	registry.Register(string_hash(Meta::operator_ne), Meta::operator_ne);
	registry.Register(string_hash(Meta::operator_lt), Meta::operator_lt);
	registry.Register(string_hash(Meta::operator_le), Meta::operator_le);
	registry.Register(string_hash(Meta::operator_gt), Meta::operator_gt);
	registry.Register(string_hash(Meta::operator_ge), Meta::operator_ge);

	registry.Register(string_hash(Meta::operator_and), Meta::operator_and);
	registry.Register(string_hash(Meta::operator_or), Meta::operator_or);
	registry.Register(string_hash(Meta::operator_not), Meta::operator_not);

	registry.Register(string_hash(Meta::operator_pos), Meta::operator_pos);
	registry.Register(string_hash(Meta::operator_neg), Meta::operator_neg);
	registry.Register(string_hash(Meta::operator_deref), Meta::operator_deref);
	registry.Register(string_hash(Meta::operator_ref), Meta::operator_ref);

	registry.Register(string_hash(Meta::operator_inc), Meta::operator_inc);
	registry.Register(string_hash(Meta::operator_dec), Meta::operator_dec);

	registry.Register(string_hash(Meta::operator_band), Meta::operator_band);
	registry.Register(string_hash(Meta::operator_bor), Meta::operator_bor);
	registry.Register(string_hash(Meta::operator_bnot), Meta::operator_bnot);
	registry.Register(string_hash(Meta::operator_bxor), Meta::operator_bxor);
	registry.Register(string_hash(Meta::operator_lshift), Meta::operator_lshift);
	registry.Register(string_hash(Meta::operator_rshift), Meta::operator_rshift);

	registry.Register(string_hash(Meta::operator_assign), Meta::operator_assign);
	registry.Register(string_hash(Meta::operator_assign_add), Meta::operator_assign_add);
	registry.Register(string_hash(Meta::operator_assign_minus), Meta::operator_assign_minus);
	registry.Register(string_hash(Meta::operator_assign_mul), Meta::operator_assign_mul);
	registry.Register(string_hash(Meta::operator_assign_div), Meta::operator_assign_div);
	registry.Register(string_hash(Meta::operator_assign_mod), Meta::operator_assign_mod);
	registry.Register(string_hash(Meta::operator_assign_band), Meta::operator_assign_band);
	registry.Register(string_hash(Meta::operator_assign_bor), Meta::operator_assign_bor);
	registry.Register(string_hash(Meta::operator_assign_bxor), Meta::operator_assign_bxor);
	registry.Register(string_hash(Meta::operator_assign_lshift), Meta::operator_assign_lshift);
	registry.Register(string_hash(Meta::operator_assign_rshift), Meta::operator_assign_rshift);

	registry.Register(string_hash(Meta::operator_new), Meta::operator_new);
	registry.Register(string_hash(Meta::operator_new_array), Meta::operator_new_array);
	registry.Register(string_hash(Meta::operator_delete), Meta::operator_delete);
	registry.Register(string_hash(Meta::operator_delete_array), Meta::operator_delete_array);

	registry.Register(string_hash(Meta::operator_member), Meta::operator_member);
	registry.Register(string_hash(Meta::operator_call), Meta::operator_call);
	registry.Register(string_hash(Meta::operator_comma), Meta::operator_comma);
	registry.Register(string_hash(Meta::operator_subscript), Meta::operator_subscript);
}
