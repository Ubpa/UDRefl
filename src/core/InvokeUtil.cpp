#include "InvokeUtil.hpp"

#include <UDRefl/UDRefl.hpp>

using namespace Ubpa::UDRefl;

bool details::IsPriorityCompatible(std::span<const Type> params, std::span<const Type> argTypes) {
	if (params.size() != argTypes.size())
		return false;

	for (size_t i = 0; i < params.size(); i++) {
		if (params[i] == argTypes[i])
			continue;

		const auto& lhs = params[i];
		const auto& rhs = argTypes[i];

		if (lhs.IsRValueReference()) { // &&{T} | &&{const{T}}
			const auto unref_lhs = lhs.Name_RemoveRValueReference(); // T | const{T}
			assert(!type_name_is_volatile(unref_lhs));
			if (!type_name_is_const(unref_lhs) && rhs.Is(unref_lhs))
				continue; // &&{T} <- T
		}
		else if (lhs.IsLValueReference()) { // &{T} | &{const{T}}
			const auto unref_lhs = lhs.Name_RemoveLValueReference(); // T | const{T}
			assert(!type_name_is_volatile(unref_lhs));
			if (type_name_is_const(unref_lhs) && rhs.Is(unref_lhs))
				continue; // &{const{T}} <- const{T}
		}
		else {
			if (lhs.Is(rhs.Name_RemoveRValueReference()))
				continue; // T <- &&{T}
		}

		return false;
	}

	return true;
}

bool details::IsRefCompatible(std::span<const Type> paramTypes, std::span<const Type> argTypes) {
	if (paramTypes.size() != argTypes.size())
		return false;

	for (size_t i = 0; i < paramTypes.size(); i++) {
		if (!is_ref_compatible(paramTypes[i], argTypes[i]))
			return false;
	}

	return true;
}

bool details::IsRefConstructible(Type paramType, std::span<const Type> argTypes) {
	auto target = Mngr.typeinfos.find(paramType);
	if (target == Mngr.typeinfos.end())
		return false;
	const auto& typeinfo = target->second;

	if (typeinfo.is_trivial && (
		argTypes.empty() // default ctor
		|| argTypes.size() == 1 && argTypes.front().RemoveCVRef() == paramType // const/ref ctor
	))
	{ return true; }

	auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::ctor);
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (IsRefCompatible(iter->second.methodptr.GetParamList(), argTypes))
			return true;
	}
	return false;
}

bool details::RefConstruct(ObjectView obj, ArgsView args) {
	auto target = Mngr.typeinfos.find(obj.GetType());
	if (target == Mngr.typeinfos.end())
		return false;

	const auto& typeinfo = target->second;

	if (typeinfo.is_trivial) {
		if (args.Types().empty())
			return true;
		if (args.Types().size() == 1 && args.Types().front().RemoveCVRef() == obj.GetType()) {
			std::memcpy(obj.GetPtr(), args.Buffer()[0], typeinfo.size);
			return true;
		}
	}

	auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::ctor);
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Variable
			&& IsRefCompatible(iter->second.methodptr.GetParamList(), args.Types()))
		{
			iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, { args.Buffer(),iter->second.methodptr.GetParamList() });
			return true;
		}
	}
	return false;
}

