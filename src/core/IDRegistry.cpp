#include <UDRefl/IDRegistry.h>

using namespace Ubpa;
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

	// container

	RegisterUnmanaged(Meta::container_assign);

	RegisterUnmanaged(Meta::container_begin);
	RegisterUnmanaged(Meta::container_cbegin);
	RegisterUnmanaged(Meta::container_end);
	RegisterUnmanaged(Meta::container_cend);
	RegisterUnmanaged(Meta::container_rbegin);
	RegisterUnmanaged(Meta::container_crbegin);
	RegisterUnmanaged(Meta::container_rend);
	RegisterUnmanaged(Meta::container_crend);

	RegisterUnmanaged(Meta::container_at);
	RegisterUnmanaged(Meta::container_data);
	RegisterUnmanaged(Meta::container_front);
	RegisterUnmanaged(Meta::container_back);

	RegisterUnmanaged(Meta::container_empty);
	RegisterUnmanaged(Meta::container_size);
	RegisterUnmanaged(Meta::container_max_size);
	RegisterUnmanaged(Meta::container_resize);
	RegisterUnmanaged(Meta::container_capacity);
	RegisterUnmanaged(Meta::container_bucket_count);
	RegisterUnmanaged(Meta::container_reserve);
	RegisterUnmanaged(Meta::container_shrink_to_fit);

	RegisterUnmanaged(Meta::container_clear);
	RegisterUnmanaged(Meta::container_insert);
	RegisterUnmanaged(Meta::container_insert_or_assign);
	RegisterUnmanaged(Meta::container_erase);
	RegisterUnmanaged(Meta::container_push_front);
	RegisterUnmanaged(Meta::container_pop_front);
	RegisterUnmanaged(Meta::container_push_back);
	RegisterUnmanaged(Meta::container_pop_back);
	RegisterUnmanaged(Meta::container_swap);
	RegisterUnmanaged(Meta::container_merge);
	RegisterUnmanaged(Meta::container_extract);

	RegisterUnmanaged(Meta::container_count);
	RegisterUnmanaged(Meta::container_find);
	RegisterUnmanaged(Meta::container_lower_bound);
	RegisterUnmanaged(Meta::container_upper_bound);
	RegisterUnmanaged(Meta::container_equal_range);

	RegisterUnmanaged(Meta::container_key_comp);
	RegisterUnmanaged(Meta::container_value_comp);
	RegisterUnmanaged(Meta::container_hash_function);
	RegisterUnmanaged(Meta::container_key_eq);
	RegisterUnmanaged(Meta::container_get_allocator);
}

TypeID TypeIDRegistry::RegisterAddConstLValueReference(TypeID ID) {
	std::string_view name = Nameof(ID);
	if (name.empty())
		return {};

	auto clref_name = type_name_add_const_lvalue_reference(name, get_allocator());

	if (clref_name.data() == name.data())
		return ID;

	return RegisterUnmanaged(clref_name);
}
