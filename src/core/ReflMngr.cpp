#include <UDRefl/ReflMngr.h>

#include <UDRefl/config.h>

#include <set>

#if defined(_WIN32) || defined(_WIN64)
#include <malloc.h>
#endif

using namespace Ubpa;
using namespace Ubpa::UDRefl;

namespace Ubpa::UDRefl::details {
	DeleteFunc GenerateDeleteFunc(Destructor&& dtor, std::pmr::memory_resource* result_rsrc, size_t size, size_t alignment) {
		assert(size > 0);
		if (dtor) {
			return[d = std::move(dtor), result_rsrc, size, alignment](void* ptr) {
				d(ptr);
				result_rsrc->deallocate(ptr, size, alignment);
			};
		}
		else { // !dtor
			return [result_rsrc, size, alignment](void* ptr) {
				result_rsrc->deallocate(ptr, size, alignment);
			};
		}
	}

	// parameter <- argument
	// - same
	// - reference
	// > - 0 (invalid), 1 (convertible)
	// > - table
	//     |    -     | T | T & | const T & | T&& | const T&& |
	//     |      T   | - |  0  |     0     |  1  |     0     |
	//     |      T & | 0 |  -  |     0     |  0  |     0     |
	//     |const T & | 0 |  0  |     -     |  0  |     0     |
	//     |      T&& | 1 |  0  |     0     |  -  |     0     |
	//     |const T&& | 0 |  0  |     0     |  0  |     -     |
	bool IsPriorityCompatible(std::span<const TypeID> params, std::span<const TypeID> argTypeIDs) {
		if (params.size() != argTypeIDs.size())
			return false;

		for (size_t i = 0; i < params.size(); i++) {
			if (params[i] == argTypeIDs[i])
				continue;

			auto lhs = Mngr->tregistry.Nameof(params[i]);

#ifndef NDEBUG
			// because rhs(arg)'s ID maybe have no name in the registry
			// so we use type_name_add_*_hash(...) to avoid it
			auto rhs = Mngr->tregistry.Nameof(argTypeIDs[i]);
#endif // !NDEBUG

			const std::size_t rhs_hash = argTypeIDs[i].GetValue();

			if (type_name_is_rvalue_reference(lhs)) { // &&{T} | &&{const{T}}
				auto unref_lhs = type_name_remove_reference(lhs); // T | const{T}
				assert(!type_name_is_volatile(unref_lhs));
				if (TypeID{ unref_lhs }.GetValue() == rhs_hash)
					continue; // &&{T} <- T
			}
			else {
				if (!type_name_is_lvalue_reference(lhs)) { // T
					if (type_name_add_rvalue_reference_hash(lhs) == rhs_hash)
						continue; // T <- &&{T}
				}
			}

			return false;
		}

		return true;
	}

	// parameter <- argument
	// - same
	// - reference
	// > - 0 (invalid), 1 (convertible), 2 (copy)
	// > - table
	//     |    -     | T | T & | const T & | T&& | const T&& |
	//     |      T   | - |  0  |     0     |  1  |     0     |
	//     |      T & | 0 |  -  |     0     |  0  |     0     |
	//     |const T & | 1 |  1  |     -     |  1  |     1     |
	//     |      T&& | 1 |  0  |     0     |  -  |     0     |
	//     |const T&& | 1 |  0  |     0     |  1  |     -     |
	bool IsNonArgConstructCompatible(std::span<const TypeID> params, std::span<const TypeID> argTypeIDs) {
		if (params.size() != argTypeIDs.size())
			return false;

		for (size_t i = 0; i < params.size(); i++) {
			if (params[i] == argTypeIDs[i])
				continue;

			auto lhs = Mngr->tregistry.Nameof(params[i]);
#ifndef NDEBUG
			// because rhs(arg)'s ID maybe have no name in the registry
			// so we use type_name_add_*_hash(...) to avoid it
			auto rhs = Mngr->tregistry.Nameof(argTypeIDs[i]);
#endif // !NDEBUG
			const std::size_t rhs_hash = argTypeIDs[i].GetValue();

			if (type_name_is_lvalue_reference(lhs)) { // &{T} | &{const{T}}
				auto unref_lhs = type_name_remove_reference(lhs); // T | const{T}
				if (type_name_is_const(unref_lhs)) { // &{const{T}}
					if (type_name_add_rvalue_reference_hash(unref_lhs) == rhs_hash)
						continue; // &{const{T}} <- &&{const{T}}

					auto raw_lhs = type_name_remove_const(unref_lhs); // T
					TypeID raw_lhs_ID{ raw_lhs };

					if (raw_lhs_ID.GetValue() == rhs_hash
						|| type_name_add_lvalue_reference_hash(raw_lhs) == rhs_hash
						|| type_name_add_rvalue_reference_hash(raw_lhs) == rhs_hash)
						continue; // &{const{T}} <- T | &{T} | &&{T}
				}
			}
			else if (type_name_is_rvalue_reference(lhs)) { // &&{T} | &&{const{T}}
				auto unref_lhs = type_name_remove_reference(lhs); // T | const{T}
				assert(!type_name_is_volatile(unref_lhs));

				if (type_name_is_const(unref_lhs)) { // &&{const{T}}
					auto raw_lhs = type_name_remove_const(unref_lhs); // T
					TypeID raw_lhs_ID{ raw_lhs };

					if (raw_lhs_ID.GetValue() == rhs_hash)
						continue; // &&{const{T}} <- T

					if (type_name_add_rvalue_reference_hash(raw_lhs) == rhs_hash) // &&{const{T}}
						continue; // &&{const{T}} <- &&{T}
				}
				else {
					TypeID unref_lhs_ID{ unref_lhs };

					if (unref_lhs_ID.GetValue() == rhs_hash)
						continue; // &&{T} <- T
				}
			}
			else { // T
				if (type_name_add_rvalue_reference_hash(lhs) == rhs_hash)
					continue; // T <- &&{T}
			}

			return false;
		}

		return true;
	}

	template<typename To, typename From>
	void AddConvertCtor(ReflMngr& mngr) {
		static_assert(std::is_convertible_v<From, To>);
		if constexpr (!std::is_same_v<To, From>) {
			mngr.AddMemberMethod(
				StrIDRegistry::Meta::ctor,
				[](To& obj, const From& value) {
					new(&obj)To{ static_cast<To>(value) };
				}
			);
		}
	}

	template<typename T>
	void RegisterArithmeticConvertion(ReflMngr& mngr) {
		AddConvertCtor<T, bool>(mngr);
		AddConvertCtor<T, std::int8_t>(mngr);
		AddConvertCtor<T, std::int16_t>(mngr);
		AddConvertCtor<T, std::int32_t>(mngr);
		AddConvertCtor<T, std::int64_t>(mngr);
		AddConvertCtor<T, std::uint8_t>(mngr);
		AddConvertCtor<T, std::uint16_t>(mngr);
		AddConvertCtor<T, std::uint32_t>(mngr);
		AddConvertCtor<T, std::uint64_t>(mngr);
		AddConvertCtor<T, float>(mngr);
		AddConvertCtor<T, double>(mngr);
	}

