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

			auto lhs = Mngr.tregistry.Nameof(params[i]);

#ifndef NDEBUG
			// because rhs(arg)'s ID maybe have no name in the registry
			// so we use type_name_add_*_hash(...) to avoid it
			auto rhs = Mngr.tregistry.Nameof(argTypeIDs[i]);
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

			auto lhs = Mngr.tregistry.Nameof(params[i]);
#ifndef NDEBUG
			// because rhs(arg)'s ID maybe have no name in the registry
			// so we use type_name_add_*_hash(...) to avoid it
			auto rhs = Mngr.tregistry.Nameof(argTypeIDs[i]);
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

				auto lhs = Mngr.tregistry.Nameof(paramTypeIDs[i]);
#ifndef NDEBUG
				auto rhs = Mngr.tregistry.Nameof(argTypeIDs[i]);
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

						if (Mngr.IsNonArgConstructible(raw_lhs_ID, std::span<const TypeID>{&argTypeIDs[i], 1})) {
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

						if (Mngr.IsNonArgConstructible(raw_lhs_ID, std::span<const TypeID>{&argTypeIDs[i], 1})) {
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

					if ((type_name_is_pointer(lhs) || Mngr.IsCopyConstructible(lhs))
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

					if (Mngr.IsNonArgConstructible(paramTypeIDs[i], std::span<const TypeID>{&argTypeIDs[i], 1})) {
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
					const auto& typeinfo = Mngr.typeinfos.at(TypeID{ info_copiedargs[k].typeID });
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
					bool success = Mngr.NonArgConstruct(
						ObjectView{ TypeID{ info_copiedargs[idx_copiedargs].typeID }, arg_buffer },
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
					bool success = Mngr.Destruct({ TypeID{ info.typeID }, argptr_buffer[info.idx] });
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
		assert(Mngr.GetConstReferenceMode(typeID) == ConstReferenceMode::None);
		auto typetarget = Mngr.typeinfos.find(typeID);

		if (typetarget == Mngr.typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (iter->second.methodptr.IsStatic()
				&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)
					: Mngr.IsCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)))
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
		assert(Mngr.GetConstReferenceMode(typeID) == ConstReferenceMode::None);
		auto typetarget = Mngr.typeinfos.find(typeID);

		if (typetarget == Mngr.typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (!iter->second.methodptr.IsMemberVariable()
				&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)
					: Mngr.IsCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)))
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
		assert(Mngr.GetConstReferenceMode(typeID) == ConstReferenceMode::None);
		auto typetarget = Mngr.typeinfos.find(typeID);

		if (typetarget == Mngr.typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;
		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		{ // 1. object variable and static
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (!iter->second.methodptr.IsMemberConst()
					&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)
						: Mngr.IsCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)))
				{
					return { true, iter->second.methodptr.GetResultDesc() };
				}
			}
		}

		{ // 2. object const
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (iter->second.methodptr.IsMemberConst()
					&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)
						: Mngr.IsCompatible(iter->second.methodptr.GetParamList(), argTypeIDs)))
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
		assert(Mngr.GetConstReferenceMode(typeID) == ConstReferenceMode::None);

		auto typetarget = Mngr.typeinfos.find(typeID);

		if (typetarget == Mngr.typeinfos.end())
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
		ObjectView obj,
		bool is_const,
		StrID methodID,
		void* result_buffer,
		std::span<const TypeID> argTypeIDs,
		ArgPtrBuffer argptr_buffer)
	{
		assert(Mngr.GetConstReferenceMode(obj.GetTypeID()) == ConstReferenceMode::None);

		auto typetarget = Mngr.typeinfos.find(obj.GetTypeID());

		if (typetarget == Mngr.typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		if (!is_const) {
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
						iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, guard.GetArgPtrBuffer())
					};
				}
			}
		}
		for (auto iter = begin_iter; iter != end_iter; ++iter) {
			if (!iter->second.methodptr.IsMemberVariable()) {
				NewArgsGuard guard{
					is_priority, args_rsrc,
					iter->second.methodptr.GetParamList(), argTypeIDs, argptr_buffer
				};
				if (!guard.IsCompatible())
					continue;
				return {
					true,
					iter->second.methodptr.GetResultDesc().typeID,
					iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), result_buffer, guard.GetArgPtrBuffer())
				};
			}
		}


		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			auto rst = Invoke(
				is_priority, args_rsrc,
				ObjectView{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, is_const,
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
		assert(Mngr.GetConstReferenceMode(typeID) == ConstReferenceMode::None);

		auto typetarget = Mngr.typeinfos.find(typeID);

		if (typetarget == Mngr.typeinfos.end())
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
					return SharedObject{ rst_desc.typeID };
				}
				else if (type_name_is_reference(Mngr.tregistry.Nameof(rst_desc.typeID))) {
					std::aligned_storage_t<sizeof(void*)> buffer;
					iter->second.methodptr.Invoke(&buffer, guard.GetArgPtrBuffer());
					return { rst_desc.typeID, buffer_as<void*>(&buffer) };
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
			if (rst.GetTypeID())
				return rst;
		}

		return nullptr;
	}

	static SharedObject MInvoke(
		bool is_priority,
		std::pmr::memory_resource* args_rsrc,
		ObjectView obj,
		bool is_const,
		StrID methodID,
		std::span<const TypeID> argTypeIDs,
		ArgPtrBuffer argptr_buffer,
		std::pmr::memory_resource* rst_rsrc)
	{
		assert(rst_rsrc);
		assert(Mngr.GetConstReferenceMode(obj.GetTypeID()) == ConstReferenceMode::None);
		auto typetarget = Mngr.typeinfos.find(obj.GetTypeID());

		if (typetarget == Mngr.typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(methodID);

		if (!is_const) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (iter->second.methodptr.IsMemberVariable()) {
					NewArgsGuard guard{
						is_priority, args_rsrc,
						iter->second.methodptr.GetParamList(), argTypeIDs, argptr_buffer
					};

					if (!guard.IsCompatible())
						continue;

					const auto& methodptr = iter->second.methodptr;
					const auto& rst_desc = methodptr.GetResultDesc();

					if (rst_desc.IsVoid()) {
						iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, argptr_buffer);
						return SharedObject{ rst_desc.typeID };
					}
					else if (type_name_is_reference(Mngr.tregistry.Nameof(rst_desc.typeID))) {
						std::aligned_storage_t<sizeof(void*)> buffer;
						iter->second.methodptr.Invoke(obj.GetPtr(), &buffer, guard.GetArgPtrBuffer());
						return { rst_desc.typeID, buffer_as<void*>(&buffer) };
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
					iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), nullptr, argptr_buffer);
					return SharedObject{ rst_desc.typeID };
				}
				else if (type_name_is_reference(Mngr.tregistry.Nameof(rst_desc.typeID))) {
					std::aligned_storage_t<sizeof(void*)> buffer;
					iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), &buffer, guard.GetArgPtrBuffer());
					return { rst_desc.typeID, buffer_as<void*>(&buffer) };
				}
				else {
					void* result_buffer = rst_rsrc->allocate(rst_desc.size, rst_desc.alignment);
					auto dtor = iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), result_buffer, guard.GetArgPtrBuffer());
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
				ObjectView{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, is_const,
				methodID, argTypeIDs, argptr_buffer, rst_rsrc
			);
			if (rst.GetTypeID())
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

		auto target = Mngr.typeinfos.find(typeID);

		if (target == Mngr.typeinfos.end())
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
		auto target = Mngr.typeinfos.find(typeID);

		if (target == Mngr.typeinfos.end())
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

	static bool ForEachVar(
		TypeID typeID,
		const std::function<bool(TypeRef, FieldRef, ObjectView)>& func,
		std::set<TypeID>& visitedVBs)
	{
		auto target = Mngr.typeinfos.find(typeID);

		if (target == Mngr.typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
			if (fieldInfo.fieldptr.IsUnowned()) {
				if (!func({ typeID, typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.Var()))
					return false;
			}
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachVar(baseID, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachVar(
		ObjectView obj,
		const std::function<bool(TypeRef, FieldRef, ObjectView)>& func,
		std::set<TypeID>& visitedVBs)
	{
		assert(Mngr.GetConstReferenceMode(obj.GetTypeID()) == ConstReferenceMode::None);

		auto target = Mngr.typeinfos.find(obj.GetTypeID());

		if (target == Mngr.typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [fieldID, fieldInfo] : typeinfo.fieldinfos) {
			if (!func({ obj.GetTypeID(), typeinfo }, { fieldID, fieldInfo }, fieldInfo.fieldptr.Var(obj.GetPtr())))
				return false;
		}

		for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(baseID) != visitedVBs.end())
					continue;
				visitedVBs.insert(baseID);
			}

			if (!ForEachVar(ObjectView{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, func, visitedVBs))
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

TypeInfo* ReflMngr::GetTypeInfo(TypeID ID) {
	auto name = tregistry.Nameof(ID);
	auto raw_name = type_name_remove_cvref(name);
	TypeID rawID{ raw_name };
	auto target = typeinfos.find(rawID);
	if (target == typeinfos.end())
		return nullptr;
	return &target->second;
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
	temporary_resource.release();
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

ObjectView ReflMngr::NonArgCopyNew(TypeID typeID, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	if (!IsNonArgConstructible(typeID, argTypeIDs))
		return nullptr;

	const auto& typeinfo = typeinfos.at(typeID);

	void* buffer = typeinfo.alignment <= std::alignment_of_v<std::max_align_t> ?
		Malloc(typeinfo.size)
		: AlignedMalloc(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectView obj{ typeID, buffer };
	bool success = NonArgConstruct(obj, argTypeIDs, argptr_buffer);
	assert(success);

	return obj;
}

ObjectView ReflMngr::New(TypeID typeID, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	if (!IsConstructible(typeID, argTypeIDs))
		return nullptr;

	const auto& typeinfo = typeinfos.at(typeID);

	void* buffer = typeinfo.alignment <= std::alignment_of_v<std::max_align_t> ?
		Malloc(typeinfo.size)
		: AlignedMalloc(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectView obj{ typeID, buffer };
	bool success = Construct(obj, argTypeIDs, argptr_buffer);
	assert(success);
	
	return obj;
}

bool ReflMngr::Delete(ObjectView obj) const {
	bool dtor_success = Destruct(obj);
	if (!dtor_success)
		return false;

	const auto& typeinfo = typeinfos.at(obj.GetTypeID());

	bool free_success;
	if (typeinfo.alignment <= std::alignment_of_v<std::max_align_t>)
		free_success = Free(const_cast<void*>(obj.GetPtr()));
	else
		free_success = AlignedFree(const_cast<void*>(obj.GetPtr()));

	return free_success;
}

SharedObject ReflMngr::MakeShared(TypeID typeID, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	ObjectView obj = New(typeID, argTypeIDs, argptr_buffer);

	if (!obj.Valid())
		return nullptr;

	return { obj, [typeID](void* ptr) {
		bool success = Mngr.Delete({typeID, ptr});
		assert(success);
	} };
}

ObjectView ReflMngr::StaticCast_DerivedToBase(ObjectView obj, TypeID typeID) const {
	assert(typeID);

	switch (GetConstReferenceMode(obj.GetTypeID()))
	{
	case ConstReferenceMode::Left:
		return StaticCast_DerivedToBase(obj.RemoveReference(), typeID).AddLValueReference();
	case ConstReferenceMode::Right:
		return StaticCast_DerivedToBase(obj.RemoveReference(), typeID).AddRValueReference();
	case ConstReferenceMode::Const:
		return StaticCast_DerivedToBase(obj.RemoveConst(), typeID).AddConst();
	case ConstReferenceMode::ConstLeft:
		return StaticCast_DerivedToBase(obj.RemoveConstReference(), typeID).AddConstLValueReference();
	case ConstReferenceMode::ConstRight:
		return StaticCast_DerivedToBase(obj.RemoveConstReference(), typeID).AddConstRValueReference();
	case ConstReferenceMode::None:
	default:
		break;
	}

	if (obj.GetTypeID() == typeID)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { typeID, nullptr };

	auto target = typeinfos.find(obj.GetTypeID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto ptr = StaticCast_DerivedToBase(ObjectView{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, typeID);
		if (ptr.GetTypeID())
			return ptr;
	}

	return nullptr;
}

ObjectView ReflMngr::StaticCast_BaseToDerived(ObjectView obj, TypeID typeID) const {
	assert(typeID);
	
	switch (GetConstReferenceMode(obj.GetTypeID()))
	{
	case ConstReferenceMode::Left:
		return StaticCast_BaseToDerived(obj.RemoveReference(), typeID).AddLValueReference();
	case ConstReferenceMode::Right:
		return StaticCast_BaseToDerived(obj.RemoveReference(), typeID).AddRValueReference();
	case ConstReferenceMode::Const:
		return StaticCast_BaseToDerived(obj.RemoveConst(), typeID).AddConst();
	case ConstReferenceMode::ConstLeft:
		return StaticCast_BaseToDerived(obj.RemoveConstReference(), typeID).AddConstLValueReference();
	case ConstReferenceMode::ConstRight:
		return StaticCast_BaseToDerived(obj.RemoveConstReference(), typeID).AddConstRValueReference();
	case ConstReferenceMode::None:
	default:
		break;
	}

	if (obj.GetTypeID() == typeID)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { typeID, nullptr };

	auto target = typeinfos.find(typeID);
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto ptr = StaticCast_BaseToDerived(obj, baseID);
		if (ptr.GetTypeID())
			return { baseID, baseinfo.IsVirtual() ? nullptr : baseinfo.StaticCast_BaseToDerived(obj.GetPtr()) };
	}
	
	return nullptr;
}

ObjectView ReflMngr::DynamicCast_BaseToDerived(ObjectView obj, TypeID typeID) const {
	assert(typeID);
	
	switch (GetConstReferenceMode(obj.GetTypeID()))
	{
	case ConstReferenceMode::Left:
		return DynamicCast_BaseToDerived(obj.RemoveReference(), typeID).AddLValueReference();
	case ConstReferenceMode::Right:
		return DynamicCast_BaseToDerived(obj.RemoveReference(), typeID).AddRValueReference();
	case ConstReferenceMode::Const:
		return DynamicCast_BaseToDerived(obj.RemoveConst(), typeID).AddConst();
	case ConstReferenceMode::ConstLeft:
		return DynamicCast_BaseToDerived(obj.RemoveConstReference(), typeID).AddConstLValueReference();
	case ConstReferenceMode::ConstRight:
		return DynamicCast_BaseToDerived(obj.RemoveConstReference(), typeID).AddConstRValueReference();
	case ConstReferenceMode::None:
	default:
		break;
	}

	if (IsConst(obj.GetTypeID()))
		return StaticCast_DerivedToBase(obj.RemoveReference(), typeID).AddConstLValueReference();

	if (obj.GetTypeID() == typeID)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { typeID, nullptr };

	auto target = typeinfos.find(obj.GetTypeID());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto ptr = DynamicCast_BaseToDerived(ObjectView{ baseID, baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) }, typeID);
		if (ptr.GetTypeID())
			return { baseID, baseinfo.IsPolymorphic() ? baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) : nullptr };
	}

	return nullptr;
}

ObjectView ReflMngr::StaticCast(ObjectView obj, TypeID typeID) const {
	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetTypeID())
		return ptr_d2b;

	auto ptr_b2d = StaticCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetTypeID())
		return ptr_b2d;

	return nullptr;
}

ObjectView ReflMngr::DynamicCast(ObjectView obj, TypeID typeID) const {
	auto ptr_b2d = DynamicCast_BaseToDerived(obj, typeID);
	if (ptr_b2d.GetTypeID())
		return ptr_b2d;

	auto ptr_d2b = StaticCast_DerivedToBase(obj, typeID);
	if (ptr_d2b.GetTypeID())
		return ptr_d2b;

	return nullptr;
}

ObjectView ReflMngr::Var(TypeID typeID, StrID fieldID) {
	switch (GetConstReferenceMode(typeID))
	{
	case ConstReferenceMode::Left:
		return Var(RemoveReference(typeID), fieldID).AddLValueReference();
	case ConstReferenceMode::Right:
		return Var(RemoveReference(typeID), fieldID).AddRValueReference();
	case ConstReferenceMode::Const:
		return Var(RemoveConst(typeID), fieldID).AddConst();
	case ConstReferenceMode::ConstLeft:
		return Var(RemoveConstReference(typeID), fieldID).AddConstLValueReference();
	case ConstReferenceMode::ConstRight:
		return Var(RemoveConstReference(typeID), fieldID).AddConstRValueReference();
	case ConstReferenceMode::None:
	default:
		break;
	}

	auto ttarget = typeinfos.find(typeID);
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;

	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end() && ftarget->second.fieldptr.IsUnowned())
		return ftarget->second.fieldptr.Var();

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = Var(baseID, fieldID);
		if (bptr.GetTypeID())
			return bptr;
	}

	return nullptr;
}

