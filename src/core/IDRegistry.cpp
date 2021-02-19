#include <UDRefl/IDRegistry.h>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

NameIDRegistry::NameIDRegistry() {
	RegisterUnmanaged(Meta::ctor.GetID(), Meta::ctor.GetView());
	RegisterUnmanaged(Meta::dtor.GetID(), Meta::dtor.GetView());

	RegisterUnmanaged(Meta::operator_bool.GetID(), Meta::operator_bool.GetView());

	RegisterUnmanaged(Meta::operator_plus.GetID(), Meta::operator_plus.GetView());
	RegisterUnmanaged(Meta::operator_minus.GetID(), Meta::operator_minus.GetView());

	RegisterUnmanaged(Meta::operator_add.GetID(), Meta::operator_add.GetView());
	RegisterUnmanaged(Meta::operator_sub.GetID(), Meta::operator_sub.GetView());
	RegisterUnmanaged(Meta::operator_mul.GetID(), Meta::operator_mul.GetView());
	RegisterUnmanaged(Meta::operator_div.GetID(), Meta::operator_div.GetView());
	RegisterUnmanaged(Meta::operator_mod.GetID(), Meta::operator_mod.GetView());

	RegisterUnmanaged(Meta::operator_bnot.GetID(), Meta::operator_bnot.GetView());
	RegisterUnmanaged(Meta::operator_band.GetID(), Meta::operator_band.GetView());
	RegisterUnmanaged(Meta::operator_bor.GetID(), Meta::operator_bor.GetView());
	RegisterUnmanaged(Meta::operator_bxor.GetID(), Meta::operator_bxor.GetView());
	RegisterUnmanaged(Meta::operator_lshift.GetID(), Meta::operator_lshift.GetView());
	RegisterUnmanaged(Meta::operator_rshift.GetID(), Meta::operator_rshift.GetView());

	RegisterUnmanaged(Meta::operator_pre_inc.GetID(), Meta::operator_pre_inc.GetView());
	RegisterUnmanaged(Meta::operator_pre_dec.GetID(), Meta::operator_pre_dec.GetView());
	RegisterUnmanaged(Meta::operator_post_inc.GetID(), Meta::operator_post_inc.GetView());
	RegisterUnmanaged(Meta::operator_post_dec.GetID(), Meta::operator_post_dec.GetView());

	RegisterUnmanaged(Meta::operator_assign.GetID(), Meta::operator_assign.GetView());
	RegisterUnmanaged(Meta::operator_assign_add.GetID(), Meta::operator_assign_add.GetView());
	RegisterUnmanaged(Meta::operator_assign_sub.GetID(), Meta::operator_assign_sub.GetView());
	RegisterUnmanaged(Meta::operator_assign_mul.GetID(), Meta::operator_assign_mul.GetView());
	RegisterUnmanaged(Meta::operator_assign_div.GetID(), Meta::operator_assign_div.GetView());
	RegisterUnmanaged(Meta::operator_assign_mod.GetID(), Meta::operator_assign_mod.GetView());
	RegisterUnmanaged(Meta::operator_assign_band.GetID(), Meta::operator_assign_band.GetView());
	RegisterUnmanaged(Meta::operator_assign_bor.GetID(), Meta::operator_assign_bor.GetView());
	RegisterUnmanaged(Meta::operator_assign_bxor.GetID(), Meta::operator_assign_bxor.GetView());
	RegisterUnmanaged(Meta::operator_assign_lshift.GetID(), Meta::operator_assign_lshift.GetView());
	RegisterUnmanaged(Meta::operator_assign_rshift.GetID(), Meta::operator_assign_rshift.GetView());

	RegisterUnmanaged(Meta::operator_eq.GetID(), Meta::operator_eq.GetView());
	RegisterUnmanaged(Meta::operator_ne.GetID(), Meta::operator_ne.GetView());
	RegisterUnmanaged(Meta::operator_lt.GetID(), Meta::operator_lt.GetView());
	RegisterUnmanaged(Meta::operator_le.GetID(), Meta::operator_le.GetView());
	RegisterUnmanaged(Meta::operator_gt.GetID(), Meta::operator_gt.GetView());
	RegisterUnmanaged(Meta::operator_ge.GetID(), Meta::operator_ge.GetView());

	RegisterUnmanaged(Meta::operator_and.GetID(), Meta::operator_and.GetView());
	RegisterUnmanaged(Meta::operator_or.GetID(), Meta::operator_or.GetView());
	RegisterUnmanaged(Meta::operator_not.GetID(), Meta::operator_not.GetView());

	RegisterUnmanaged(Meta::operator_subscript.GetID(), Meta::operator_subscript.GetView());
	RegisterUnmanaged(Meta::operator_deref.GetID(), Meta::operator_deref.GetView());
	RegisterUnmanaged(Meta::operator_ref.GetID(), Meta::operator_ref.GetView());
	RegisterUnmanaged(Meta::operator_member.GetID(), Meta::operator_member.GetView());
	RegisterUnmanaged(Meta::operator_member_of_pointer.GetID(), Meta::operator_member_of_pointer.GetView());

	RegisterUnmanaged(Meta::operator_call.GetID(), Meta::operator_call.GetView());
	RegisterUnmanaged(Meta::operator_comma.GetID(), Meta::operator_comma.GetView());

	// tuple

	RegisterUnmanaged(Meta::tuple_size.GetID(), Meta::tuple_size.GetView());
	RegisterUnmanaged(Meta::tuple_get.GetID(), Meta::tuple_get.GetView());
	RegisterUnmanaged(Meta::tuple_element.GetID(), Meta::tuple_element.GetView());

	// variant

	RegisterUnmanaged(Meta::variant_index.GetID(), Meta::variant_index.GetView());
	RegisterUnmanaged(Meta::variant_valueless_by_exception.GetID(), Meta::variant_valueless_by_exception.GetView());
	RegisterUnmanaged(Meta::variant_holds_alternative.GetID(), Meta::variant_holds_alternative.GetView());
	RegisterUnmanaged(Meta::variant_get.GetID(), Meta::variant_get.GetView());
	RegisterUnmanaged(Meta::variant_get_if.GetID(), Meta::variant_get_if.GetView());
	RegisterUnmanaged(Meta::variant_size.GetID(), Meta::variant_size.GetView());
	RegisterUnmanaged(Meta::variant_alternative.GetID(), Meta::variant_alternative.GetView());
	RegisterUnmanaged(Meta::variant_visit_get.GetID(), Meta::variant_visit_get.GetView());

	// optional

	RegisterUnmanaged(Meta::optional_has_value.GetID(), Meta::optional_has_value.GetView());
	RegisterUnmanaged(Meta::optional_value.GetID(), Meta::optional_value.GetView());
	RegisterUnmanaged(Meta::optional_reset.GetID(), Meta::optional_reset.GetView());

	// iterator

	RegisterUnmanaged(Meta::iterator_advance.GetID(), Meta::iterator_advance.GetView());
	RegisterUnmanaged(Meta::iterator_distance.GetID(), Meta::iterator_distance.GetView());
	RegisterUnmanaged(Meta::iterator_next.GetID(), Meta::iterator_next.GetView());
	RegisterUnmanaged(Meta::iterator_prev.GetID(), Meta::iterator_prev.GetView());

	// container

	RegisterUnmanaged(Meta::container_assign.GetID(), Meta::container_assign.GetView());

	RegisterUnmanaged(Meta::container_begin.GetID(), Meta::container_begin.GetView());
	RegisterUnmanaged(Meta::container_cbegin.GetID(), Meta::container_cbegin.GetView());
	RegisterUnmanaged(Meta::container_end.GetID(), Meta::container_end.GetView());
	RegisterUnmanaged(Meta::container_cend.GetID(), Meta::container_cend.GetView());
	RegisterUnmanaged(Meta::container_rbegin.GetID(), Meta::container_rbegin.GetView());
	RegisterUnmanaged(Meta::container_crbegin.GetID(), Meta::container_crbegin.GetView());
	RegisterUnmanaged(Meta::container_rend.GetID(), Meta::container_rend.GetView());
	RegisterUnmanaged(Meta::container_crend.GetID(), Meta::container_crend.GetView());

	RegisterUnmanaged(Meta::container_at.GetID(), Meta::container_at.GetView());
	RegisterUnmanaged(Meta::container_data.GetID(), Meta::container_data.GetView());
	RegisterUnmanaged(Meta::container_front.GetID(), Meta::container_front.GetView());
	RegisterUnmanaged(Meta::container_back.GetID(), Meta::container_back.GetView());
	RegisterUnmanaged(Meta::container_top.GetID(), Meta::container_top.GetView());

	RegisterUnmanaged(Meta::container_empty.GetID(), Meta::container_empty.GetView());
	RegisterUnmanaged(Meta::container_size.GetID(), Meta::container_size.GetView());
	RegisterUnmanaged(Meta::container_size_bytes.GetID(), Meta::container_size_bytes.GetView());
	RegisterUnmanaged(Meta::container_max_size.GetID(), Meta::container_max_size.GetView());
	RegisterUnmanaged(Meta::container_resize.GetID(), Meta::container_resize.GetView());
	RegisterUnmanaged(Meta::container_capacity.GetID(), Meta::container_capacity.GetView());
	RegisterUnmanaged(Meta::container_bucket_count.GetID(), Meta::container_bucket_count.GetView());
	RegisterUnmanaged(Meta::container_reserve.GetID(), Meta::container_reserve.GetView());
	RegisterUnmanaged(Meta::container_shrink_to_fit.GetID(), Meta::container_shrink_to_fit.GetView());

	RegisterUnmanaged(Meta::container_clear.GetID(), Meta::container_clear.GetView());
	RegisterUnmanaged(Meta::container_insert.GetID(), Meta::container_insert.GetView());
	RegisterUnmanaged(Meta::container_insert_after.GetID(), Meta::container_insert_after.GetView());
	RegisterUnmanaged(Meta::container_insert_or_assign.GetID(), Meta::container_insert_or_assign.GetView());
	RegisterUnmanaged(Meta::container_erase.GetID(), Meta::container_erase.GetView());
	RegisterUnmanaged(Meta::container_erase_after.GetID(), Meta::container_erase_after.GetView());
	RegisterUnmanaged(Meta::container_push_front.GetID(), Meta::container_push_front.GetView());
	RegisterUnmanaged(Meta::container_pop_front.GetID(), Meta::container_pop_front.GetView());
	RegisterUnmanaged(Meta::container_push_back.GetID(), Meta::container_push_back.GetView());
	RegisterUnmanaged(Meta::container_pop_back.GetID(), Meta::container_pop_back.GetView());
	RegisterUnmanaged(Meta::container_push.GetID(), Meta::container_push.GetView());
	RegisterUnmanaged(Meta::container_pop.GetID(), Meta::container_pop.GetView());
	RegisterUnmanaged(Meta::container_swap.GetID(), Meta::container_swap.GetView());
	RegisterUnmanaged(Meta::container_merge.GetID(), Meta::container_merge.GetView());
	RegisterUnmanaged(Meta::container_extract.GetID(), Meta::container_extract.GetView());

	RegisterUnmanaged(Meta::container_splice_after.GetID(), Meta::container_splice_after.GetView());
	RegisterUnmanaged(Meta::container_splice.GetID(), Meta::container_splice.GetView());
	RegisterUnmanaged(Meta::container_remove.GetID(), Meta::container_remove.GetView());
	RegisterUnmanaged(Meta::container_reverse.GetID(), Meta::container_reverse.GetView());
	RegisterUnmanaged(Meta::container_unique.GetID(), Meta::container_unique.GetView());
	RegisterUnmanaged(Meta::container_sort.GetID(), Meta::container_sort.GetView());

	RegisterUnmanaged(Meta::container_count.GetID(), Meta::container_count.GetView());
	RegisterUnmanaged(Meta::container_find.GetID(), Meta::container_find.GetView());
	RegisterUnmanaged(Meta::container_lower_bound.GetID(), Meta::container_lower_bound.GetView());
	RegisterUnmanaged(Meta::container_upper_bound.GetID(), Meta::container_upper_bound.GetView());
	RegisterUnmanaged(Meta::container_equal_range.GetID(), Meta::container_equal_range.GetView());

	RegisterUnmanaged(Meta::container_key_comp.GetID(), Meta::container_key_comp.GetView());
	RegisterUnmanaged(Meta::container_value_comp.GetID(), Meta::container_value_comp.GetView());
	RegisterUnmanaged(Meta::container_hash_function.GetID(), Meta::container_hash_function.GetView());
	RegisterUnmanaged(Meta::container_key_eq.GetID(), Meta::container_key_eq.GetView());
	RegisterUnmanaged(Meta::container_get_allocator.GetID(), Meta::container_get_allocator.GetView());
}