	class NewArgsGuard {
		struct ArgInfo {
			std::size_t typeID;
			std::uint32_t offset;
			std::uint16_t idx;
			bool is_ptr;
		}; // 16 bytes
		// MaxArgNum <= 2^8
		static_assert(sizeof(ArgInfo) * MaxArgNum < 16384);
	public:
		NewArgsGuard(
			bool is_priority,
			std::pmr::memory_resource* rsrc,
			std::span<const TypeID> paramTypeIDs,
			std::span<const TypeID> argTypeIDs,
			ArgPtrBuffer orig_argptr_buffer) :
			rsrc{ rsrc }
		{
			if (argTypeIDs.size() != paramTypeIDs.size())
				return;

			if (is_priority) {
				is_compatible = IsPriorityCompatible(paramTypeIDs, argTypeIDs);
				argptr_buffer = orig_argptr_buffer;
				return;
			}

			// 1. is compatible ? (collect infos)

			const std::uint16_t num_args = static_cast<std::uint16_t>(argTypeIDs.size());

			ArgInfo info_copiedargs[MaxArgNum + 1];
			std::uint16_t num_copiedargs = 0;
			std::uint32_t size_copiedargs = 0;

			for (std::uint16_t i = 0; i < argTypeIDs.size(); i++) {
				if (paramTypeIDs[i] == argTypeIDs[i])
					continue;

				auto lhs = Mngr->tregistry.Nameof(paramTypeIDs[i]);
#ifndef NDEBUG
				auto rhs = Mngr->tregistry.Nameof(argTypeIDs[i]);
#endif // NDEBUG
				const std::size_t rhs_hash = argTypeIDs[i].GetValue();

				if (type_name_is_lvalue_reference(lhs)) { // &{T} | &{const{T}}
					auto unref_lhs = type_name_remove_reference(lhs); // T | const{T}
					if (type_name_is_const(unref_lhs)) { // &{const{T}}
						if (type_name_add_rvalue_reference_hash(unref_lhs) == rhs_hash)
							continue; // &{const{T}} <- &&{const{T}}

						auto raw_lhs = type_name_remove_const(unref_lhs); // T
						TypeID raw_lhs_ID{ raw_lhs };
						if (raw_lhs_ID.GetValue() == rhs_hash
							|| type_name_add_lvalue_reference_hash(raw_lhs) == rhs_hash
							|| type_name_add_const_rvalue_reference_hash(raw_lhs) == rhs_hash)
							continue; // &{const{T}} <- T | &{T} | &&{T}

						assert(!type_name_is_pointer(raw_lhs));

						if (Mngr->IsNonArgConstructible(raw_lhs_ID, std::span<const TypeID>{&argTypeIDs[i], 1})) {
							auto& info = info_copiedargs[num_copiedargs++];
							assert(num_copiedargs <= MaxArgNum);

							info.idx = i;
							info.is_ptr = false;
							info.typeID = raw_lhs_ID.GetValue();

							continue; // &{const{T}} <- T{arg}
						}
					}
				}
				else if (type_name_is_rvalue_reference(lhs)) { // &&{T} | &&{const{T}}
					auto unref_lhs = type_name_remove_reference(lhs); // T | const{T}
					if (type_name_is_const(unref_lhs)) { // &&{const{T}}
						auto raw_lhs = type_name_remove_const(unref_lhs); // T
						TypeID raw_lhs_ID{ raw_lhs };

						if (raw_lhs_ID.GetValue() == rhs_hash)
							continue; // &&{const{T}} <- T

						if (type_name_add_rvalue_reference_hash(raw_lhs) == rhs_hash) // &&{const{T}}
							continue; // &&{const{T}} <- &&{T}

						assert(!type_name_is_pointer(raw_lhs));

						if (Mngr->IsNonArgConstructible(raw_lhs_ID, std::span<const TypeID>{&argTypeIDs[i], 1})) {
							auto& info = info_copiedargs[num_copiedargs++];
							assert(num_copiedargs <= MaxArgNum);

							info.idx = i;
							info.is_ptr = false;
							info.typeID = raw_lhs_ID.GetValue();

							continue; // &&{const{T}} <- T{arg}
						}
					}
					else { // &&{T}
						if (string_hash(unref_lhs) == rhs_hash)
							continue;
					}
				}
				else { // T
					if (type_name_add_rvalue_reference_hash(lhs) == rhs_hash)
						continue; // T <- &&{T}

					if ((type_name_is_pointer(lhs) || Mngr->IsCopyConstructible(lhs))
						&& (
							type_name_add_lvalue_reference_hash(lhs) == rhs_hash
							|| type_name_add_const_lvalue_reference_hash(lhs) == rhs_hash
							|| type_name_add_const_rvalue_reference_hash(lhs) == rhs_hash
						))
					{
						auto& info = info_copiedargs[num_copiedargs++];
						assert(num_copiedargs <= MaxArgNum);

						info.idx = i;
						info.is_ptr = type_name_is_pointer(lhs);
						info.typeID = paramTypeIDs[i].GetValue();

						continue; // T <- T{arg} [copy]
					}

					if (Mngr->IsNonArgConstructible(paramTypeIDs[i], std::span<const TypeID>{&argTypeIDs[i], 1})) {
						auto& info = info_copiedargs[num_copiedargs++];
						assert(num_copiedargs <= MaxArgNum);

						info.idx = i;
						info.is_ptr = false;
						info.typeID = paramTypeIDs[i].GetValue();

						continue; // T <- T{arg}
					}
				}

				return; // not compatible
			}

			is_compatible = true;

			if (num_copiedargs == 0) {
				argptr_buffer = orig_argptr_buffer;
				return;
			}

			// 2. compute offset and alignment

			for (std::uint16_t k = 0; k < num_copiedargs; ++k) {
				std::uint32_t size, alignment;
				if (info_copiedargs[k].is_ptr) {
					size = static_cast<std::uint32_t>(sizeof(void*));
					alignment = static_cast<std::uint32_t>(alignof(void*));
				}
				else {
					++num_copied_nonptr_args;
					const auto& typeinfo = Mngr->typeinfos.at(TypeID{ info_copiedargs[k].typeID });
					size = static_cast<std::uint32_t>(typeinfo.size);
					alignment = static_cast<std::uint32_t>(typeinfo.alignment);
				}

				std::uint32_t offset = (size_copiedargs + (alignment - 1)) / alignment * alignment;
				info_copiedargs[k].offset = offset;
				size_copiedargs = offset + size;

				if (alignment > max_alignment)
					max_alignment = alignment;
			}

			// 3. fill buffer

			// buffer = copied args buffer + argptr buffer + non-ptr arg info buffer

			std::uint32_t offset_new_arg_buffer = 0;
			std::uint32_t offset_new_argptr_buffer = (size_copiedargs + alignof(void*) - 1) / alignof(void*) * alignof(void*);
			std::uint32_t offset_new_nonptr_arg_info_buffer = offset_new_argptr_buffer + num_args * sizeof(void*);

			buffer_size = offset_new_nonptr_arg_info_buffer + num_copied_nonptr_args * sizeof(ArgInfo);

			buffer = rsrc->allocate(buffer_size, max_alignment);

			auto new_arg_buffer = forward_offset(buffer, offset_new_arg_buffer);
			auto new_argptr_buffer = reinterpret_cast<void**>(forward_offset(buffer, offset_new_argptr_buffer));
			new_nonptr_arg_info_buffer = reinterpret_cast<ArgInfo*>(forward_offset(buffer, offset_new_nonptr_arg_info_buffer));

			info_copiedargs[num_copiedargs].idx = static_cast<std::uint16_t>(-1); // guard
			std::uint16_t idx_copiedargs = 0, idx_nonptr_args = 0;
			for (std::uint16_t i = 0; i < num_args; i++) {
				if (i < info_copiedargs[idx_copiedargs].idx) {
					new_argptr_buffer[i] = orig_argptr_buffer[i];
					continue;
				}
				assert(i == info_copiedargs[idx_copiedargs].idx);

				void* arg_buffer = forward_offset(new_arg_buffer, info_copiedargs[idx_copiedargs].offset);
				new_argptr_buffer[i] = arg_buffer;

				// copy
				if (info_copiedargs[idx_copiedargs].is_ptr)
					buffer_as<void*>(arg_buffer) = orig_argptr_buffer[i];
				else {
					bool success = Mngr->NonArgConstruct(
						ObjectPtr{ TypeID{ info_copiedargs[idx_copiedargs].typeID }, arg_buffer },
						std::span<const TypeID>{&argTypeIDs[i], 1},
						static_cast<ArgPtrBuffer>(&orig_argptr_buffer[i])
					);
					assert(success);
					new_nonptr_arg_info_buffer[idx_nonptr_args++] = info_copiedargs[idx_copiedargs];
				}

				++idx_copiedargs;
			}
			assert(idx_copiedargs == num_copiedargs);
			assert(idx_nonptr_args == num_copied_nonptr_args);

			argptr_buffer = new_argptr_buffer;
		}

		~NewArgsGuard() {
			if (buffer) {
				for (const auto& info : std::span<const ArgInfo>{ new_nonptr_arg_info_buffer, num_copied_nonptr_args }) {
					bool success = Mngr->Destruct({ TypeID{ info.typeID }, argptr_buffer[info.idx] });
					assert(success);
				}
				rsrc->deallocate(buffer, buffer_size, max_alignment);
			}
		}

		NewArgsGuard(const NewArgsGuard&) = delete;
		NewArgsGuard& operator=(NewArgsGuard&) = delete;

		bool IsCompatible() const noexcept { return is_compatible; }

		ArgPtrBuffer GetArgPtrBuffer() const noexcept {
			assert(IsCompatible());
			return argptr_buffer;
		}

	private:
		std::pmr::memory_resource* rsrc;
		bool is_compatible{ false };
		std::uint32_t buffer_size{ 0 };
		std::uint32_t max_alignment{ alignof(std::max_align_t) };
		void* buffer{ nullptr };
		ArgInfo* new_nonptr_arg_info_buffer{ nullptr };
		std::uint16_t num_copied_nonptr_args{ 0 };
		ArgPtrBuffer argptr_buffer{ nullptr };
	};

