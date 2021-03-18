#include <UDRefl/IDRegistry.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

#define UBPA_UDREFL_META_GET_ID_VIEW(meta) meta.GetID(), meta.GetView()
#define UBPA_UDREFL_META_GET_ID_NAME(meta) meta.GetID(), meta.GetName()

NameIDRegistry::NameIDRegistry() {
	// operators
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_bool));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_add));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_sub));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_mul));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_div));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_mod));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_bnot));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_band));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_bor));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_bxor));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_shl));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_shr));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_pre_inc));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_pre_dec));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_post_inc));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_post_dec));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_assignment));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_assignment_add));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_assignment_sub));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_assignment_mul));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_assignment_div));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_assignment_mod));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_assignment_band));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_assignment_bor));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_assignment_bxor));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_assignment_shl));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_assignment_shr));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_eq));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_ne));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_lt));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_le));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_gt));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_ge));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_and));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_or));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_not));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_subscript));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_indirection));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::operator_call));

	// non-member functions


	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::ctor));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::dtor));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::get));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::tuple_size));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::tuple_element));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::holds_alternative));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::get_if));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::variant_size));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::variant_alternative));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::variant_visit_get));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::advance));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::distance));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::next));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::prev));

	// member functions

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_assign));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_begin));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_cbegin));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_end));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_cend));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_rbegin));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_crbegin));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_rend));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_crend));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_at));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_data));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_front));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_back));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_top));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_empty));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_size));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_size_bytes));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_resize));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_capacity));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_bucket_count));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_reserve));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_shrink_to_fit));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_clear));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_insert));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_insert_after));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_insert_or_assign));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_erase));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_erase_after));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_push_front));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_pop_front));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_push_back));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_pop_back));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_push));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_pop));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_swap));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_merge));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_extract));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_splice_after));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_splice));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_remove));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_reverse));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_unique));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_sort));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_count));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_find));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_lower_bound));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_upper_bound));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::container_equal_range));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::variant_index));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::variant_valueless_by_exception));

	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::optional_has_value));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::optional_value));
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_VIEW(Meta::optional_reset));
}

Name NameIDRegistry::Nameof(NameID ID) const {
	auto view = Viewof(ID);
	if (view.empty())
		return {};
	return { view, ID };
}

TypeIDRegistry::TypeIDRegistry() {
	RegisterUnmanaged(UBPA_UDREFL_META_GET_ID_NAME(Meta::global));
}

Type TypeIDRegistry::Typeof(TypeID ID) const {
	auto view = Viewof(ID);
	if (view.empty())
		return {};
	return { view, ID };
}

//
// Type Computation
/////////////////////

Type TypeIDRegistry::RegisterAddConst(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_const_hash(name) };
	if (auto ref_name = Viewof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	std::string_view rst_name;
	{
		std::lock_guard wlock{ smutex }; // write resource
		rst_name = type_name_add_const(name, get_allocator());
	}

	RegisterUnmanaged(ref_ID, rst_name);
	return { rst_name, ref_ID };
}

Type TypeIDRegistry::RegisterAddLValueReference(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_lvalue_reference_hash(name) };
	if (auto ref_name = Viewof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	std::string_view rst_name;
	{
		std::lock_guard wlock{ smutex }; // write resource
		rst_name = type_name_add_lvalue_reference(name, get_allocator());
	}

	RegisterUnmanaged(ref_ID, rst_name);
	return { rst_name, ref_ID };
}

Type TypeIDRegistry::RegisterAddLValueReferenceWeak(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_lvalue_reference_weak_hash(name) };
	if (auto ref_name = Viewof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	std::string_view rst_name;
	{
		std::lock_guard wlock{ smutex }; // write resource
		rst_name = type_name_add_lvalue_reference_weak(name, get_allocator());
	}

	RegisterUnmanaged(ref_ID, rst_name);

	return { rst_name, ref_ID };
}

Type TypeIDRegistry::RegisterAddRValueReference(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_rvalue_reference_hash(name) };
	if (auto ref_name = Viewof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	std::string_view rst_name;
	{
		std::lock_guard wlock{ smutex }; // write resource
		rst_name = type_name_add_rvalue_reference(name, get_allocator());
	}

	RegisterUnmanaged(ref_ID, rst_name);

	return { rst_name, ref_ID };
}

Type TypeIDRegistry::RegisterAddConstLValueReference(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_const_lvalue_reference_hash(name) };
	if (auto ref_name = Viewof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	std::string_view rst_name;
	{
		std::lock_guard wlock{ smutex }; // write resource
		rst_name = type_name_add_const_lvalue_reference(name, get_allocator());
	}

	RegisterUnmanaged(ref_ID, rst_name);

	return { rst_name, ref_ID };
}

Type TypeIDRegistry::RegisterAddConstRValueReference(Type type) {
	std::string_view name = type.GetName();
	if (name.empty())
		return {};

	TypeID ref_ID{ type_name_add_const_rvalue_reference_hash(name) };
	if (auto ref_name = Viewof(ref_ID); !ref_name.empty())
		return { ref_name, ref_ID };

	std::string_view rst_name;
	{
		std::lock_guard wlock{ smutex }; // write resource
		rst_name = type_name_add_const_rvalue_reference(name, get_allocator());
	}

	RegisterUnmanaged(ref_ID, rst_name);
	return { rst_name, ref_ID };
}

#undef UBPA_UDREFL_META_GET_ID_VIEW
#undef UBPA_UDREFL_META_GET_ID_NAME
