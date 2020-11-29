#include <UDRefl/Registry.h>

#include <cassert>

using namespace Ubpa::UDRefl;

size_t Registry::Register(std::string_view name) {
	assert(!name.empty());

	auto target = name2id.find(name);
	if (target != name2id.end())
		return target->second;

	size_t idx = names.size();
	names.push_back(std::string{ name });
	name2id.emplace_hint(target, std::pair{ std::string_view{names[idx]}, idx });

	return idx;
}

size_t Registry::GetID(std::string_view name) const noexcept {
	auto target = name2id.find(name);
	return target == name2id.end() ? static_cast<size_t>(-1) : target->second;
}

std::string_view Registry::Nameof(size_t ID) const noexcept
{
	if (IsRegistered(ID))
		return names[ID];
	else
		return {};
}

NameRegistry::NameRegistry() {
	//
	// Global
	///////////

	Register(Meta::malloc);
	Register(Meta::free);

	//
	// Common
	///////////

	Register(Meta::size);
	Register(Meta::alignment);
	Register(Meta::ptr);

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

TypeRegistry::TypeRegistry() {
	Register(Meta::global);

	Register(Meta::t_float);
	Register(Meta::t_double);

	Register(Meta::t_int8_t);
	Register(Meta::t_int16_t);
	Register(Meta::t_int32_t);
	Register(Meta::t_int64_t);

	Register(Meta::t_uint8_t);
	Register(Meta::t_uint16_t);
	Register(Meta::t_uint32_t);
	Register(Meta::t_uint64_t);

	Register(Meta::t_void_ptr);
	Register(Meta::t_const_void_ptr);
}