	static InvocableResult IsStaticInvocable(
		bool is_priority,
		TypeID typeID,
		StrID methodID,
		std::span<const TypeID> argTypeIDs)
	{
		auto typetarget = Mngr->typeinfos.find(typeID);

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (iter->second.methodptr.IsStatic()
				&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)
					: Mngr->IsCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)))
				return { true, iter->second.methodptr.GetResultDesc() };
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			auto rst = IsStaticInvocable(is_priority, baseID, methodID, argTypeIDs);
			if (rst)
				return rst;
		}

		return {};
	}

	static InvocableResult IsConstInvocable(
		bool is_priority,
		TypeID typeID,
		StrID methodID,
		std::span<const TypeID> argTypeIDs)
	{
		auto typetarget = Mngr->typeinfos.find(typeID);

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (!iter->second.methodptr.IsMemberVariable()
				&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)
					: Mngr->IsCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)))
				return { true, iter->second.methodptr.GetResultDesc() };
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			auto rst = IsConstInvocable(is_priority, baseID, methodID, argTypeIDs);
			if (rst)
				return rst;
		}

		return {};
	}

	static InvocableResult IsInvocable(
		bool is_priority,
		TypeID typeID,
		StrID methodID,
		std::span<const TypeID> argTypeIDs)
	{
		auto typetarget = Mngr->typeinfos.find(typeID);

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;
		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		{ // 1. object variable and static
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (!iter->second.methodptr.IsMemberConst()
					&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)
						: Mngr->IsCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)))
				{
					return { true, iter->second.methodptr.GetResultDesc() };
				}
			}
		}

		{ // 2. object const
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (iter->second.methodptr.IsMemberConst()
					&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)
						: Mngr->IsCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)))
				{
					return { true, iter->second.methodptr.GetResultDesc() };
				}
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			auto rst = IsInvocable(is_priority, baseID, methodID, argTypeIDs);
			if (rst)
				return rst;
		}

		return {};
	}

	static InvokeResult Invoke(
		bool is_priority,
		std::pmr::memory_resource* args_rsrc,
		TypeID typeID,
		StrID methodID,
		void* result_buffer,
		std::span<const TypeID> argTypeIDs,
		ArgPtrBuffer argptr_buffer)
	{
		assert(Mngr->GetDereferenceProperty(typeID) == DereferenceProperty::NotReference);

		auto typetarget = Mngr->typeinfos.find(typeID);

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);
		
		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (iter->second.methodptr.IsStatic()) {
				NewArgsGuard guard{
					is_priority, args_rsrc,
					iter->second.methodptr.GetParamList(), argTypeIDs, argptr_buffer
				};
				if (!guard.IsCompatible())
					continue;

				return {
					true,
					iter->second.methodptr.GetResultDesc().typeID,
					std::move(iter->second.methodptr.Invoke(result_buffer, guard.GetArgPtrBuffer()))
				};
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			auto rst = Invoke(is_priority, args_rsrc, baseID, methodID, result_buffer, argTypeIDs, argptr_buffer);
			if (rst.success)
				return rst;
		}

		return {};
	}

	static InvokeResult Invoke(
		bool is_priority,
		std::pmr::memory_resource* args_rsrc,
		ConstObjectPtr obj,
		StrID methodID,
		void* result_buffer,
		std::span<const TypeID> argTypeIDs,
		ArgPtrBuffer argptr_buffer)
	{
		assert(Mngr->GetDereferenceProperty(obj.GetID()) == DereferenceProperty::NotReference);

		auto typetarget = Mngr->typeinfos.find(obj.GetID());

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (iter->second.methodptr.IsMemberVariable()) {
				NewArgsGuard guard{
					is_priority, args_rsrc,
					iter->second.methodptr.GetParamList(), argTypeIDs, argptr_buffer
				};
				if (!guard.IsCompatible())
					continue;
				return {
						true,
						iter->second.methodptr.GetResultDesc().typeID,
						std::move(iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, guard.GetArgPtrBuffer()))
				};
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			auto rst = Invoke(
				is_priority, args_rsrc,
				ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
				methodID, result_buffer, argTypeIDs, argptr_buffer
			);
			if (rst.success)
				return rst;
		}

		return {};
	}

	static InvokeResult Invoke(
		bool is_priority,
		std::pmr::memory_resource* args_rsrc,
		ObjectPtr obj,
		StrID methodID,
		void* result_buffer,
		std::span<const TypeID> argTypeIDs,
		ArgPtrBuffer argptr_buffer)
	{
		assert(Mngr->GetDereferenceProperty(obj.GetID()) == DereferenceProperty::NotReference);

		auto typetarget = Mngr->typeinfos.find(obj.GetID());

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;


		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		// 1. object variable and static
		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (!iter->second.methodptr.IsMemberConst()) {
				NewArgsGuard guard{
					is_priority, args_rsrc,
					iter->second.methodptr.GetParamList(), argTypeIDs, argptr_buffer
				};
				if (!guard.IsCompatible())
					continue;

				return {
					true,
					iter->second.methodptr.GetResultDesc().typeID,
					std::move(iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, guard.GetArgPtrBuffer()))
				};
			}
		}

		// 2. object const
		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (iter->second.methodptr.IsMemberConst()) {
				NewArgsGuard guard{
					is_priority, args_rsrc,
					iter->second.methodptr.GetParamList(), argTypeIDs, argptr_buffer
				};
				if (!guard.IsCompatible())
					continue;

				return {
					true,
					iter->second.methodptr.GetResultDesc().typeID,
					std::move(iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, guard.GetArgPtrBuffer()))
				};
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			auto rst = Invoke(
				is_priority, args_rsrc,
				ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
				methodID, result_buffer, argTypeIDs, argptr_buffer
			);
			if (rst.success)
				return rst;
		}

		return {};
	}

	static SharedObject MInvoke(
		bool is_priority,
		std::pmr::memory_resource* args_rsrc,
		TypeID typeID,
		StrID methodID,
		std::span<const TypeID> argTypeIDs,
		ArgPtrBuffer argptr_buffer,
		std::pmr::memory_resource* rst_rsrc)
	{
		assert(Mngr->GetDereferenceProperty(typeID) == DereferenceProperty::NotReference);

		auto typetarget = Mngr->typeinfos.find(typeID);

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (iter->second.methodptr.IsStatic()) {
				NewArgsGuard guard{
					is_priority, args_rsrc,
					iter->second.methodptr.GetParamList(), argTypeIDs, argptr_buffer
				};

				if (!guard.IsCompatible())
					continue;

				const auto& methodptr = iter->second.methodptr;
				const auto& rst_desc = methodptr.GetResultDesc();

				if (rst_desc.IsVoid()) {
					iter->second.methodptr.Invoke(nullptr, guard.GetArgPtrBuffer());
					return {
						{rst_desc.typeID, nullptr},
						[](void* ptr) { assert(ptr); }
					};
				}
				else if (type_name_is_reference(Mngr->tregistry.Nameof(rst_desc.typeID))) {
					std::aligned_storage_t<sizeof(void*)> buffer;
					iter->second.methodptr.Invoke(&buffer, guard.GetArgPtrBuffer());
					return {
						{rst_desc.typeID, buffer_as<void*>(&buffer)},
						[](void* ptr) { assert(ptr); }
					};
				}
				else {
					void* result_buffer = rst_rsrc->allocate(rst_desc.size, rst_desc.alignment);
					auto dtor = iter->second.methodptr.Invoke(result_buffer, guard.GetArgPtrBuffer());
					return {
						{rst_desc.typeID, result_buffer},
						GenerateDeleteFunc(std::move(dtor), rst_rsrc, rst_desc.size, rst_desc.alignment)
					};
				}
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			auto rst = MInvoke(is_priority, args_rsrc, baseID, methodID, argTypeIDs, argptr_buffer, rst_rsrc);
			if (rst.GetID())
				return rst;
		}

		return nullptr;
	}

	static SharedObject MInvoke(
		bool is_priority,
		std::pmr::memory_resource* args_rsrc,
		ConstObjectPtr obj,
		StrID methodID,
		std::span<const TypeID> argTypeIDs,
		ArgPtrBuffer argptr_buffer,
		std::pmr::memory_resource* rst_rsrc)
	{
		assert(rst_rsrc);
		assert(Mngr->GetDereferenceProperty(obj.GetID()) == DereferenceProperty::NotReference);

		auto typetarget = Mngr->typeinfos.find(obj.GetID());

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (!iter->second.methodptr.IsMemberVariable()) {
				NewArgsGuard guard{
					is_priority, args_rsrc,
					iter->second.methodptr.GetParamList(), argTypeIDs, argptr_buffer
				};

				if (!guard.IsCompatible())
					continue;

				const auto& methodptr = iter->second.methodptr;
				const auto& rst_desc = methodptr.GetResultDesc();

				if (rst_desc.IsVoid()) {
					auto dtor = iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, guard.GetArgPtrBuffer());
					return {
						{rst_desc.typeID, nullptr},
						[](void* ptr) { assert(!ptr); }
					};
				}
				else if (type_name_is_reference(Mngr->tregistry.Nameof(rst_desc.typeID))) {
					std::aligned_storage_t<sizeof(void*)> buffer;
					iter->second.methodptr.Invoke(obj.GetPtr(), &buffer, guard.GetArgPtrBuffer());
					return {
						{rst_desc.typeID, buffer_as<void*>(&buffer)},
						[](void* ptr) { assert(ptr); }
					};
				}
				else {
					void* result_buffer = rst_rsrc->allocate(rst_desc.size, rst_desc.alignment);
					auto dtor = iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, guard.GetArgPtrBuffer());
					return {
						{rst_desc.typeID, result_buffer},
						GenerateDeleteFunc(std::move(dtor), rst_rsrc, rst_desc.size, rst_desc.alignment)
					};
				}
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			auto rst = MInvoke(
				is_priority, args_rsrc,
				ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
				methodID, argTypeIDs, argptr_buffer, rst_rsrc
			);
			if (rst.GetID())
				return rst;
		}

		return nullptr;
	}

	static SharedObject MInvoke(
		bool is_priority,
		std::pmr::memory_resource* args_rsrc,
		ObjectPtr obj,
		StrID methodID,
		std::span<const TypeID> argTypeIDs,
		ArgPtrBuffer argptr_buffer,
		std::pmr::memory_resource* rst_rsrc)
	{
		assert(rst_rsrc);
		assert(Mngr->GetDereferenceProperty(obj.GetID()) == DereferenceProperty::NotReference);

		auto typetarget = Mngr->typeinfos.find(obj.GetID());

		if (typetarget == Mngr->typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		{ // 1. object variable and static
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (!iter->second.methodptr.IsMemberConst()) {
					NewArgsGuard guard{
						is_priority, args_rsrc,
						iter->second.methodptr.GetParamList(), argTypeIDs, argptr_buffer
					};

					if (!guard.IsCompatible())
						continue;

					const auto& methodptr = iter->second.methodptr;
					const auto& rst_desc = methodptr.GetResultDesc();

					if (rst_desc.IsVoid()) {
						auto dtor = iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, guard.GetArgPtrBuffer());
						return {
							{rst_desc.typeID, nullptr},
							[](void* ptr) { assert(!ptr); }
						};
					}
					else if (type_name_is_reference(Mngr->tregistry.Nameof(rst_desc.typeID))) {
						std::aligned_storage_t<sizeof(void*)> buffer;
						iter->second.methodptr.Invoke(obj.GetPtr(), &buffer, guard.GetArgPtrBuffer());
						return {
							{rst_desc.typeID, buffer_as<void*>(&buffer)},
							[](void* ptr) { assert(ptr); }
						};
					}
					else {
						void* result_buffer = rst_rsrc->allocate(rst_desc.size, rst_desc.alignment);
						auto dtor = iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, guard.GetArgPtrBuffer());
						return {
							{rst_desc.typeID, result_buffer},
							GenerateDeleteFunc(std::move(dtor), rst_rsrc, rst_desc.size, rst_desc.alignment)
						};
					}
				}
			}
		}

		{ // 2. object const
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (iter->second.methodptr.IsMemberConst()) {
					NewArgsGuard guard{
						is_priority, args_rsrc,
						iter->second.methodptr.GetParamList(), argTypeIDs, argptr_buffer
					};

					if (!guard.IsCompatible())
						continue;

					const auto& methodptr = iter->second.methodptr;
					const auto& rst_desc = methodptr.GetResultDesc();

					if (rst_desc.IsVoid()) {
						auto dtor = iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, guard.GetArgPtrBuffer());
						return {
							{rst_desc.typeID, nullptr},
							[](void* ptr) { assert(!ptr); }
						};
					}
					else if (type_name_is_reference(Mngr->tregistry.Nameof(rst_desc.typeID))) {
						std::aligned_storage_t<sizeof(void*)> buffer;
						iter->second.methodptr.Invoke(obj.GetPtr(), &buffer, guard.GetArgPtrBuffer());
						return {
							{rst_desc.typeID, buffer_as<void*>(&buffer)},
							[](void* ptr) { assert(ptr); }
						};
					}
					else {
						void* result_buffer = rst_rsrc->allocate(rst_desc.size, rst_desc.alignment);
						auto dtor = iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, guard.GetArgPtrBuffer());
						return {
							{rst_desc.typeID, result_buffer},
							GenerateDeleteFunc(std::move(dtor), rst_rsrc, rst_desc.size, rst_desc.alignment)
						};
					}
				}
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			auto rst = MInvoke(
				is_priority, args_rsrc,
				ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
				methodID, argTypeIDs, argptr_buffer, rst_rsrc
			);
			if (rst.GetID())
				return rst;
		}

		return nullptr;
	}

	static bool ForEachTypeID(
		TypeID typeID,
		const std::function<bool(TypeID)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!func(typeID))
			return false;

		auto target = ReflMngr::Instance().typeinfos.find(typeID);

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		const auto& typeinfo = target->second;

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachTypeID(baseID, func, visitedVBs))
				return false;
		}
		return true;
	}

	static bool ForEachTypeInfo(
		TypeID typeID,
		const std::function<bool(TypeRef)>& func,
		std::set<TypeID>& visitedVBs)
	{
		auto target = ReflMngr::Instance().typeinfos.find(typeID);

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		if (!func({ typeID, typeinfo }))
			return false;

		for (auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachTypeInfo(baseID, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachRWVar(
		ObjectPtr obj,
		const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!obj.Valid())
			return true;

		auto target = ReflMngr::Instance().typeinfos.find(obj.GetID());

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
			if (!fieldInfo.fieldptr.IsConst()) {
				if (!func({ obj.GetID(), typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.RWVar(obj.GetPtr())))
					return false;
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachRWVar(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachRVar(
		ConstObjectPtr obj,
		const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!obj.Valid())
			return true;

		auto target = ReflMngr::Instance().typeinfos.find(obj.GetID());

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
			if (!func({ obj.GetID(), typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.RVar(obj.GetPtr())))
				return false;
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachRVar(ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachRWVar(
		TypeID typeID,
		const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!typeID)
			return true;

		auto target = ReflMngr::Instance().typeinfos.find(typeID);

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
			if (fieldInfo.fieldptr.IsUnowned() && !fieldInfo.fieldptr.IsConst()) {
				if (!func({ typeID, typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.RWVar()))
					return false;
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachRWVar(baseID, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachRVar(
		TypeID typeID,
		const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func,
		std::set<TypeID>& visitedVBs)
	{
		if (!typeID)
			return true;

		auto target = ReflMngr::Instance().typeinfos.find(typeID);

		if (target == ReflMngr::Instance().typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
			if (fieldInfo.fieldptr.IsUnowned()) {
				if (!func({ typeID, typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.RVar()))
					return false;
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachRVar(baseID, func, visitedVBs))
				return false;
		}

		return true;
	}
}

ReflMngr::ReflMngr() {
	MethodInfo methodinfo_malloc{ GenerateMethodPtr<&malloc>() };
	MethodInfo methodinfo_free{ GenerateMethodPtr<&free>() };

#if defined(_WIN32) || defined(_WIN64)
	#define UBPA_UDREFL_ALIGN_MALLOC _aligned_malloc
#else
	#define UBPA_UDREFL_ALIGN_MALLOC aligned_alloc
#endif // defined(_WIN32) || defined(_WIN64)
	MethodInfo methodinfo_aligned_malloc{ GenerateMethodPtr<&UBPA_UDREFL_ALIGN_MALLOC>() };
#undef UBPA_UDREFL_ALIGN_MALLOC

#if defined(_WIN32) || defined(_WIN64)
#define UBPA_UDREFL_ALIGN_FREE _aligned_free
#else
#define UBPA_UDREFL_ALIGN_FREE free
#endif // defined(_WIN32) || defined(_WIN64)
	MethodInfo methodinfo_aligned_free{ GenerateMethodPtr<&UBPA_UDREFL_ALIGN_FREE>() };
#undef UBPA_UDREFL_ALIGN_FREE

	TypeInfo global{
		0,
		0,
		{}, // fieldinfos
		{ // methodinfos
			{StrIDRegistry::MetaID::malloc, std::move(methodinfo_malloc)},
			{StrIDRegistry::MetaID::free, std::move(methodinfo_free)},
			{StrIDRegistry::MetaID::aligned_malloc, std::move(methodinfo_aligned_malloc)},
			{StrIDRegistry::MetaID::aligned_free, std::move(methodinfo_aligned_free)},
		},
	};

	typeinfos.emplace(tregistry.Register(TypeIDRegistry::Meta::global), std::move(global));

	RegisterType<ContainerType>();
	AddField<ContainerType::Unknown>("Unknown");
	AddField<ContainerType::Array>("Array");
	AddField<ContainerType::Vector>("Vector");
	AddField<ContainerType::Deque>("Deque");
	AddField<ContainerType::ForwardList>("ForwardList");
	AddField<ContainerType::List>("List");
	AddField<ContainerType::Set>("Set");
	AddField<ContainerType::Map>("Map");
	AddField<ContainerType::UnorderedSet>("UnorderedSet");
	AddField<ContainerType::UnorderedMap>("UnorderedMap");
	AddField<ContainerType::Stack>("Stack");
	AddField<ContainerType::Queue>("Queue");

	RegisterType<bool>();
	RegisterType<std::int8_t>();
	RegisterType<std::int16_t>();
	RegisterType<std::int32_t>();
	RegisterType<std::int64_t>();
	RegisterType<std::uint8_t>();
	RegisterType<std::uint16_t>();
	RegisterType<std::uint32_t>();
	RegisterType<std::uint64_t>();
	RegisterType<float>();
	RegisterType<double>();

	details::RegisterArithmeticConvertion<bool>(*this);
	details::RegisterArithmeticConvertion<std::int8_t>(*this);
	details::RegisterArithmeticConvertion<std::int16_t>(*this);
	details::RegisterArithmeticConvertion<std::int32_t>(*this);
	details::RegisterArithmeticConvertion<std::int64_t>(*this);
	details::RegisterArithmeticConvertion<std::uint8_t>(*this);
	details::RegisterArithmeticConvertion<std::uint16_t>(*this);
	details::RegisterArithmeticConvertion<std::uint32_t>(*this);
	details::RegisterArithmeticConvertion<std::uint64_t>(*this);
	details::RegisterArithmeticConvertion<float>(*this);
	details::RegisterArithmeticConvertion<double>(*this);
}

void ReflMngr::Clear() noexcept {
	// field attrs
	for (auto& [typeID, typeinfo] : typeinfos) {
		for (auto& [fieldID, fieldinfo] : typeinfo.fieldinfos)
			fieldinfo.attrs.clear();
	}

	// type attrs
	for (auto& [ID, typeinfo] : typeinfos)
		typeinfo.attrs.clear();

	// type dynamic field
	for (auto& [typeID, typeinfo] : typeinfos) {
		auto iter = typeinfo.fieldinfos.begin();
		while (iter != typeinfo.fieldinfos.end()) {
			auto cur = iter;
			++iter;

			if (cur->second.fieldptr.IsDynamicShared())
				typeinfo.fieldinfos.erase(cur);
		}
	}

	typeinfos.clear();
}

ReflMngr::~ReflMngr() {
	Clear();
}

TypeID ReflMngr::RegisterType(std::string_view name, size_t size, size_t alignment) {
	TypeID ID{ name };

	auto target = typeinfos.find(ID);
	if (target != typeinfos.end())
		return ID;

	tregistry.Register(ID, name);
	typeinfos.emplace_hint(target, ID, TypeInfo{ size,alignment });

	return ID;
}

StrID ReflMngr::AddField(TypeID typeID, std::string_view name, FieldInfo fieldinfo) {
	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return {};
	auto& typeinfo = ttarget->second;
	StrID fieldID = nregistry.Register(name);
	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end())
		return {};
	typeinfo.fieldinfos.emplace_hint(ftarget, fieldID, std::move(fieldinfo));
	return fieldID;
}

StrID ReflMngr::AddMethod(TypeID typeID, std::string_view name, MethodInfo methodinfo) {
	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return {};
	auto& typeinfo = ttarget->second;
	StrID methodID = nregistry.Register(name);
	size_t mcnt = typeinfo.methodinfos.count(methodID);
	auto miter = typeinfo.methodinfos.find(methodID);
	bool flag = true;
	for (size_t i = 0; i < mcnt; ++i, ++miter) {
		if (!miter->second.methodptr.IsDistinguishableWith(methodinfo.methodptr)) {
			flag = false;
			break;
		}
	}
	if (!flag)
		return {};
	typeinfo.methodinfos.emplace(methodID, std::move(methodinfo));
	return methodID;
}

bool ReflMngr::AddBase(TypeID derivedID, TypeID baseID, BaseInfo baseinfo) {
	auto ttarget = typeinfos.find(derivedID);
	if (ttarget == typeinfos.end())
		return false;
	auto& typeinfo = ttarget->second;
	auto btarget = typeinfo.baseinfos.find(baseID);
	if (btarget != typeinfo.baseinfos.end())
		return false;
	typeinfo.baseinfos.emplace_hint(btarget, baseID, std::move(baseinfo));
	return true;
}

bool ReflMngr::AddAttr(TypeID typeID, const Attr& attr) {
	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return false;
	auto& typeinfo = ttarget->second;
	auto atarget = typeinfo.attrs.find(attr);
	if (atarget != typeinfo.attrs.end())
		return false;
	typeinfo.attrs.emplace_hint(atarget, attr);
	return true;
}

void* ReflMngr::Malloc(size_t size) const {
	return Invoke<void*>(GlobalID, StrIDRegistry::MetaID::malloc, std::move(size));
}

bool ReflMngr::Free(void* ptr) const {
	return InvokeArgs(GlobalID, StrIDRegistry::MetaID::free, nullptr, std::move(ptr));
}

void* ReflMngr::AlignedMalloc(size_t size, size_t alignment) const {
	return Invoke<void*>(GlobalID, StrIDRegistry::MetaID::aligned_malloc, std::move(size), std::move(alignment));
}

bool ReflMngr::AlignedFree(void* ptr) const {
	return InvokeArgs(GlobalID, StrIDRegistry::MetaID::aligned_free, nullptr, std::move(ptr));
}

ObjectPtr ReflMngr::NonArgCopyNew(TypeID typeID, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	if (!IsNonArgConstructible(typeID, argTypeIDs))
		return nullptr;

	const auto& typeinfo = typeinfos.at(typeID);

	void* buffer = typeinfo.alignment <= std::alignment_of_v<std::max_align_t> ?
		Malloc(typeinfo.size)
		: AlignedMalloc(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectPtr obj{ typeID, buffer };
	bool success = NonArgConstruct(obj, argTypeIDs, argptr_buffer);
	assert(success);

	return obj;
}

ObjectPtr ReflMngr::New(TypeID typeID, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	if (!IsConstructible(typeID, argTypeIDs))
		return nullptr;

	const auto& typeinfo = typeinfos.at(typeID);

	void* buffer = typeinfo.alignment <= std::alignment_of_v<std::max_align_t> ?
		Malloc(typeinfo.size)
		: AlignedMalloc(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectPtr obj{ typeID, buffer };
	bool success = Construct(obj, argTypeIDs, argptr_buffer);
	assert(success);
	
	return obj;
}

bool ReflMngr::Delete(ConstObjectPtr obj) const {
	bool dtor_success = Destruct(obj);
	if (!dtor_success)
		return false;

	const auto& typeinfo = typeinfos.at(obj.GetID());

	bool free_success;
	if (typeinfo.alignment <= std::alignment_of_v<std::max_align_t>)
		free_success = Free(const_cast<void*>(obj.GetPtr()));
	else
		free_success = AlignedFree(const_cast<void*>(obj.GetPtr()));

	return free_success;
}

SharedObject ReflMngr::MakeShared(TypeID typeID, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	ObjectPtr obj = New(typeID, argTypeIDs, argptr_buffer);

	if (!obj.Valid())
		return { obj, [](void*) {} };

	return { obj, [typeID](void* ptr) {
		bool success = ReflMngr::Instance().Delete({typeID, ptr});
		assert(success);
	} };
}

ObjectPtr ReflMngr::StaticCast_DerivedToBase(ObjectPtr obj, TypeID typeID) const {
	assert(typeID);
	assert(GetDereferenceProperty(obj.GetID()) == DereferenceProperty::NotReference);

	if (obj.GetID() == typeID)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { typeID, nullptr };

	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto ptr = StaticCast_DerivedToBase(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, typeID);
		if (ptr.GetID())
			return ptr;
	}

	return nullptr;
}

ObjectPtr ReflMngr::StaticCast_BaseToDerived(ObjectPtr obj, TypeID typeID) const {
	assert(typeID);
	assert(GetDereferenceProperty(obj.GetID()) == DereferenceProperty::NotReference);

	if (obj.GetID() == typeID)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { typeID, nullptr };

	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto ptr = StaticCast_BaseToDerived(obj, baseID);
		if (ptr.GetID())
			return { baseID, baseinfo.IsVirtual() ? nullptr : baseinfo.StaticCast_BaseToDerived(obj.GetPtr()) };
	}
	
	return nullptr;
}

ObjectPtr ReflMngr::DynamicCast_BaseToDerived(ObjectPtr obj, TypeID typeID) const {
	assert(typeID);
	assert(GetDereferenceProperty(obj.GetID()) == DereferenceProperty::NotReference);

	if (obj.GetID() == typeID)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { typeID, nullptr };

	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto ptr = DynamicCast_BaseToDerived(ObjectPtr{ baseID, baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) }, typeID);
		if (ptr.GetID())
			return { baseID, baseinfo.IsPolymorphic() ? baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) : nullptr };
	}

	return nullptr;
}

ObjectPtr ReflMngr::StaticCast(ObjectPtr obj, TypeID typeID) const {
	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetID())
		return ptr_d2b;

	auto ptr_b2d = StaticCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetID())
		return ptr_b2d;

	return nullptr;
}

ObjectPtr ReflMngr::DynamicCast(ObjectPtr obj, TypeID typeID) const {
	auto ptr_b2d = DynamicCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetID())
		return ptr_b2d;

	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetID())
		return ptr_d2b;

	return nullptr;
}

ConstObjectPtr ReflMngr::StaticCast_DerivedToBase(ConstObjectPtr obj, TypeID typeID) const {
	if (auto prop = GetDereferenceProperty(typeID); DereferenceProperty_IsReference(prop))
		return StaticCast_DerivedToBase(DereferenceAsConst(obj), typeID);

	return StaticCast_DerivedToBase(ConstCast(obj), typeID);
}

ConstObjectPtr ReflMngr::StaticCast_BaseToDerived(ConstObjectPtr obj, TypeID typeID) const {
	if (auto prop = GetDereferenceProperty(typeID); DereferenceProperty_IsReference(prop))
		return StaticCast_BaseToDerived(DereferenceAsConst(obj), typeID);

	return StaticCast_BaseToDerived(ConstCast(obj), typeID);
}

ConstObjectPtr ReflMngr::DynamicCast_BaseToDerived(ConstObjectPtr obj, TypeID typeID) const {
	if (auto prop = GetDereferenceProperty(typeID); DereferenceProperty_IsReference(prop))
		return DynamicCast_BaseToDerived(DereferenceAsConst(obj), typeID);

	return DynamicCast_BaseToDerived(ConstCast(obj), typeID);
}

ConstObjectPtr ReflMngr::StaticCast(ConstObjectPtr obj, TypeID typeID) const {
	if (auto prop = GetDereferenceProperty(typeID); DereferenceProperty_IsReference(prop))
		return StaticCast(DereferenceAsConst(obj), typeID);

	return StaticCast(ConstCast(obj), typeID);
}

ConstObjectPtr ReflMngr::DynamicCast(ConstObjectPtr obj, TypeID typeID) const {
	if (auto prop = GetDereferenceProperty(typeID); DereferenceProperty_IsReference(prop))
		return DynamicCast(DereferenceAsConst(obj), typeID);

	return DynamicCast(ConstCast(obj), typeID);
}

ObjectPtr ReflMngr::RWVar(TypeID typeID, StrID fieldID) {
	if (auto prop = GetDereferenceProperty(typeID); DereferenceProperty_IsReference(prop)) {
		if (prop == DereferenceProperty::Const)
			return nullptr;

		return RWVar(Dereference(typeID), fieldID);
	}

	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;

	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end() && ftarget->second.fieldptr.IsUnowned())
		return ftarget->second.fieldptr.RWVar();

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RWVar(baseID, fieldID);
		if (bptr.GetID())
			return bptr;
	}

	return nullptr;
}

ConstObjectPtr ReflMngr::RVar(TypeID typeID, StrID fieldID) const {
	if (GetDereferenceProperty(typeID) != DereferenceProperty::NotReference)
		return RVar(Dereference(typeID), fieldID);

	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;


	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end() && ftarget->second.fieldptr.IsUnowned())
		return ftarget->second.fieldptr.RVar();

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RVar(baseID, fieldID);
		if (bptr.GetID())
			return bptr;
	}

	return nullptr;
}