ObjectView ReflMngr::Var(ObjectView obj, StrID fieldID) {
	switch (GetConstReferenceMode(obj.GetTypeID()))
	{
	case ConstReferenceMode::Left:
		return Var(obj.RemoveReference(), fieldID).AddLValueReference();
	case ConstReferenceMode::Right:
		return Var(obj.RemoveReference(), fieldID).AddRValueReference();
	case ConstReferenceMode::Const:
		return Var(obj.RemoveConst(), fieldID).AddConst();
	case ConstReferenceMode::ConstLeft:
		return Var(obj.RemoveConstReference(), fieldID).AddConstLValueReference();
	case ConstReferenceMode::ConstRight:
		return Var(obj.RemoveConstReference(), fieldID).AddConstRValueReference();
	case ConstReferenceMode::None:
	default:
		break;
	}

	auto ttarget = typeinfos.find(obj.GetTypeID());
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;

	auto ftarget = typeinfo.fieldinfos.find(fieldID);
	if (ftarget != typeinfo.fieldinfos.end())
		return ftarget->second.fieldptr.Var(obj.GetPtr());

	for (const auto& [baseID, baseinfo] : typeinfo.baseinfos) {
		auto bptr = Var(ObjectView{ baseID, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, fieldID);
		if (bptr.GetTypeID())
			return bptr;
	}

	return nullptr;
}

ObjectView ReflMngr::Var(ObjectView obj, TypeID baseID, StrID fieldID) {
	auto base = StaticCast_DerivedToBase(obj, baseID);
	if (!base.GetTypeID())
		return nullptr;
	return Var(base, fieldID);
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
	switch (GetConstReferenceMode(typeID))
	{
	case ConstReferenceMode::Left:
		return IsStaticInvocable(RemoveReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::Right:
		return IsStaticInvocable(RemoveReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::Const:
		return IsStaticInvocable(RemoveConst(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::ConstLeft:
		return IsStaticInvocable(RemoveConstReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::ConstRight:
		return IsStaticInvocable(RemoveConstReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::None:
	default:
		break;
	}

	if (auto priority_rst = details::IsStaticInvocable(true, typeID, methodID, argTypeIDs))
		return priority_rst;

	return details::IsStaticInvocable(false, typeID, methodID, argTypeIDs);
}

InvocableResult ReflMngr::IsConstInvocable(
	TypeID typeID,
	StrID methodID,
	std::span<const TypeID> argTypeIDs) const
{
	switch (GetConstReferenceMode(typeID))
	{
	case ConstReferenceMode::Left:
		return IsConstInvocable(RemoveReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::Right:
		return IsConstInvocable(RemoveReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::Const:
		return IsConstInvocable(RemoveConst(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::ConstLeft:
		return IsConstInvocable(RemoveConstReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::ConstRight:
		return IsConstInvocable(RemoveConstReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::None:
	default:
		break;
	}

	if (auto priority_rst = details::IsConstInvocable(true, typeID, methodID, argTypeIDs))
		return priority_rst;

	return details::IsConstInvocable(false, typeID, methodID, argTypeIDs);
}

InvocableResult ReflMngr::IsInvocable(
	TypeID typeID,
	StrID methodID,
	std::span<const TypeID> argTypeIDs) const
{
	switch (GetConstReferenceMode(typeID))
	{
	case ConstReferenceMode::Left:
		return IsInvocable(RemoveReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::Right:
		return IsInvocable(RemoveReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::Const:
		return IsConstInvocable(RemoveConst(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::ConstLeft:
		return IsConstInvocable(RemoveConstReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::ConstRight:
		return IsConstInvocable(RemoveConstReference(typeID), methodID, argTypeIDs);
	case ConstReferenceMode::None:
	default:
		break;
	}

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
	TypeID rawTypeID;
	switch (GetConstReferenceMode(typeID))
	{
	case ConstReferenceMode::Left: [[fallthrough]];
	case ConstReferenceMode::Right:
		rawTypeID = RemoveReference(typeID);
		break;
	case ConstReferenceMode::Const:
		rawTypeID = RemoveConst(typeID);
		break;
	case ConstReferenceMode::ConstLeft: [[fallthrough]];
	case ConstReferenceMode::ConstRight:
		rawTypeID = RemoveConstReference(typeID);
		break;
	case ConstReferenceMode::None:
	default:
		rawTypeID = typeID;
		break;
	}

	if(auto priority_rst = details::Invoke(true, &temporary_resource, rawTypeID, methodID, result_buffer, argTypeIDs, argptr_buffer))
		return priority_rst;

	return details::Invoke(false, &temporary_resource, rawTypeID, methodID, result_buffer, argTypeIDs, argptr_buffer);
}

InvokeResult ReflMngr::Invoke(
	ObjectView obj,
	StrID methodID,
	void* result_buffer,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer) const
{
	ObjectView rawObj;
	bool is_const;
	switch (GetConstReferenceMode(obj.GetTypeID()))
	{
	case ConstReferenceMode::Left: [[fallthrough]];
	case ConstReferenceMode::Right:
		rawObj = obj.RemoveReference();
		is_const = false;
		break;
	case ConstReferenceMode::Const:
		rawObj = obj.RemoveConst();
		is_const = true;
		break;
	case ConstReferenceMode::ConstLeft: [[fallthrough]];
	case ConstReferenceMode::ConstRight:
		rawObj = obj.RemoveConstReference();
		is_const = true;
		break;
	case ConstReferenceMode::None:
	default:
		rawObj = obj;
		is_const = false;
		break;
	}

	if (auto priority_rst = details::Invoke(true, &temporary_resource, rawObj, is_const, methodID, result_buffer, argTypeIDs, argptr_buffer))
		return priority_rst;

	return details::Invoke(false, &temporary_resource, rawObj, is_const, methodID, result_buffer, argTypeIDs, argptr_buffer);
}

SharedObject ReflMngr::MInvoke(
	TypeID typeID,
	StrID methodID,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer,
	std::pmr::memory_resource* rst_rsrc) const
{
	assert(rst_rsrc);

	TypeID rawTypeID;
	switch (GetConstReferenceMode(typeID))
	{
	case ConstReferenceMode::Left: [[fallthrough]];
	case ConstReferenceMode::Right:
		rawTypeID = RemoveReference(typeID);
		break;
	case ConstReferenceMode::Const:
		rawTypeID = RemoveConst(typeID);
		break;
	case ConstReferenceMode::ConstLeft: [[fallthrough]];
	case ConstReferenceMode::ConstRight:
		rawTypeID = RemoveConstReference(typeID);
		break;
	case ConstReferenceMode::None:
	default:
		rawTypeID = typeID;
		break;
	}

	if (auto priority_rst = details::MInvoke(true, &temporary_resource, rawTypeID, methodID, argTypeIDs, argptr_buffer, rst_rsrc); priority_rst.Valid())
		return priority_rst;

	return details::MInvoke(false, &temporary_resource, rawTypeID, methodID, argTypeIDs, argptr_buffer, rst_rsrc);
}

SharedObject ReflMngr::MInvoke(
	ObjectView obj,
	StrID methodID,
	std::span<const TypeID> argTypeIDs,
	ArgPtrBuffer argptr_buffer,
	std::pmr::memory_resource* rst_rsrc) const
{
	assert(rst_rsrc);

	ObjectView rawObj;
	bool is_const;
	switch (GetConstReferenceMode(obj.GetTypeID()))
	{
	case ConstReferenceMode::Left: [[fallthrough]];
	case ConstReferenceMode::Right:
		rawObj = obj.RemoveReference();
		is_const = false;
		break;
	case ConstReferenceMode::Const:
		rawObj = obj.RemoveConst();
		is_const = true;
		break;
	case ConstReferenceMode::ConstLeft: [[fallthrough]];
	case ConstReferenceMode::ConstRight:
		rawObj = obj.RemoveConstReference();
		is_const = true;
		break;
	case ConstReferenceMode::None:
	default:
		rawObj = obj;
		is_const = false;
		break;
	}

	if (auto priority_rst = details::MInvoke(true, &temporary_resource, rawObj, is_const, methodID, argTypeIDs, argptr_buffer, rst_rsrc); priority_rst.Valid())
		return priority_rst;

	return details::MInvoke(false, &temporary_resource, rawObj, is_const, methodID, argTypeIDs, argptr_buffer, rst_rsrc);
}

ObjectView ReflMngr::NonArgCopyMNew(TypeID typeID, std::pmr::memory_resource* rsrc, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	assert(rsrc);

	if (!IsConstructible(typeID, argTypeIDs))
		return nullptr;

	const auto& typeinfo = typeinfos.at(typeID);

	void* buffer = rsrc->allocate(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectView obj{ typeID, buffer };
	bool success = NonArgConstruct(obj, argTypeIDs, argptr_buffer);
	assert(success);

	return obj;
}

ObjectView ReflMngr::MNew(TypeID typeID, std::pmr::memory_resource* rsrc, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	assert(rsrc);

	if (!IsConstructible(typeID, argTypeIDs))
		return nullptr;

	const auto& typeinfo = typeinfos.at(typeID);

	void* buffer = rsrc->allocate(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectView obj{ typeID, buffer };
	bool success = Construct(obj, argTypeIDs, argptr_buffer);
	assert(success);

	return obj;
}

bool ReflMngr::MDelete(ObjectView obj, std::pmr::memory_resource* rsrc) const {
	assert(rsrc);

	bool dtor_success = Destruct(obj);
	if (!dtor_success)
		return false;

	const auto& typeinfo = typeinfos.at(obj.GetTypeID());

	rsrc->deallocate(obj.GetPtr(), typeinfo.size, typeinfo.alignment);

	return true;
}

bool ReflMngr::IsNonArgConstructible(TypeID typeID, std::span<const TypeID> argTypeIDs) const {
	assert(GetConstReferenceMode(typeID) == ConstReferenceMode::None);

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
	assert(GetConstReferenceMode(typeID) == ConstReferenceMode::None);

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
	assert(GetConstReferenceMode(typeID) == ConstReferenceMode::None);

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

bool ReflMngr::NonArgConstruct(ObjectView obj, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	assert(GetConstReferenceMode(obj.GetTypeID()) == ConstReferenceMode::None);

	if (!obj.Valid())
		return false;

	auto target = typeinfos.find(obj.GetTypeID());
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

bool ReflMngr::Construct(ObjectView obj, std::span<const TypeID> argTypeIDs, ArgPtrBuffer argptr_buffer) const {
	assert(GetConstReferenceMode(obj.GetTypeID()) == ConstReferenceMode::None);

	if (!obj.Valid())
		return false;

	auto target = typeinfos.find(obj.GetTypeID());
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

bool ReflMngr::Destruct(ObjectView obj) const {
	assert(GetConstReferenceMode(obj.GetTypeID()) == ConstReferenceMode::None);

	if (!obj.Valid())
		return false;

	auto target = typeinfos.find(obj.GetTypeID());
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
	details::ForEachTypeID(RemoveReference(typeID), func, visitedVBs);
}

void ReflMngr::ForEachType(TypeID typeID, const std::function<bool(TypeRef)>& func) const {
	std::set<TypeID> visitedVBs;
	details::ForEachTypeInfo(RemoveReference(typeID), func, visitedVBs);
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

void ReflMngr::ForEachVar(
	TypeID typeID,
	const std::function<bool(TypeRef, FieldRef, ObjectView)>& func) const
{
	std::set<TypeID> visitedVBs;
	details::ForEachVar(RemoveConstReference(typeID), func, visitedVBs);
}

void ReflMngr::ForEachVar(
	ObjectView obj,
	const std::function<bool(TypeRef, FieldRef, ObjectView)>& func) const
{
	std::set<TypeID> visitedVBs;
	switch (GetConstReferenceMode(obj.GetTypeID()))
	{
	case ConstReferenceMode::Left: [[fallthrough]];
	case ConstReferenceMode::Right:
		details::ForEachVar(obj.RemoveReference(), [&func](TypeRef t, FieldRef f, ObjectView o) {
			return func(t, f, o.AddRValueReference());
		}, visitedVBs);
		break;
	case ConstReferenceMode::Const:
		details::ForEachVar(obj.RemoveConst(), [&func](TypeRef t, FieldRef f, ObjectView o) {
			return func(t, f, o.AddConst());
		}, visitedVBs);
		break;
	case ConstReferenceMode::ConstLeft:
		details::ForEachVar(obj.RemoveConstReference(), [&func](TypeRef t, FieldRef f, ObjectView o) {
			return func(t, f, o.AddConstLValueReference());
		}, visitedVBs);
		break;
	case ConstReferenceMode::ConstRight:
		details::ForEachVar(obj.RemoveConstReference(), [&func](TypeRef t, FieldRef f, ObjectView o) {
			return func(t, f, o.AddConstRValueReference());
		}, visitedVBs);
		break;
	case ConstReferenceMode::None:
	default:
		details::ForEachVar(obj, func, visitedVBs);
		break;
	}
}

void ReflMngr::ForEachOwnedVar(
	ObjectView obj,
	const std::function<bool(TypeRef, FieldRef, ObjectView)>& func) const
{
	ForEachVar(obj, [func](TypeRef type, FieldRef field, ObjectView obj) {
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

std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> ReflMngr::GetTypeFieldVars(TypeID typeID) {
	std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> rst;
	ForEachVar(typeID, [&rst](TypeRef type, FieldRef field, ObjectView var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::vector<ObjectView> ReflMngr::GetVars(TypeID typeID) {
	std::vector<ObjectView> rst;
	ForEachVar(typeID, [&rst](TypeRef type, FieldRef field, ObjectView var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> ReflMngr::GetTypeFieldVars(ObjectView obj) {
	std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> rst;
	ForEachVar(obj, [&rst](TypeRef type, FieldRef field, ObjectView var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	});
	return rst;
}

std::vector<ObjectView> ReflMngr::GetVars(ObjectView obj) {
	std::vector<ObjectView> rst;
	ForEachVar(obj, [&rst](TypeRef type, FieldRef field, ObjectView var) {
		rst.push_back(var);
		return true;
	});
	return rst;
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> ReflMngr::GetTypeFieldOwnedVars(ObjectView obj) {
	std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> rst;
	ForEachVar(obj, [&rst](TypeRef type, FieldRef field, ObjectView var) {
		if (field.info.fieldptr.IsOwned())
			rst.emplace_back(std::tuple{ type, field, var });

		return true;
	});
	return rst;
}

std::vector<ObjectView> ReflMngr::GetOwnedVars(ObjectView obj) {
	std::vector<ObjectView> rst;
	ForEachVar(obj, [&rst](TypeRef type, FieldRef field, ObjectView var) {
		if (field.info.fieldptr.IsOwned())
			rst.push_back(var);
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

ObjectView ReflMngr::FindVar(TypeID typeID, const std::function<bool(ObjectView)>& func) const {
	ObjectView rst;
	ForEachVar(typeID, [&rst, func](TypeRef type, FieldRef field, ObjectView obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

ObjectView ReflMngr::FindVar(ObjectView obj, const std::function<bool(ObjectView)>& func) const {
	ObjectView rst;
	ForEachVar(obj, [&rst, func](TypeRef type, FieldRef field, ObjectView obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

ObjectView ReflMngr::FindOwnedVar(ObjectView obj, const std::function<bool(ObjectView)>& func) const {
	ObjectView rst;
	ForEachVar(obj, [&rst, func](TypeRef type, FieldRef field, ObjectView obj) {
		if (field.info.fieldptr.IsUnowned() || !func(obj))
			return true;

		rst = obj;
		return false; // stop
	});
	return rst;
}

bool ReflMngr::IsConst(TypeID ID) const {
	auto name = tregistry.Nameof(ID);
	return type_name_is_const(name);
}

bool ReflMngr::IsReadOnly(TypeID ID) const {
	auto name = tregistry.Nameof(ID);
	return type_name_is_const(type_name_remove_reference(name));
}

ConstReferenceMode ReflMngr::GetConstReferenceMode(TypeID ID) const {
	auto name = tregistry.Nameof(ID);
	if (type_name_is_lvalue_reference(name)) {
		if (type_name_is_const(type_name_remove_reference(name)))
			return ConstReferenceMode::ConstLeft;
		else
			return ConstReferenceMode::Left;
	}
	else if (type_name_is_rvalue_reference(name)) {
		if (type_name_is_const(type_name_remove_reference(name)))
			return ConstReferenceMode::ConstRight;
		else
			return ConstReferenceMode::Right;
	}
	else {
		if (type_name_is_const(name))
			return ConstReferenceMode::Const;
		else
			return ConstReferenceMode::None;
	}
}

bool ReflMngr::IsReference(TypeID ID) const {
	auto name = tregistry.Nameof(ID);
	return type_name_is_reference(name);
}

TypeID ReflMngr::RemoveConst(TypeID ID) const {
	auto name = tregistry.Nameof(ID);

	if (!type_name_is_const(name))
		return ID;

	return type_name_remove_const(name);
}

TypeID ReflMngr::RemoveReference(TypeID ID) const {
	auto name = tregistry.Nameof(ID);

	if (!type_name_is_reference(name))
		return ID;

	return type_name_remove_reference(name);
}

TypeID ReflMngr::RemoveConstReference(TypeID ID) const {
	auto name = tregistry.Nameof(ID);

	if (!type_name_is_reference(name) && !type_name_is_const(name))
		return ID;

	return type_name_remove_cvref(name);
}

TypeID ReflMngr::AddConst(TypeID ID) {
	return tregistry.RegisterAddConst(ID);
}

TypeID ReflMngr::AddLValueReference(TypeID ID) {
	return tregistry.RegisterAddLValueReference(ID);
}

TypeID ReflMngr::AddLValueReferenceWeak(TypeID ID) {
	return tregistry.RegisterAddLValueReferenceWeak(ID);
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
