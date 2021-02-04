#include <UDRefl/IDRegistry.h>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

StrIDRegistry::StrIDRegistry() {
	//
	// Global
	///////////

	RegisterUnmanaged(MetaID::malloc, Meta::malloc);
	RegisterUnmanaged(MetaID::free, Meta::free);
	RegisterUnmanaged(MetaID::aligned_malloc, Meta::aligned_malloc);
	RegisterUnmanaged(MetaID::aligned_free, Meta::aligned_free);

	//
	// Member
	///////////

	RegisterUnmanaged(MetaID::ctor, Meta::ctor);
	RegisterUnmanaged(MetaID::dtor, Meta::dtor);

	RegisterUnmanaged(MetaID::operator_bool, Meta::operator_bool);

	RegisterUnmanaged(MetaID::operator_plus, Meta::operator_plus);
	RegisterUnmanaged(MetaID::operator_minus, Meta::operator_minus);

	RegisterUnmanaged(MetaID::operator_add, Meta::operator_add);
	RegisterUnmanaged(MetaID::operator_sub, Meta::operator_sub);
	RegisterUnmanaged(MetaID::operator_mul, Meta::operator_mul);
	RegisterUnmanaged(MetaID::operator_div, Meta::operator_div);
	RegisterUnmanaged(MetaID::operator_mod, Meta::operator_mod);

	RegisterUnmanaged(MetaID::operator_bnot, Meta::operator_bnot);
	RegisterUnmanaged(MetaID::operator_band, Meta::operator_band);
	RegisterUnmanaged(MetaID::operator_bor, Meta::operator_bor);
	RegisterUnmanaged(MetaID::operator_bxor, Meta::operator_bxor);
	RegisterUnmanaged(MetaID::operator_lshift, Meta::operator_lshift);
	RegisterUnmanaged(MetaID::operator_rshift, Meta::operator_rshift);

	RegisterUnmanaged(MetaID::operator_pre_inc, Meta::operator_pre_inc);
	RegisterUnmanaged(MetaID::operator_pre_dec, Meta::operator_pre_dec);
	RegisterUnmanaged(MetaID::operator_post_inc, Meta::operator_post_inc);
	RegisterUnmanaged(MetaID::operator_post_dec, Meta::operator_post_dec);

	RegisterUnmanaged(MetaID::operator_assign, Meta::operator_assign);
	RegisterUnmanaged(MetaID::operator_assign_add, Meta::operator_assign_add);
	RegisterUnmanaged(MetaID::operator_assign_sub, Meta::operator_assign_sub);
	RegisterUnmanaged(MetaID::operator_assign_mul, Meta::operator_assign_mul);
	RegisterUnmanaged(MetaID::operator_assign_div, Meta::operator_assign_div);
	RegisterUnmanaged(MetaID::operator_assign_mod, Meta::operator_assign_mod);
	RegisterUnmanaged(MetaID::operator_assign_band, Meta::operator_assign_band);
	RegisterUnmanaged(MetaID::operator_assign_bor, Meta::operator_assign_bor);
	RegisterUnmanaged(MetaID::operator_assign_bxor, Meta::operator_assign_bxor);
	RegisterUnmanaged(MetaID::operator_assign_lshift, Meta::operator_assign_lshift);
	RegisterUnmanaged(MetaID::operator_assign_rshift, Meta::operator_assign_rshift);

	RegisterUnmanaged(MetaID::operator_eq, Meta::operator_eq);
	RegisterUnmanaged(MetaID::operator_ne, Meta::operator_ne);
	RegisterUnmanaged(MetaID::operator_lt, Meta::operator_lt);
	RegisterUnmanaged(MetaID::operator_le, Meta::operator_le);
	RegisterUnmanaged(MetaID::operator_gt, Meta::operator_gt);
	RegisterUnmanaged(MetaID::operator_ge, Meta::operator_ge);

	RegisterUnmanaged(MetaID::operator_and, Meta::operator_and);
	RegisterUnmanaged(MetaID::operator_or, Meta::operator_or);
	RegisterUnmanaged(MetaID::operator_not, Meta::operator_not);

	RegisterUnmanaged(MetaID::operator_subscript, Meta::operator_subscript);
	RegisterUnmanaged(MetaID::operator_deref, Meta::operator_deref);
	RegisterUnmanaged(MetaID::operator_ref, Meta::operator_ref);
	RegisterUnmanaged(MetaID::operator_member, Meta::operator_member);
	RegisterUnmanaged(MetaID::operator_member_of_pointer, Meta::operator_member_of_pointer);

	RegisterUnmanaged(MetaID::operator_call, Meta::operator_call);
	RegisterUnmanaged(MetaID::operator_comma, Meta::operator_comma);

	// tuple

	RegisterUnmanaged(MetaID::tuple_size, Meta::tuple_size);
	RegisterUnmanaged(MetaID::tuple_get, Meta::tuple_get);

	// container

	RegisterUnmanaged(MetaID::container_assign, Meta::container_assign);

	RegisterUnmanaged(MetaID::container_begin, Meta::container_begin);
	RegisterUnmanaged(MetaID::container_cbegin, Meta::container_cbegin);
	RegisterUnmanaged(MetaID::container_end, Meta::container_end);
	RegisterUnmanaged(MetaID::container_cend, Meta::container_cend);
	RegisterUnmanaged(MetaID::container_rbegin, Meta::container_rbegin);
	RegisterUnmanaged(MetaID::container_crbegin, Meta::container_crbegin);
	RegisterUnmanaged(MetaID::container_rend, Meta::container_rend);
	RegisterUnmanaged(MetaID::container_crend, Meta::container_crend);

	RegisterUnmanaged(MetaID::container_at, Meta::container_at);
	RegisterUnmanaged(MetaID::container_data, Meta::container_data);
	RegisterUnmanaged(MetaID::container_front, Meta::container_front);
	RegisterUnmanaged(MetaID::container_back, Meta::container_back);

	RegisterUnmanaged(MetaID::container_empty, Meta::container_empty);
	RegisterUnmanaged(MetaID::container_size, Meta::container_size);
	RegisterUnmanaged(MetaID::container_max_size, Meta::container_max_size);
	RegisterUnmanaged(MetaID::container_resize, Meta::container_resize);
	RegisterUnmanaged(MetaID::container_capacity, Meta::container_capacity);
	RegisterUnmanaged(MetaID::container_bucket_count, Meta::container_bucket_count);
	RegisterUnmanaged(MetaID::container_reserve, Meta::container_reserve);
	RegisterUnmanaged(MetaID::container_shrink_to_fit, Meta::container_shrink_to_fit);

	RegisterUnmanaged(MetaID::container_clear, Meta::container_clear);
	RegisterUnmanaged(MetaID::container_insert, Meta::container_insert);
	RegisterUnmanaged(MetaID::container_insert_or_assign, Meta::container_insert_or_assign);
	RegisterUnmanaged(MetaID::container_erase, Meta::container_erase);
	RegisterUnmanaged(MetaID::container_push_front, Meta::container_push_front);
	RegisterUnmanaged(MetaID::container_pop_front, Meta::container_pop_front);
	RegisterUnmanaged(MetaID::container_push_back, Meta::container_push_back);
	RegisterUnmanaged(MetaID::container_pop_back, Meta::container_pop_back);
	RegisterUnmanaged(MetaID::container_swap, Meta::container_swap);
	RegisterUnmanaged(MetaID::container_merge, Meta::container_merge);
	RegisterUnmanaged(MetaID::container_extract, Meta::container_extract);

	RegisterUnmanaged(MetaID::container_count, Meta::container_count);
	RegisterUnmanaged(MetaID::container_find, Meta::container_find);
	RegisterUnmanaged(MetaID::container_lower_bound, Meta::container_lower_bound);
	RegisterUnmanaged(MetaID::container_upper_bound, Meta::container_upper_bound);
	RegisterUnmanaged(MetaID::container_equal_range, Meta::container_equal_range);

	RegisterUnmanaged(MetaID::container_key_comp, Meta::container_key_comp);
	RegisterUnmanaged(MetaID::container_value_comp, Meta::container_value_comp);
	RegisterUnmanaged(MetaID::container_hash_function, Meta::container_hash_function);
	RegisterUnmanaged(MetaID::container_key_eq, Meta::container_key_eq);
	RegisterUnmanaged(MetaID::container_get_allocator, Meta::container_get_allocator);
}