ObjectPtr ReflMngr::RWVar(ObjectPtr obj, StrID fieldID) {
	if (auto prop = GetDereferenceProperty(obj.GetID()); DereferenceProperty_IsReference(prop)) {
		if (prop == DereferenceProperty::Const)
			return nullptr;

		return RWVar(Dereference(obj), fieldID);
	}

	auto ttarget = typeinfos.find(obj.GetID());
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;

	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end() && ftarget->second.fieldptr.IsVariable())
		return ftarget->second.fieldptr.RWVar(obj.GetPtr());

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RWVar(ObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, fieldID);
		if (bptr.GetID())
			return bptr;
	}

	return nullptr;
}

ConstObjectPtr ReflMngr::RVar(ConstObjectPtr obj, StrID fieldID) const {
	if (GetDereferenceProperty(obj.GetID()) != DereferenceProperty::NotReference)
		return RVar(DereferenceAsConst(obj), fieldID);

	auto ttarget = typeinfos.find(obj.GetID());
	if (ttarget == typeinfos.end())
		return nullptr;

	const auto& typeinfo = ttarget->second;

	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end())
		return ftarget->second.fieldptr.RVar(obj.GetPtr());

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = RVar(ConstObjectPtr{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, fieldID);
		if (bptr.GetID())
			return bptr;
	}

	return nullptr;
}