details::NewArgsGuard::NewArgsGuard(
	bool is_priority,
	std::pmr::memory_resource* rsrc,
	std::span<const Type> paramTypes,
	ArgsView args)
{
	auto argTypes = args.Types();
	auto orig_argptr_buffer = args.Buffer();

	if (argTypes.size() != paramTypes.size())
		return;

	if (is_priority) {
		is_compatible = IsPriorityCompatible(paramTypes, argTypes);
		new_args = { orig_argptr_buffer, paramTypes };
		return;
	}

	// 1. is compatible ? (collect infos)

	const std::uint8_t num_args = static_cast<std::uint8_t>(argTypes.size());

	ArgInfo info_copiedargs[MaxArgNum + 1];
	std::uint8_t num_copiedargs = 0;
	std::uint32_t size_copiedargs = 0;
	std::uint8_t num_copied_nonptr_args = 0;
	bool contains_objview = false;
	for (std::uint8_t i = 0; i < argTypes.size(); i++) {
		if (paramTypes[i] == argTypes[i])
			continue;

		if (paramTypes[i] == Type_of<ObjectView>) {
			contains_objview = true;
			continue;
		}

		const auto& lhs = paramTypes[i];
		const auto& rhs = argTypes[i];

		if (lhs.IsLValueReference()) { // &{T} | &{const{T}}
			const auto unref_lhs = lhs.Name_RemoveLValueReference(); // T | const{T}
			if (type_name_is_const(unref_lhs)) { // &{const{T}}
				if (unref_lhs == rhs.Name_RemoveRValueReference())
					continue; // &{const{T}} <- &&{const{T}} || const{T}

				const auto raw_lhs = type_name_remove_const(unref_lhs); // T
				if (rhs.Is(raw_lhs) || raw_lhs == rhs.Name_RemoveReference())
					continue; // &{const{T}} <- T | &{T} | &&{T}

				Type raw_lhs_type{ raw_lhs };
				if (IsRefConstructible(raw_lhs_type, std::span<const Type>{&rhs, 1}) && Mngr.IsDestructible(raw_lhs_type)) {
					auto& info = info_copiedargs[num_copiedargs++];
					assert(num_copiedargs <= MaxArgNum);

					info.idx = i;
					info.is_pointer_or_array = false;
					info.name = raw_lhs_type.GetName().data();
					info.name_size = static_cast<std::uint16_t>(raw_lhs_type.GetName().size());
					info.name_hash = raw_lhs_type.GetID().GetValue();

					continue; // &{const{T}} <- T{arg}
				}
			}
		}
		else if (lhs.IsRValueReference()) { // &&{T} | &&{const{T}}
			const auto unref_lhs = lhs.Name_RemoveRValueReference(); // T | const{T}
			if (type_name_is_const(unref_lhs)) { // &&{const{T}}
				if (rhs.Is(unref_lhs))
					continue; // &&{const{T}} <- const{T}

				const auto raw_lhs = type_name_remove_const(unref_lhs); // T

				if (rhs.Is(raw_lhs))
					continue; // &&{const{T}} <- T

				if (raw_lhs == rhs.Name_RemoveRValueReference())
					continue; // &&{const{T}} <- &&{T}

				Type raw_lhs_type{ raw_lhs };
				if (IsRefConstructible(raw_lhs_type, std::span<const Type>{&rhs, 1}) && Mngr.IsDestructible(raw_lhs_type)) {
					auto& info = info_copiedargs[num_copiedargs++];
					assert(num_copiedargs <= MaxArgNum);

					info.idx = i;
					info.is_pointer_or_array = false;
					info.name = raw_lhs_type.GetName().data();
					info.name_size = static_cast<std::uint16_t>(raw_lhs_type.GetName().size());
					info.name_hash = raw_lhs_type.GetID().GetValue();

					continue; // &&{const{T}} <- T{arg}
				}
			}
			else { // &&{T}
				if (rhs.Is(unref_lhs))
					continue; // &&{T} <- T

				Type raw_lhs_type{ unref_lhs };
				if (IsRefConstructible(raw_lhs_type, std::span<const Type>{&rhs, 1}) && Mngr.IsDestructible(raw_lhs_type)) {
					auto& info = info_copiedargs[num_copiedargs++];
					assert(num_copiedargs <= MaxArgNum);

					info.idx = i;
					info.is_pointer_or_array = false;
					info.name = raw_lhs_type.GetName().data();
					info.name_size = static_cast<std::uint16_t>(raw_lhs_type.GetName().size());
					info.name_hash = raw_lhs_type.GetID().GetValue();

					continue; // &&{T} <- T{arg}
				}
			}
		}
		else { // T
			if (lhs.Is(rhs.Name_RemoveRValueReference()))
				continue; // T <- &&{T}

			if (IsRefConstructible(lhs, std::span<const Type>{&rhs, 1}) && Mngr.IsDestructible(lhs)) {
				auto& info = info_copiedargs[num_copiedargs++];
				assert(num_copiedargs <= MaxArgNum);

				info.idx = i;
				info.is_pointer_or_array = false;
				info.name = lhs.GetName().data();
				info.name_size = static_cast<std::uint16_t>(lhs.GetName().size());
				info.name_hash = lhs.GetID().GetValue();

				continue; // T <- T{arg}
			}
		}

		if (is_pointer_array_compatible(lhs, rhs)) {
			auto raw_lhs = lhs.Name_RemoveCVRef();
			auto& info = info_copiedargs[num_copiedargs++];
			assert(num_copiedargs <= MaxArgNum);
			info.idx = i;
			info.is_pointer_or_array = true;
			info.name = raw_lhs.data();
			info.name_size = static_cast<std::uint16_t>(raw_lhs.size());
			info.name_hash = TypeID{ raw_lhs }.GetValue();
			continue;
		}

		return; // not compatible
	}

	is_compatible = true;

	std::span<const Type> correct_types;
	if (contains_objview) {
		new(&type_buffer)BufferGuard{ rsrc, num_args * sizeof(Type), alignof(Type) };
		auto* types = (Type*)type_buffer.Get();
		for (std::uint8_t i = 0; i < num_args; i++)
			types[i] = paramTypes[i].Is<ObjectView>() ? argTypes[i] : paramTypes[i];
		correct_types = { types,num_args };
	}
	else
		correct_types = paramTypes;

	if (num_copiedargs == 0) {
		new_args = { orig_argptr_buffer, correct_types };
		return;
	}

	// 2. compute offset and alignment

	std::uint32_t max_alignment = 1;
	for (std::uint8_t k = 0; k < num_copiedargs; ++k) {
		std::uint32_t size, alignment;
		if (info_copiedargs[k].is_pointer_or_array) {
			size = static_cast<std::uint32_t>(sizeof(void*));
			alignment = static_cast<std::uint32_t>(alignof(void*));
		}
		else {
			++num_copied_nonptr_args;
			const auto& typeinfo = Mngr.typeinfos.at(info_copiedargs[k].GetType());
			size = static_cast<std::uint32_t>(typeinfo.size);
			alignment = static_cast<std::uint32_t>(typeinfo.alignment);
		}

		std::uint32_t offset = (size_copiedargs + (alignment - 1)) & ~(alignment - 1);
		info_copiedargs[k].offset = offset;
		size_copiedargs = offset + size;

		if (alignment > max_alignment)
			max_alignment = alignment;
	}

	// 3. fill buffer

	// buffer = copied args buffer + argptr buffer + non-ptr arg info buffer

	std::uint32_t offset_new_arg_buffer = 0;
	std::uint32_t offset_new_argptr_buffer = (size_copiedargs + alignof(void*) - 1) & ~(alignof(void*) - 1);
	std::uint32_t offset_new_nonptr_arg_info_buffer = offset_new_argptr_buffer + num_args * sizeof(void*);

	std::uint32_t buffer_size = offset_new_nonptr_arg_info_buffer + num_copied_nonptr_args * sizeof(ArgInfo);

	new(&buffer)BufferGuard{ rsrc, buffer_size, max_alignment };

	auto new_arg_buffer = forward_offset(buffer, offset_new_arg_buffer);
	auto new_argptr_buffer = reinterpret_cast<void**>(forward_offset(buffer, offset_new_argptr_buffer));
	auto new_nonptr_arg_info_buffer = reinterpret_cast<ArgInfo*>(forward_offset(buffer, offset_new_nonptr_arg_info_buffer));

	nonptr_arg_infos = { new_nonptr_arg_info_buffer,num_copied_nonptr_args };

	info_copiedargs[num_copiedargs].idx = static_cast<std::uint8_t>(-1); // guard
	std::uint8_t idx_copiedargs = 0, idx_nonptr_args = 0;
	for (std::uint8_t i = 0; i < num_args; i++) {
		const auto& info = info_copiedargs[idx_copiedargs];
		if (i < info.idx) {
			new_argptr_buffer[i] = orig_argptr_buffer[i];
			continue;
		}
		assert(i == info.idx);

		void* arg_buffer = forward_offset(new_arg_buffer, info.offset);
		new_argptr_buffer[i] = arg_buffer;
		
		// copy
		if (info.is_pointer_or_array)
			buffer_as<void*>(arg_buffer) = orig_argptr_buffer[i];
		else {
			bool success = RefConstruct(
				ObjectView{ info.GetType(), arg_buffer },
				ArgsView{ &orig_argptr_buffer[i], std::span<const Type>{&argTypes[i], 1} }
			);
			assert(success);
			nonptr_arg_infos[idx_nonptr_args++] = info;
		}

		++idx_copiedargs;
	}
	assert(idx_copiedargs == num_copiedargs);
	assert(idx_nonptr_args == num_copied_nonptr_args);

	new_args = { new_argptr_buffer, correct_types };
}

details::NewArgsGuard::~NewArgsGuard() {
	if (buffer.Get()) {
		for (const auto& info : nonptr_arg_infos)
			Mngr.Destruct({ info.GetType(), new_args[info.idx].GetPtr() });
	}
}