TypeIDRegistry::TypeIDRegistry() {
	RegisterUnmanaged(Meta::global);
	RegisterUnmanaged(Meta::t_void);
}

void TypeIDRegistry::RegisterUnmanaged(TypeID ID, std::string_view name) {
	assert(!type_name_is_const(name) && !type_name_is_volatile(name));
	IDRegistry<TypeID>::RegisterUnmanaged(ID, name);
}

TypeID TypeIDRegistry::RegisterUnmanaged(std::string_view name) {
	assert(!type_name_is_const(name) && !type_name_is_volatile(name));
	return IDRegistry<TypeID>::RegisterUnmanaged(name);
}

void TypeIDRegistry::Register(TypeID ID, std::string_view name) {
	if (type_name_is_const(name) || type_name_is_volatile(name)) {
		assert(false);
		return;
	}

	IDRegistry<TypeID>::Register(ID, name);
}

TypeID TypeIDRegistry::Register(std::string_view name) {
	if (type_name_is_const(name) || type_name_is_volatile(name)) {
		assert(false);
		return {};
	}

	return IDRegistry<TypeID>::Register(name);
}

//
// Type Computation
/////////////////////

TypeID TypeIDRegistry::RegisterAddLValueReference(TypeID ID) {
	std::string_view name = Nameof(ID);
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_lvalue_reference_hash(name) };
	if (IsRegistered(ref_ID))
		return ref_ID;

	auto rst_name = type_name_add_lvalue_reference(name, get_allocator());

	return RegisterUnmanaged(rst_name);
}

TypeID TypeIDRegistry::RegisterAddConstLValueReference(TypeID ID) {
	std::string_view name = Nameof(ID);
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_const_lvalue_reference_hash(name) };
	if (IsRegistered(ref_ID))
		return ref_ID;

	auto rst_name = type_name_add_const_lvalue_reference(name, get_allocator());

	return RegisterUnmanaged(rst_name);
}

TypeID TypeIDRegistry::RegisterAddRValueReference(TypeID ID) {
	std::string_view name = Nameof(ID);
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_rvalue_reference_hash(name) };
	if (IsRegistered(ref_ID))
		return ref_ID;

	auto rst_name = type_name_add_rvalue_reference(name, get_allocator());

	return RegisterUnmanaged(rst_name);
}

TypeID TypeIDRegistry::RegisterAddConstRValueReference(TypeID ID) {
	std::string_view name = Nameof(ID);
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_const_rvalue_reference_hash(name) };
	if (IsRegistered(ref_ID))
		return ref_ID;

	auto rst_name = type_name_add_const_rvalue_reference(name, get_allocator());

	return RegisterUnmanaged(rst_name);
}