ObjectPtr ReflMngr::RWVar(ObjectPtr obj, TypeID baseID, StrID fieldID) {
	auto base = StaticCast_DerivedToBase(obj, baseID);
	if (!base.GetID())
		return nullptr;
	return RWVar(base, fieldID);
}

ConstObjectPtr ReflMngr::RVar(ConstObjectPtr obj, TypeID baseID, StrID fieldID) const {
	auto base = StaticCast_DerivedToBase(obj, baseID);
	if (!base.GetID())
		return nullptr;
	return RVar(base, fieldID);
}

bool ReflMngr::IsCompatible(std::span<const TypeID> params, std::span<const TypeID> argTypeIDs) const {
	if (params.size() != argTypeIDs.size())
		return false;

	for (size_t i = 0; i < params.size(); i++) {
		if (params[i] == argTypeIDs[i])
			continue;

		auto lhs = tregistry.Nameof(params[i]);
#ifndef NDEBUG
		// because rhs(arg)'s ID maybe have no name in the registry
		// so we use type_name_add_*_hash(...) to avoid it
		auto rhs = tregistry.Nameof(argTypeIDs[i]);
#endif // !NDEBUG
		const std::size_t rhs_hash = argTypeIDs[i].GetValue();

		if (type_name_is_lvalue_reference(lhs)) { // &{T} | &{const{T}}
			auto unref_lhs = type_name_remove_reference(lhs); // T | const{T}
			if (type_name_is_const(unref_lhs)) { // &{const{T}}
				if (type_name_add_rvalue_reference_hash(unref_lhs) == rhs_hash)
					continue; // &{const{T}} <- &&{const{T}}

				auto raw_lhs = type_name_remove_const(unref_lhs); // T
				TypeID raw_lhs_ID{ raw_lhs };

				if (raw_lhs_ID.GetValue() == rhs_hash
					|| type_name_add_lvalue_reference_hash(raw_lhs) == rhs_hash
					|| type_name_add_rvalue_reference_hash(raw_lhs) == rhs_hash)
					continue; // &{const{T}} <- T | &{T} | &&{T}

				if (IsNonArgConstructible(raw_lhs_ID, std::span<const TypeID>{&argTypeIDs[i], 1}))
					continue; // &{const{T}} <- T{arg}
			}
		}
		else if (type_name_is_rvalue_reference(lhs)) { // &&{T} | &&{const{T}}
			auto unref_lhs = type_name_remove_reference(lhs); // T | const{T}

			if (type_name_is_const(unref_lhs)) { // &&{const{T}}
				auto raw_lhs = type_name_remove_const(unref_lhs); // T
				TypeID raw_lhs_ID{ raw_lhs };

				if (raw_lhs_ID.GetValue() == rhs_hash)
					continue; // &&{const{T}} <- T

				if (type_name_add_rvalue_reference_hash(raw_lhs) == rhs_hash) // &&{const{T}}
					continue; // &&{const{T}} <- &&{T}

				if (IsNonArgConstructible(raw_lhs_ID, std::span<const TypeID>{&argTypeIDs[i], 1}))
					continue; // &&{const{T}} <- T{arg}
			}
			else {
				TypeID unref_lhs_ID{ unref_lhs };

				if (unref_lhs_ID.GetValue() == rhs_hash)
					continue; // &&{T} <- T

				if (IsNonArgConstructible(unref_lhs_ID, std::span<const TypeID>{&argTypeIDs[i], 1}))
					continue; // &&{T} <- T{arg}
			}
		}
		else { // T
			if (type_name_add_rvalue_reference_hash(lhs) == rhs_hash)
				continue; // T <- &&{T}

			if ((type_name_is_pointer(lhs) || IsCopyConstructible(lhs))
				&& (
					type_name_add_lvalue_reference_hash(lhs) == rhs_hash
					|| type_name_add_const_lvalue_reference_hash(lhs) == rhs_hash
					|| type_name_add_const_rvalue_reference_hash(lhs) == rhs_hash
				))
			{
				continue; // T <- T{arg} [copy]
			}

			if (IsNonArgConstructible(params[i], std::span<const TypeID>{&argTypeIDs[i], 1}))
				continue; // T <- T{arg}
		}

		return false;
	}

	return true;
}