TypeIDRegistry::TypeIDRegistry() {
	RegisterUnmanaged(Meta::global.GetID(), Meta::global.GetName());
	RegisterUnmanaged(Meta::t_void.GetID(), Meta::t_void.GetName());
}

//
// Type Computation
/////////////////////

Type TypeIDRegistry::RegisterAddConst(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_const_hash(name) };
	if (auto ref_name = Nameof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	auto rst_name = type_name_add_const(name, get_allocator());

	RegisterUnmanaged(ref_ID, rst_name);
	return { rst_name, ref_ID };
}

Type TypeIDRegistry::RegisterAddLValueReference(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_lvalue_reference_hash(name) };
	if (auto ref_name = Nameof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	auto rst_name = type_name_add_lvalue_reference(name, get_allocator());

	RegisterUnmanaged(ref_ID, rst_name);
	return { rst_name, ref_ID };
}

Type TypeIDRegistry::RegisterAddLValueReferenceWeak(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_lvalue_reference_weak_hash(name) };
	if (auto ref_name = Nameof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	auto rst_name = type_name_add_lvalue_reference_weak(name, get_allocator());

	RegisterUnmanaged(ref_ID, rst_name);

	return { rst_name, ref_ID };
}

Type TypeIDRegistry::RegisterAddRValueReference(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_rvalue_reference_hash(name) };
	if (auto ref_name = Nameof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	auto rst_name = type_name_add_rvalue_reference(name, get_allocator());

	RegisterUnmanaged(ref_ID, rst_name);

	return { rst_name, ref_ID };
}

Type TypeIDRegistry::RegisterAddConstLValueReference(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_const_lvalue_reference_hash(name) };
	if (auto ref_name = Nameof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	auto rst_name = type_name_add_const_lvalue_reference(name, get_allocator());

	RegisterUnmanaged(ref_ID, rst_name);

	return { rst_name, ref_ID };
}

Type TypeIDRegistry::RegisterAddConstRValueReference(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_const_rvalue_reference_hash(name) };
	if (auto ref_name = Nameof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	auto rst_name = type_name_add_const_rvalue_reference(name, get_allocator());

	RegisterUnmanaged(ref_ID, rst_name);
	return { rst_name, ref_ID };
}