InvocableResult ReflMngr::IsStaticInvocable(
	TypeID typeID,
	StrID methodID,
	std::span<const TypeID> argTypeIDs) const
{
	if (GetDereferenceProperty(typeID) != DereferenceProperty::NotReference)
		return IsStaticInvocable(Dereference(typeID), methodID, argTypeIDs);

	if (auto priority_rst = details::IsStaticInvocable(true, typeID, methodID, argTypeIDs))
		return priority_rst;

	return details::IsStaticInvocable(false, typeID, methodID, argTypeIDs);
}

InvocableResult ReflMngr::IsConstInvocable(
	TypeID typeID,
	StrID methodID,
	std::span<const TypeID> argTypeIDs) const
{
	if (GetDereferenceProperty(typeID) != DereferenceProperty::NotReference)
		return IsConstInvocable(Dereference(typeID), methodID, argTypeIDs);

	if (auto priority_rst = details::IsConstInvocable(true, typeID, methodID, argTypeIDs))
		return priority_rst;

	return details::IsConstInvocable(false, typeID, methodID, argTypeIDs);
}

InvocableResult ReflMngr::IsInvocable(
	TypeID typeID,
	StrID methodID,
	std::span<const TypeID> argTypeIDs) const
{
	if (GetDereferenceProperty(typeID) != DereferenceProperty::NotReference)
		return IsInvocable(Dereference(typeID), methodID, argTypeIDs);

	if (auto priority_rst = details::IsInvocable(true, typeID, methodID, argTypeIDs))
		return priority_rst;

	return details::IsInvocable(false, typeID, methodID, argTypeIDs);
}

InvokeResult ReflMngr::Invoke(
	TypeID typeID,
	StrID methodID,
	void* result_buffer,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer) const
{
	if (GetDereferenceProperty(typeID) != DereferenceProperty::NotReference)
		return Invoke(Dereference(typeID), methodID, result_buffer, argTypeIDs, argptr_buffer);

	if(auto priority_rst = details::Invoke(true, &temporary_resource, typeID, methodID, result_buffer, argTypeIDs, argptr_buffer))
		return priority_rst;

	return details::Invoke(false, &temporary_resource, typeID, methodID, result_buffer, argTypeIDs, argptr_buffer);
}

InvokeResult ReflMngr::Invoke(
	ConstObjectPtr obj,
	StrID methodID,
	void* result_buffer,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer) const
{
	auto deref_prop = GetDereferenceProperty(obj.GetID());
	switch (deref_prop)
	{
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		return Invoke(Dereference(obj), methodID, result_buffer, argTypeIDs, argptr_buffer);
	case Ubpa::UDRefl::DereferenceProperty::Const:
		return Invoke(DereferenceAsConst(obj), methodID, result_buffer, argTypeIDs, argptr_buffer);
	default:
		break;
	}

	if (auto priority_rst = details::Invoke(true, &temporary_resource, obj, methodID, result_buffer, argTypeIDs, argptr_buffer))
		return priority_rst;

	return details::Invoke(false, &temporary_resource, obj, methodID, result_buffer, argTypeIDs, argptr_buffer);
}

InvokeResult ReflMngr::Invoke(
	ObjectPtr obj,
	StrID methodID,
	void* result_buffer,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer) const
{
	auto deref_prop = GetDereferenceProperty(obj.GetID());
	switch (deref_prop)
	{
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		return Invoke(Dereference(obj), methodID, result_buffer, argTypeIDs, argptr_buffer);
	case Ubpa::UDRefl::DereferenceProperty::Const:
		return Invoke(DereferenceAsConst(obj), methodID, result_buffer, argTypeIDs, argptr_buffer);
	default:
		break;
	}

	if (auto priority_rst = details::Invoke(true, &temporary_resource, obj, methodID, result_buffer, argTypeIDs, argptr_buffer))
		return priority_rst;

	return details::Invoke(false, &temporary_resource, obj, methodID, result_buffer, argTypeIDs, argptr_buffer);
}

SharedObject ReflMngr::MInvoke(
	TypeID typeID,
	StrID methodID,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer,
	std::pmr::memory_resource* rst_rsrc) const
{
	assert(rst_rsrc);
	if (GetDereferenceProperty(typeID) != DereferenceProperty::NotReference)
		return MInvoke(Dereference(typeID), methodID, argTypeIDs, argptr_buffer, rst_rsrc);

	if (auto priority_rst = details::MInvoke(true, &temporary_resource, typeID, methodID, argTypeIDs, argptr_buffer, rst_rsrc); priority_rst.Valid())
		return priority_rst;

	return details::MInvoke(false, &temporary_resource, typeID, methodID, argTypeIDs, argptr_buffer, rst_rsrc);
}

SharedObject ReflMngr::MInvoke(
	ConstObjectPtr obj,
	StrID methodID,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer,
	std::pmr::memory_resource* rst_rsrc) const
{
	assert(rst_rsrc);
	auto deref_prop = GetDereferenceProperty(obj.GetID());
	switch (deref_prop)
	{
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		return MInvoke(Dereference(obj), methodID, argTypeIDs, argptr_buffer, rst_rsrc);
	case Ubpa::UDRefl::DereferenceProperty::Const:
		return MInvoke(DereferenceAsConst(obj), methodID, argTypeIDs, argptr_buffer, rst_rsrc);
	default:
		break;
	}

	if (auto priority_rst = details::MInvoke(true, &temporary_resource, obj, methodID, argTypeIDs, argptr_buffer, rst_rsrc); priority_rst.Valid())
		return priority_rst;

	return details::MInvoke(false, &temporary_resource, obj, methodID, argTypeIDs, argptr_buffer, rst_rsrc);
}

SharedObject ReflMngr::MInvoke(
	ObjectPtr obj,
	StrID methodID,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer,
	std::pmr::memory_resource* rst_rsrc) const
{
	assert(rst_rsrc);
	auto deref_prop = GetDereferenceProperty(obj.GetID());
	switch (deref_prop)
	{
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		return MInvoke(Dereference(obj), methodID, argTypeIDs, argptr_buffer, rst_rsrc);
	case Ubpa::UDRefl::DereferenceProperty::Const:
		return MInvoke(DereferenceAsConst(obj), methodID, argTypeIDs, argptr_buffer, rst_rsrc);
	default:
		break;
	}

	if (auto priority_rst = details::MInvoke(true, &temporary_resource, obj, methodID, argTypeIDs, argptr_buffer, rst_rsrc); priority_rst.Valid())
		return priority_rst;

	return details::MInvoke(false, &temporary_resource, obj, methodID, argTypeIDs, argptr_buffer, rst_rsrc);
}

ObjectPtr ReflMngr::NonArgCopyMNew(TypeID typeID, std::pmr::memory_resource* rsrc, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	assert(rsrc);

	if (!IsConstructible(typeID, argTypeIDs))
		return nullptr;

	const auto& typeinfo = typeinfos.at(typeID);

	void* buffer = rsrc->allocate(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectPtr obj{ typeID, buffer };
	bool success = NonArgConstruct(obj, argTypeIDs, argptr_buffer);
	assert(success);

	return obj;
}

ObjectPtr ReflMngr::MNew(TypeID typeID, std::pmr::memory_resource* rsrc, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	assert(rsrc);

	if (!IsConstructible(typeID, argTypeIDs))
		return nullptr;

	const auto& typeinfo = typeinfos.at(typeID);

	void* buffer = rsrc->allocate(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectPtr obj{ typeID, buffer };
	bool success = Construct(obj, argTypeIDs, argptr_buffer);
	assert(success);

	return obj;
}

bool ReflMngr::MDelete(ConstObjectPtr obj, std::pmr::memory_resource* rsrc) const {
	assert(rsrc);

	bool dtor_success = Destruct(obj);
	if (!dtor_success)
		return false;

	const auto& typeinfo = typeinfos.at(obj.GetID());

	rsrc->deallocate(ConstCast(obj).GetPtr(), typeinfo.size, typeinfo.alignment);

	return true;
}

bool ReflMngr::IsNonArgConstructible(TypeID typeID, std::span<const TypeID> argTypeIDs) const {
	assert(GetDereferenceProperty(typeID) == DereferenceProperty::NotReference);

	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto ctorID = StrIDRegistry::MetaID::ctor;
	auto mtarget = typeinfo.methodinfos.find(ctorID);
	size_t num = typeinfo.methodinfos.count(ctorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (details::IsNonArgConstructCompatible(mtarget->second.methodptr.GetParamList(), argTypeIDs))
			return true;
	}
	return false;
}

bool ReflMngr::IsConstructible(TypeID typeID, std::span<const TypeID> argTypeIDs) const {
	assert(GetDereferenceProperty(typeID) == DereferenceProperty::NotReference);

	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto ctorID = StrIDRegistry::MetaID::ctor;
	auto mtarget = typeinfo.methodinfos.find(ctorID);
	size_t num = typeinfo.methodinfos.count(ctorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (IsCompatible(mtarget->second.methodptr.GetParamList(), argTypeIDs))
			return true;
	}
	return false;
}

bool ReflMngr::IsCopyConstructible(TypeID typeID) const {
	std::array argTypeIDs = { TypeID{ type_name_add_const_lvalue_reference_hash(tregistry.Nameof(typeID)) } };
	return IsNonArgConstructible(typeID, argTypeIDs);
}

bool ReflMngr::IsMoveConstructible(TypeID typeID) const {
	std::array argTypeIDs = { TypeID{ type_name_add_rvalue_reference_hash(tregistry.Nameof(typeID)) } };
	return IsNonArgConstructible(typeID, argTypeIDs);
}

bool ReflMngr::IsDestructible(TypeID typeID) const {
	assert(GetDereferenceProperty(typeID) == DereferenceProperty::NotReference);

	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto dtorID = StrIDRegistry::MetaID::dtor;

	auto mtarget = typeinfo.methodinfos.find(dtorID);
	size_t num = typeinfo.methodinfos.count(dtorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (!mtarget->second.methodptr.IsMemberVariable()
			&& IsCompatible(mtarget->second.methodptr.GetParamList(), {}))
			return true;
	}
	return false;
}

bool ReflMngr::NonArgConstruct(ObjectPtr obj, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	assert(GetDereferenceProperty(obj.GetID()) == DereferenceProperty::NotReference);

	if (!obj.Valid())
		return false;

	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto ctorID = StrIDRegistry::MetaID::ctor;
	auto mtarget = typeinfo.methodinfos.find(ctorID);
	size_t num = typeinfo.methodinfos.count(ctorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.IsMemberVariable()
			&& details::IsNonArgConstructCompatible(mtarget->second.methodptr.GetParamList(), argTypeIDs))
		{
			mtarget->second.methodptr.Invoke(obj.GetPtr(), nullptr, argptr_buffer);
			return true;
		}
	}
	return false;
}

bool ReflMngr::Construct(ObjectPtr obj, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	assert(GetDereferenceProperty(obj.GetID()) == DereferenceProperty::NotReference);

	if (!obj.Valid())
		return false;

	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto ctorID = StrIDRegistry::MetaID::ctor;
	auto mtarget = typeinfo.methodinfos.find(ctorID);
	size_t num = typeinfo.methodinfos.count(ctorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.IsMemberVariable()) {
			details::NewArgsGuard guard{
				false, &temporary_resource,
				mtarget->second.methodptr.GetParamList(), argTypeIDs, argptr_buffer
			};
			if (!guard.IsCompatible())
				continue;
			mtarget->second.methodptr.Invoke(obj.GetPtr(), nullptr, guard.GetArgPtrBuffer());
			return true;
		}
	}
	return false;
}

bool ReflMngr::Destruct(ConstObjectPtr obj) const {
	assert(GetDereferenceProperty(obj.GetID()) == DereferenceProperty::NotReference);

	if (!obj.Valid())
		return false;

	auto target = typeinfos.find(obj.GetID());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto dtorID = StrIDRegistry::MetaID::dtor;
	auto mtarget = typeinfo.methodinfos.find(dtorID);
	size_t num = typeinfo.methodinfos.count(dtorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.IsMemberConst()
			&& IsCompatible(mtarget->second.methodptr.GetParamList(), {}))
		{
			mtarget->second.methodptr.Invoke(obj.GetPtr(), nullptr, {});
			return true;
		}
	}
	return false;
}

void ReflMngr::ForEachTypeID(TypeID typeID, const std::function<bool(TypeID)>& func) const {
	std::set<TypeID> visitedVBs;
	details::ForEachTypeID(Dereference(typeID), func, visitedVBs);
}

void ReflMngr::ForEachType(TypeID typeID, const std::function<bool(TypeRef)>& func) const {
	std::set<TypeID> visitedVBs;
	details::ForEachTypeInfo(Dereference(typeID), func, visitedVBs);
}

void ReflMngr::ForEachField(
	TypeID typeID,
	const std::function<bool(TypeRef, FieldRef)>& func) const
{
	ForEachType(typeID, [&func](TypeRef type) {
		for (auto& [fieldID, fieldInfo] : type.info.fieldinfos) {
			if (!func(type, { fieldID, fieldInfo }))
				return false;
		}
		return true;
	});
}

void ReflMngr::ForEachMethod(
	TypeID typeID,
	const std::function<bool(TypeRef, MethodRef)>& func) const
{
	ForEachType(typeID, [&func](TypeRef type) {
		for (auto& [methodID, methodInfo] : type.info.methodinfos) {
			if (!func(type, { methodID, methodInfo }))
				return false;
		}
		return true;
	});
}

void ReflMngr::ForEachRWVar(
	TypeID typeID,
	const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	details::ForEachRWVar(Dereference(typeID), func, visitedVBs);
}

void ReflMngr::ForEachRVar(
	TypeID typeID,
	const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	details::ForEachRVar(Dereference(typeID), func, visitedVBs);
}

void ReflMngr::ForEachRWVar(
	ObjectPtr obj,
	const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	switch (GetDereferenceProperty(obj.GetID()))
	{
	case Ubpa::UDRefl::DereferenceProperty::NotReference:
		details::ForEachRWVar(obj, func, visitedVBs);
		break;
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		details::ForEachRWVar(Dereference(obj), func, visitedVBs);
		break;
	case Ubpa::UDRefl::DereferenceProperty::Const:
	default:
		break;
	}
}

void ReflMngr::ForEachRVar(
	ConstObjectPtr obj,
	const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const
{
	std::set<TypeID> visitedVBs;
	switch (GetDereferenceProperty(obj.GetID()))
	{
	case Ubpa::UDRefl::DereferenceProperty::NotReference:
		details::ForEachRVar(obj, func, visitedVBs);
		break;
	case Ubpa::UDRefl::DereferenceProperty::Const:
	case Ubpa::UDRefl::DereferenceProperty::Variable:
		details::ForEachRVar(DereferenceAsConst(obj), func, visitedVBs);
		break;
	default:
		break;
	}
}

void ReflMngr::ForEachRWOwnedVar(
	ObjectPtr obj,
	const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const
{
	ForEachRWVar(obj, [func](TypeRef type, FieldRef field, ObjectPtr obj) {
		if (field.info.fieldptr.IsUnowned())
			return true;

		return func(type, field, obj);
	});
}

void ReflMngr::ForEachROwnedVar(
	ConstObjectPtr obj,
	const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const
{
	ForEachRVar(obj, [func](TypeRef type, FieldRef field, ConstObjectPtr obj) {
		if (field.info.fieldptr.IsUnowned())
			return true;

		return func(type, field, obj);
	});
}

std::vector<TypeID> ReflMngr::GetTypeIDs(TypeID typeID) {
	std::vector<TypeID> rst;
	ForEachTypeID(typeID, [&rst](TypeID typeID) {
		rst.push_back(typeID);
		return true;
	});
	return rst;
}

std::vector<TypeRef> ReflMngr::GetTypes(TypeID typeID) {
	std::vector<TypeRef> rst;
	ForEachType(typeID, [&rst](TypeRef type) {
		rst.push_back(type);
		return true;
	});
	return rst;
}

std::vector<TypeFieldRef> ReflMngr::GetTypeFields(TypeID typeID) {
	std::vector<TypeFieldRef> rst;
	ForEachField(typeID, [&rst](TypeRef type, FieldRef field) {
		rst.emplace_back(TypeFieldRef{ type, field });
		return true;
	});
	return rst;
}

std::vector<FieldRef> ReflMngr::GetFields(TypeID typeID) {
	std::vector<FieldRef> rst;
	ForEachField(typeID, [&rst](TypeRef type, FieldRef field) {
		rst.push_back(field);
		return true;
	});
	return rst;
}

std::vector<TypeMethodRef> ReflMngr::GetTypeMethods(TypeID typeID) {
	std::vector<TypeMethodRef> rst;
	ForEachMethod(typeID, [&rst](TypeRef type, MethodRef field) {
		rst.emplace_back(TypeMethodRef{ type, field });
		return true;
	});
	return rst;
}

std::vector<MethodRef> ReflMngr::GetMethods(TypeID typeID) {
	std::vector<MethodRef> rst;
	ForEachMethod(typeID, [&rst](TypeRef type, MethodRef field) {
		rst.push_back(field);
		return true;
	});
	return rst;
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> ReflMngr::GetTypeFieldRWVars(TypeID typeID) {
	std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> rst;
	ForEachRWVar(typeID, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::vector<ObjectPtr> ReflMngr::GetRWVars(TypeID typeID) {
	std::vector<ObjectPtr> rst;
	ForEachRWVar(typeID, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}

std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> ReflMngr::GetTypeFieldRVars(TypeID typeID) {
	std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> rst;
	ForEachRVar(typeID, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::vector<ConstObjectPtr> ReflMngr::GetRVars(TypeID typeID) {
	std::vector<ConstObjectPtr> rst;
	ForEachRVar(typeID, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> ReflMngr::GetTypeFieldRWVars(ObjectPtr obj) {
	std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> rst;
	ForEachRWVar(obj, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::vector<ObjectPtr> ReflMngr::GetRWVars(ObjectPtr obj) {
	std::vector<ObjectPtr> rst;
	ForEachRWVar(obj, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}


std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> ReflMngr::GetTypeFieldRVars(ConstObjectPtr obj) {
	std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> rst;
	ForEachRVar(obj, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::vector<ConstObjectPtr> ReflMngr::GetRVars(ConstObjectPtr obj) {
	std::vector<ConstObjectPtr> rst;
	ForEachRVar(obj, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> ReflMngr::GetTypeFieldRWOwnedVars(ObjectPtr obj) {
	std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>> rst;
	ForEachRWVar(obj, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		if (field.info.fieldptr.IsOwned())
			rst.emplace_back(std::tuple{ type, field, var });

		return true;
	});
	return rst;
}

std::vector<ObjectPtr> ReflMngr::GetRWOwnedVars(ObjectPtr obj) {
	std::vector<ObjectPtr> rst;
	ForEachRWVar(obj, [&rst](TypeRef type, FieldRef field, ObjectPtr var) {
		if (field.info.fieldptr.IsOwned())
			rst.push_back(var);
		return true;
	});
	return rst;
}

std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> ReflMngr::GetTypeFieldROwnedVars(ConstObjectPtr obj) {
	std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> rst;
	ForEachRVar(obj, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		if (field.info.fieldptr.IsOwned())
			rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::vector<ConstObjectPtr> ReflMngr::GetROwnedVars(ConstObjectPtr obj) {
	std::vector<ConstObjectPtr> rst;
	ForEachRVar(obj, [&rst](TypeRef type, FieldRef field, ConstObjectPtr var) {
		if (field.info.fieldptr.IsOwned())
			rst.emplace_back(var);
		return true;
	});
	return rst;
}

std::optional<TypeID> ReflMngr::FindTypeID(TypeID typeID, const std::function<bool(TypeID)>& func) const {
	std::optional<TypeID> rst;
	ForEachTypeID(typeID, [&rst, func](TypeID typeID) {
		if (!func(typeID))
			return true;

		rst.emplace(typeID);
		return false; // stop
	});
	return rst;
}

std::optional<TypeRef> ReflMngr::FindType(TypeID typeID, const std::function<bool(TypeRef)>& func) const {
	std::optional<TypeRef> rst;
	ForEachType(typeID, [&rst, func](TypeRef type) {
		if (!func(type))
			return true;

		rst.emplace(type);
		return false; // stop
	});
	return rst;
}

std::optional<FieldRef> ReflMngr::FindField(TypeID typeID, const std::function<bool(FieldRef)>& func) const {
	std::optional<FieldRef> rst;
	ForEachField(typeID, [&rst, func](TypeRef type, FieldRef field) {
		if (!func(field))
			return true;

		rst.emplace(field);
		return false; // stop
	});
	return rst;
}

std::optional<MethodRef> ReflMngr::FindMethod(TypeID typeID, const std::function<bool(MethodRef)>& func) const {
	std::optional<MethodRef> rst;
	ForEachMethod(typeID, [&rst, func](TypeRef type, MethodRef method) {
		if (!func(method))
			return true;

		rst.emplace(method);
		return false; // stop
	});
	return rst;
}

ObjectPtr ReflMngr::FindRWVar(TypeID typeID, const std::function<bool(ObjectPtr)>& func) const {
	ObjectPtr rst;
	ForEachRWVar(typeID, [&rst, func](TypeRef type, FieldRef field, ObjectPtr obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

ConstObjectPtr ReflMngr::FindRVar(TypeID typeID, const std::function<bool(ConstObjectPtr)>& func) const {
	ConstObjectPtr rst;
	ForEachRVar(typeID, [&rst, func](TypeRef type, FieldRef field, ConstObjectPtr obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

ObjectPtr ReflMngr::FindRWVar(ObjectPtr obj, const std::function<bool(ObjectPtr)>& func) const {
	ObjectPtr rst;
	ForEachRWVar(obj, [&rst, func](TypeRef type, FieldRef field, ObjectPtr obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

ConstObjectPtr ReflMngr::FindRVar(ConstObjectPtr obj, const std::function<bool(ConstObjectPtr)>& func) const {
	ConstObjectPtr rst;
	ForEachRVar(obj, [&rst, func](TypeRef type, FieldRef field, ConstObjectPtr obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

ObjectPtr ReflMngr::FindRWOwnedVar(ObjectPtr obj, const std::function<bool(ObjectPtr)>& func) const {
	ObjectPtr rst;
	ForEachRWVar(obj, [&rst, func](TypeRef type, FieldRef field, ObjectPtr obj) {
		if (field.info.fieldptr.IsUnowned() || !func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

ConstObjectPtr ReflMngr::FindROwnedVar(ConstObjectPtr obj, const std::function<bool(ConstObjectPtr)>& func) const {
	ConstObjectPtr rst;
	ForEachRVar(obj, [&rst, func](TypeRef type, FieldRef field, ConstObjectPtr obj) {
		if (field.info.fieldptr.IsUnowned() || !func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

DereferenceProperty ReflMngr::GetDereferenceProperty(TypeID ID) const {
	auto name = tregistry.Nameof(ID);

	if (!type_name_is_reference(name))
		return DereferenceProperty::NotReference;

	auto unref_name = type_name_remove_reference(name);

	return type_name_is_const(unref_name) ? DereferenceProperty::Const : DereferenceProperty::Variable;
}

TypeID ReflMngr::Dereference(TypeID ID) const {
	auto name = tregistry.Nameof(ID);

	if (!type_name_is_reference(name))
		return ID;

	std::string_view ele_name = type_name_remove_reference(name);

	auto rst_name = type_name_remove_cv(ele_name);

	return rst_name;
}

ObjectPtr ReflMngr::Dereference(ConstObjectPtr ref_obj) const {
	if (!ref_obj.GetPtr())
		return nullptr;

	auto name = tregistry.Nameof(ref_obj.GetID());

	if (!type_name_is_reference(name))
		return nullptr;

	std::string_view rst_name = type_name_remove_reference(name);

	if (type_name_is_const(rst_name))
		return nullptr;

	assert(!type_name_is_volatile(rst_name));

	return { TypeID{ rst_name }, const_cast<void*>(ref_obj.GetPtr()) };
}

ConstObjectPtr ReflMngr::DereferenceAsConst(ConstObjectPtr ref_obj) const {
	if (!ref_obj.Valid())
		return nullptr;

	auto name = tregistry.Nameof(ref_obj.GetID());

	if (!type_name_is_reference(name))
		return ref_obj;

	std::string_view ele_name = type_name_remove_reference(name);

	auto rst_name = type_name_remove_const(ele_name);
	assert(!type_name_is_volatile(rst_name));

	return { TypeID{ rst_name }, ref_obj.GetPtr() };
}

TypeID ReflMngr::AddLValueReference(TypeID ID) {
	return tregistry.RegisterAddLValueReference(ID);
}

TypeID ReflMngr::AddRValueReference(TypeID ID) {
	return tregistry.RegisterAddRValueReference(ID);
}

TypeID ReflMngr::AddConstLValueReference(TypeID ID) {
	return tregistry.RegisterAddConstLValueReference(ID);
}

TypeID ReflMngr::AddConstRValueReference(TypeID ID) {
	return tregistry.RegisterAddConstRValueReference(ID);
}

ObjectPtr ReflMngr::AddLValueReference(ObjectPtr obj) {
	TypeID newID = tregistry.RegisterAddLValueReference(obj.GetID());
	if (!newID.Valid())
		return {};

	return { newID, obj.GetPtr() };
}

ObjectPtr ReflMngr::AddRValueReference(ObjectPtr obj) {
	TypeID newID = tregistry.RegisterAddRValueReference(obj.GetID());
	if (!newID.Valid())
		return {};

	return { newID, obj.GetPtr() };
}

ConstObjectPtr ReflMngr::AddConstLValueReference(ConstObjectPtr obj) {
	TypeID newID = tregistry.RegisterAddConstLValueReference(obj.GetID());
	if (!newID.Valid())
		return {};

	return { newID, const_cast<void*>(obj.GetPtr()) };
}

ConstObjectPtr ReflMngr::AddConstRValueReference(ConstObjectPtr obj) {
	TypeID newID = tregistry.RegisterAddConstRValueReference(obj.GetID());
	if (!newID.Valid())
		return {};

	return { newID, const_cast<void*>(obj.GetPtr()) };
}

bool ReflMngr::ContainsBase(TypeID typeID, TypeID baseID) const {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;

	const auto& info = target->second;
	if (info.baseinfos.contains(baseID))
		return true;

	for (const auto& [ID, baseinfo] : info.baseinfos) {
		bool found = ContainsBase(ID, baseID);
		if (found)
			return true;
	}
	return false;
}

bool ReflMngr::ContainsField(TypeID typeID, StrID fieldID) const {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;

	const auto& info = target->second;
	if (info.fieldinfos.contains(fieldID))
		return true;

	for (const auto& [ID, baseinfo] : info.baseinfos) {
		bool found = ContainsField(ID, fieldID);
		if (found)
			return true;
	}
	return false;
}

bool ReflMngr::ContainsRWField(TypeID typeID, StrID fieldID) const {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;

	const auto& info = target->second;
	auto ftarget = info.fieldinfos.find(fieldID);
	if (ftarget != info.fieldinfos.end() && ftarget->second.fieldptr.IsVariable())
		return true;

	for (const auto& [ID, baseinfo] : info.baseinfos) {
		bool found = ContainsField(ID, fieldID);
		if (found)
			return true;
	}
	return false;
}

bool ReflMngr::ContainsMethod(TypeID typeID, StrID methodID) const {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;

	const auto& info = target->second;
	if (info.methodinfos.contains(methodID))
		return true;

	for (const auto& [ID, baseinfo] : info.baseinfos) {
		bool found = ContainsMethod(ID, methodID);
		if (found)
			return true;
	}
	return false;
}

bool ReflMngr::ContainsVariableMethod(TypeID typeID, StrID methodID) const {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;

	const auto& info = target->second;
	auto begin_end = info.methodinfos.equal_range(methodID);
	for (auto iter = begin_end.first; iter != begin_end.second; ++iter) {
		if (iter->second.methodptr.IsMemberVariable())
			return true;
	}

	for (const auto& [ID, baseinfo] : info.baseinfos) {
		bool found = ContainsMethod(ID, methodID);
		if (found)
			return true;
	}
	return false;
}

bool ReflMngr::ContainsConstMethod(TypeID typeID, StrID methodID) const {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;

	const auto& info = target->second;
	auto begin_end = info.methodinfos.equal_range(methodID);
	for (auto iter = begin_end.first; iter != begin_end.second; ++iter) {
		if (iter->second.methodptr.IsMemberConst())
			return true;
	}

	for (const auto& [ID, baseinfo] : info.baseinfos) {
		bool found = ContainsMethod(ID, methodID);
		if (found)
			return true;
	}
	return false;
}

bool ReflMngr::ContainsStaticMethod(TypeID typeID, StrID methodID) const {
	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return false;

	const auto& info = target->second;
	auto begin_end = info.methodinfos.equal_range(methodID);
	for (auto iter = begin_end.first; iter != begin_end.second; ++iter) {
		if (iter->second.methodptr.IsStatic())
			return true;
	}

	for (const auto& [ID, baseinfo] : info.baseinfos) {
		bool found = ContainsMethod(ID, methodID);
		if (found)
			return true;
	}
	return false;
}
