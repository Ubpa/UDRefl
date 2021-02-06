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
	bool IsPriorityCompatible(std::span<const Type> params, std::span<const Type> argTypes) {
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
				if (!type_name_is_const(unref_lhs)) {
					if (rhs.Is(unref_lhs))
						continue; // &&{T} <- T
				}
			}
			else {
				if (!lhs.IsLValueReference()) { // T
					if (lhs.Is(rhs.Name_RemoveRValueReference()))
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
	bool IsNonCopiedArgConstructCompatible(std::span<const Type> params, std::span<const Type> argTypes) {
		if (params.size() != argTypes.size())
			return false;

		for (size_t i = 0; i < params.size(); i++) {
			if (params[i] == argTypes[i])
				continue;

			const auto& lhs = params[i];
			const auto& rhs = argTypes[i];

			if (lhs.IsLValueReference()) { // &{T} | &{const{T}}
				const auto unref_lhs = lhs.Name_RemoveLValueReference(); // T | const{T}
				if (type_name_is_const(unref_lhs)) { // &{const{T}}
					if (unref_lhs == rhs.Name_RemoveRValueReference())
						continue; // &{const{T}} <- &&{const{T}}

					const auto raw_lhs = type_name_remove_const(unref_lhs); // T

					if (rhs.Is(raw_lhs) || raw_lhs == rhs.Name_RemoveReference())
						continue; // &{const{T}} <- T | &{T} | &&{T}
				}
			}
			else if (lhs.IsRValueReference()) { // &&{T} | &&{const{T}}
				const auto unref_lhs = lhs.Name_RemoveRValueReference(); // T | const{T}
				assert(!type_name_is_volatile(unref_lhs));

				if (type_name_is_const(unref_lhs)) { // &&{const{T}}
					const auto raw_lhs = type_name_remove_const(unref_lhs); // T

					if (rhs.Is(raw_lhs))
						continue; // &&{const{T}} <- T

					if (raw_lhs == rhs.Name_RemoveRValueReference()) // &&{const{T}}
						continue; // &&{const{T}} <- &&{T}
				}
				else {
					if (rhs.Is(unref_lhs))
						continue; // &&{T} <- T
				}
			}
			else { // T
				if (lhs.Is(rhs.Name_RemoveRValueReference()))
					continue; // T <- &&{T}
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
	bool IsNonCopiedArgConstructCompatible(std::span<const Type> params, std::span<const TypeID> argTypeIDs) {
		if (params.size() != argTypeIDs.size())
			return false;

		for (size_t i = 0; i < params.size(); i++) {
			if (params[i] == argTypeIDs[i])
				continue;

			const auto& lhs = params[i];
#ifndef NDEBUG
			// because rhs(arg)'s ID maybe have no name in the registry
			// so we use type_name_add_*_hash(...) to avoid it
			auto rhs = Mngr.tregistry.Nameof(argTypeIDs[i]);
#endif // !NDEBUG
			const std::size_t rhs_hash = argTypeIDs[i].GetValue();

			if (lhs.IsLValueReference()) { // &{T} | &{const{T}}
				auto unref_lhs = lhs.Name_RemoveLValueReference(); // T | const{T}
				if (type_name_is_const(unref_lhs)) { // &{const{T}}
					if (type_name_add_rvalue_reference_hash(unref_lhs) == rhs_hash)
						continue; // &{const{T}} <- &&{const{T}}

					auto raw_lhs = type_name_remove_const(unref_lhs); // T

					if (TypeID{ raw_lhs }.GetValue() == rhs_hash
						|| type_name_add_lvalue_reference_hash(raw_lhs) == rhs_hash
						|| type_name_add_rvalue_reference_hash(raw_lhs) == rhs_hash)
						continue; // &{const{T}} <- T | &{T} | &&{T}
				}
			}
			else if (lhs.IsRValueReference()) { // &&{T} | &&{const{T}}
				auto unref_lhs = lhs.Name_RemoveRValueReference(); // T | const{T}
				assert(!type_name_is_volatile(unref_lhs));

				if (type_name_is_const(unref_lhs)) { // &&{const{T}}
					auto raw_lhs = type_name_remove_const(unref_lhs); // T

					if (TypeID{ raw_lhs }.GetValue() == rhs_hash)
						continue; // &&{const{T}} <- T

					if (type_name_add_rvalue_reference_hash(raw_lhs) == rhs_hash) // &&{const{T}}
						continue; // &&{const{T}} <- &&{T}
				}
				else {
					if (TypeID{ unref_lhs }.GetValue() == rhs_hash)
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
				NameIDRegistry::Meta::ctor,
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
			const char* name;
			std::size_t name_hash;
			std::uint32_t offset;
			std::uint16_t name_size;
			std::uint8_t idx;
			bool is_ptr;
			Type GetType() const {
				return { std::string_view{name,name_size}, TypeID{name_hash} };
			}
		}; // 24 bytes
		// MaxArgNum <= 2^8
		static_assert(sizeof(ArgInfo) * MaxArgNum < 16384);
	public:
		NewArgsGuard(
			bool is_priority,
			std::pmr::memory_resource* rsrc,
			std::span<const Type> paramTypeIDs,
			std::span<const Type> argTypes,
			ArgPtrBuffer orig_argptr_buffer) :
			rsrc{ rsrc }
		{
			if (argTypes.size() != paramTypeIDs.size())
				return;

			if (is_priority) {
				is_compatible = IsPriorityCompatible(paramTypeIDs, argTypes);
				argptr_buffer = orig_argptr_buffer;
				return;
			}

			// 1. is compatible ? (collect infos)

			const std::uint8_t num_args = static_cast<std::uint8_t>(argTypes.size());

			ArgInfo info_copiedargs[MaxArgNum + 1];
			std::uint8_t num_copiedargs = 0;
			std::uint32_t size_copiedargs = 0;

			for (std::uint8_t i = 0; i < argTypes.size(); i++) {
				if (paramTypeIDs[i] == argTypes[i])
					continue;

				const auto& lhs = paramTypeIDs[i];
				const auto& rhs = argTypes[i];

				if (lhs.IsLValueReference()) { // &{T} | &{const{T}}
					const auto unref_lhs = lhs.Name_RemoveLValueReference(); // T | const{T}
					if (type_name_is_const(unref_lhs)) { // &{const{T}}
						if (unref_lhs == rhs.Name_RemoveRValueReference())
							continue; // &{const{T}} <- &&{const{T}}

						const auto raw_lhs = type_name_remove_const(unref_lhs); // T
						if (rhs.Is(raw_lhs) || raw_lhs == rhs.Name_RemoveReference())
							continue; // &{const{T}} <- T | &{T} | &&{T}

						assert(!type_name_is_pointer(raw_lhs));

						Type raw_lhs_type{ raw_lhs };
						if (Mngr.IsNonCopiedArgConstructible(raw_lhs_type, std::span<const Type>{&rhs, 1})) {
							auto& info = info_copiedargs[num_copiedargs++];
							assert(num_copiedargs <= MaxArgNum);

							info.idx = i;
							info.is_ptr = false;
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
						const auto raw_lhs = type_name_remove_const(unref_lhs); // T

						if (rhs.Is(raw_lhs))
							continue; // &&{const{T}} <- T

						if (raw_lhs == rhs.Name_RemoveRValueReference()) // &&{const{T}}
							continue; // &&{const{T}} <- &&{T}

						assert(!type_name_is_pointer(raw_lhs));

						Type raw_lhs_type{ raw_lhs };
						if (Mngr.IsNonCopiedArgConstructible(raw_lhs_type, std::span<const Type>{&rhs, 1})) {
							auto& info = info_copiedargs[num_copiedargs++];
							assert(num_copiedargs <= MaxArgNum);

							info.idx = i;
							info.is_ptr = false;
							info.name = raw_lhs_type.GetName().data();
							info.name_size = static_cast<std::uint16_t>(raw_lhs_type.GetName().size());
							info.name_hash = raw_lhs_type.GetID().GetValue();

							continue; // &&{const{T}} <- T{arg}
						}
					}
					else { // &&{T}
						if (rhs.Is(unref_lhs))
							continue;
					}
				}
				else { // T
					if (lhs.Is(rhs.Name_RemoveRValueReference()))
						continue; // T <- &&{T}

					if ((lhs.IsPointer() || Mngr.IsCopyConstructible(lhs))
						&& (
							lhs.Is(rhs.Name_RemoveLValueReference())
							|| lhs.Is(rhs.Name_RemoveCVRef())
						))
					{
						auto& info = info_copiedargs[num_copiedargs++];
						assert(num_copiedargs <= MaxArgNum);

						info.idx = i;
						info.is_ptr = type_name_is_pointer(lhs);
						info.name = lhs.GetName().data();
						info.name_size = static_cast<std::uint16_t>(lhs.GetName().size());
						info.name_hash = lhs.GetID().GetValue();

						continue; // T <- T{arg} [copy]
					}

					if (Mngr.IsNonCopiedArgConstructible(lhs, std::span<const Type>{&rhs, 1})) {
						auto& info = info_copiedargs[num_copiedargs++];
						assert(num_copiedargs <= MaxArgNum);

						info.idx = i;
						info.is_ptr = false;
						info.name = lhs.GetName().data();
						info.name_size = static_cast<std::uint16_t>(lhs.GetName().size());
						info.name_hash = lhs.GetID().GetValue();

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

			for (std::uint8_t k = 0; k < num_copiedargs; ++k) {
				std::uint32_t size, alignment;
				if (info_copiedargs[k].is_ptr) {
					size = static_cast<std::uint32_t>(sizeof(void*));
					alignment = static_cast<std::uint32_t>(alignof(void*));
				}
				else {
					++num_copied_nonptr_args;
					const auto& typeinfo = Mngr.typeinfos.at(info_copiedargs[k].GetType());
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
				if (info.is_ptr)
					buffer_as<void*>(arg_buffer) = orig_argptr_buffer[i];
				else {
					bool success = Mngr.NonCopiedArgConstruct(
						ObjectView{ info.GetType(), arg_buffer },
						std::span<const Type>{&argTypes[i], 1},
						static_cast<ArgPtrBuffer>(&orig_argptr_buffer[i])
					);
					assert(success);
					new_nonptr_arg_info_buffer[idx_nonptr_args++] = info;
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
					bool success = Mngr.Destruct({ info.GetType(), argptr_buffer[info.idx] });
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
		std::uint8_t num_copied_nonptr_args{ 0 };
		ArgPtrBuffer argptr_buffer{ nullptr };
	};

	static InvocableResult IsInvocable(
		bool is_priority,
		Type type,
		Name method_name,
		std::span<const Type> argTypes,
		FuncFlag flag)
	{
		assert(type.CVRefMode() == CVRefMode::None);
		auto typetarget = Mngr.typeinfos.find(type);

		if (typetarget == Mngr.typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;
		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(method_name);

		// 1. object variable
		if (enum_contain(flag, FuncFlag::Variable)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (iter->second.methodptr.IsMemberVariable()
					&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypes)
						: Mngr.IsCompatible(iter->second.methodptr.GetParamList(), argTypes)))
				{
					return { true, iter->second.methodptr.GetResultDesc() };
				}
			}
		}

		// 2. object const and static
		if(enum_contain(flag, FuncFlag::Const | FuncFlag::Static)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (!iter->second.methodptr.IsMemberVariable() && enum_contain_any(flag, iter->second.methodptr.GetFuncFlag())
					&& (is_priority ? IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypes)
						: Mngr.IsCompatible(iter->second.methodptr.GetParamList(), argTypes)))
				{
					return { true, iter->second.methodptr.GetResultDesc() };
				}
			}
		}

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			if (auto rst = IsInvocable(is_priority, base, method_name, argTypes, flag))
				return rst;
		}

		return {};
	}

	static InvokeResult Invoke(
		bool is_priority,
		std::pmr::memory_resource* args_rsrc,
		ObjectView obj,
		Name method_name,
		void* result_buffer,
		std::span<const Type> argTypes,
		ArgPtrBuffer argptr_buffer,
		FuncFlag flag)
	{
		assert(obj.GetType().CVRefMode() == CVRefMode::None);

		auto typetarget = Mngr.typeinfos.find(obj.GetType());

		if (typetarget == Mngr.typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(method_name);

		if (enum_contain(flag, FuncFlag::Variable)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (iter->second.methodptr.IsMemberVariable()) {
					NewArgsGuard guard{
						is_priority, args_rsrc,
						iter->second.methodptr.GetParamList(), argTypes, argptr_buffer
					};
					if (!guard.IsCompatible())
						continue;
					return {
						true,
						iter->second.methodptr.GetResultDesc().type,
						iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, guard.GetArgPtrBuffer())
					};
				}
			}
		}
		if (enum_contain_any(flag, FuncFlag::Const | FuncFlag::Static)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (!iter->second.methodptr.IsMemberVariable() && enum_contain(flag, iter->second.methodptr.GetFuncFlag())) {
					NewArgsGuard guard{
						is_priority, args_rsrc,
						iter->second.methodptr.GetParamList(), argTypes, argptr_buffer
					};
					if (!guard.IsCompatible())
						continue;
					return {
						true,
						iter->second.methodptr.GetResultDesc().type,
						iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), result_buffer, guard.GetArgPtrBuffer())
					};
				}
			}
		}


		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			auto rst = Invoke(
				is_priority, args_rsrc,
				ObjectView{ base, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
				method_name, result_buffer, argTypes, argptr_buffer,
				flag
			);
			if (rst.success)
				return rst;
		}

		return {};
	}

	static SharedObject MInvoke(
		bool is_priority,
		std::pmr::memory_resource* args_rsrc,
		ObjectView obj,
		Name method_name,
		std::pmr::memory_resource* rst_rsrc,
		std::span<const Type> argTypes,
		ArgPtrBuffer argptr_buffer,
		FuncFlag flag)
	{
		assert(rst_rsrc);
		assert(obj.GetType().CVRefMode() == CVRefMode::None);
		auto typetarget = Mngr.typeinfos.find(obj.GetType());

		if (typetarget == Mngr.typeinfos.end())
			return {};

		const auto& typeinfo = typetarget->second;

		auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(method_name);

		if (enum_contain(flag, FuncFlag::Variable)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (iter->second.methodptr.IsMemberVariable()) {
					NewArgsGuard guard{
						is_priority, args_rsrc,
						iter->second.methodptr.GetParamList(), argTypes, argptr_buffer
					};

					if (!guard.IsCompatible())
						continue;

					const auto& methodptr = iter->second.methodptr;
					const auto& rst_desc = methodptr.GetResultDesc();

					if (rst_desc.type.Is<void>()) {
						iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, argptr_buffer);
						return SharedObject{ rst_desc.type };
					}
					else if (rst_desc.type.IsReference()) {
						std::aligned_storage_t<sizeof(void*)> buffer;
						iter->second.methodptr.Invoke(obj.GetPtr(), &buffer, guard.GetArgPtrBuffer());
						return { rst_desc.type, buffer_as<void*>(&buffer) };
					}
					else {
						void* result_buffer = rst_rsrc->allocate(rst_desc.size, rst_desc.alignment);
						auto dtor = iter->second.methodptr.Invoke(obj.GetPtr(), result_buffer, guard.GetArgPtrBuffer());
						return {
							{rst_desc.type, result_buffer},
							GenerateDeleteFunc(std::move(dtor), rst_rsrc, rst_desc.size, rst_desc.alignment)
						};
					}
				}
			}
		}

		if (enum_contain_any(flag, FuncFlag::Const | FuncFlag::Static)) {
			for (auto iter = begin_iter; iter != end_iter; ++iter) {
				if (!iter->second.methodptr.IsMemberVariable() && enum_contain(flag, iter->second.methodptr.GetFuncFlag())) {
					NewArgsGuard guard{
						is_priority, args_rsrc,
						iter->second.methodptr.GetParamList(), argTypes, argptr_buffer
					};

					if (!guard.IsCompatible())
						continue;

					const auto& methodptr = iter->second.methodptr;
					const auto& rst_desc = methodptr.GetResultDesc();

					if (rst_desc.type.Is<void>()) {
						iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), nullptr, argptr_buffer);
						return SharedObject{ rst_desc.type };
					}
					else if (rst_desc.type.IsReference()) {
						std::aligned_storage_t<sizeof(void*)> buffer;
						iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), &buffer, guard.GetArgPtrBuffer());
						return { rst_desc.type, buffer_as<void*>(&buffer) };
					}
					else {
						void* result_buffer = rst_rsrc->allocate(rst_desc.size, rst_desc.alignment);
						auto dtor = iter->second.methodptr.Invoke(static_cast<const void*>(obj.GetPtr()), result_buffer, guard.GetArgPtrBuffer());
						return {
							{rst_desc.type, result_buffer},
							GenerateDeleteFunc(std::move(dtor), rst_rsrc, rst_desc.size, rst_desc.alignment)
						};
					}
				}
			}
		}

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			auto rst = MInvoke(
				is_priority, args_rsrc,
				ObjectView{ base, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) },
				method_name, rst_rsrc, argTypes, argptr_buffer,
				flag
			);
			if (rst.GetType())
				return rst;
		}

		return nullptr;
	}

	static bool ForEachTypeInfo(
		Type type,
		const std::function<bool(TypeRef)>& func,
		std::set<TypeID>& visitedVBs)
	{
		auto target = Mngr.typeinfos.find(type);

		if (target == Mngr.typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		if (!func({ type, typeinfo }))
			return false;

		for (auto& [base, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(base) != visitedVBs.end())
					continue;
				visitedVBs.insert(base);
			}

			if (!ForEachTypeInfo(base, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachVar(
		Type type,
		const std::function<bool(TypeRef, FieldRef, ObjectView)>& func,
		std::set<TypeID>& visitedVBs)
	{
		auto target = Mngr.typeinfos.find(type);

		if (target == Mngr.typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [field, fieldInfo] : typeinfo.fieldinfos) {
			if (fieldInfo.fieldptr.IsUnowned()) {
				if (!func({ type, typeinfo }, { field, fieldInfo }, fieldInfo.fieldptr.Var()))
					return false;
			}
		}

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(base) != visitedVBs.end())
					continue;
				visitedVBs.insert(base);
			}

			if (!ForEachVar(base, func, visitedVBs))
				return false;
		}

		return true;
	}

	static bool ForEachVar(
		ObjectView obj,
		const std::function<bool(TypeRef, FieldRef, ObjectView)>& func,
		FieldFlag flag,
		std::set<TypeID>& visitedVBs)
	{
		assert(obj.GetType().CVRefMode() == CVRefMode::None);

		auto target = Mngr.typeinfos.find(obj.GetType());

		if (target == Mngr.typeinfos.end())
			return true;

		auto& typeinfo = target->second;

		for (auto& [field, fieldInfo] : typeinfo.fieldinfos) {
			if (!enum_contain(flag, fieldInfo.fieldptr.GetFieldFlag()))
				continue;

			if (!func({ obj.GetType(), typeinfo }, { field, fieldInfo }, fieldInfo.fieldptr.Var(obj.GetPtr())))
				return false;
		}

		for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
			if (baseinfo.IsVirtual()) {
				if (visitedVBs.find(base) != visitedVBs.end())
					continue;
				visitedVBs.insert(base);
			}

			if (!ForEachVar(ObjectView{ base, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, func, flag, visitedVBs))
				return false;
		}

		return true;
	}
}

ReflMngr::ReflMngr() {
	RegisterType(Global, 0, 0);

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

TypeInfo* ReflMngr::GetTypeInfo(Type type) {
	auto target = typeinfos.find(type.RemoveCVRef());
	if (target == typeinfos.end())
		return nullptr;
	return &target->second;
}

void ReflMngr::Clear() noexcept {
	// field attrs
	for (auto& [type, typeinfo] : typeinfos) {
		for (auto& [field, fieldinfo] : typeinfo.fieldinfos)
			fieldinfo.attrs.clear();
	}

	// type attrs
	for (auto& [ID, typeinfo] : typeinfos)
		typeinfo.attrs.clear();

	// type dynamic field
	for (auto& [type, typeinfo] : typeinfos) {
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
	object_resource.release();
}

ReflMngr::~ReflMngr() {
	Clear();
}

void ReflMngr::RegisterType(Type type, size_t size, size_t alignment) {
	assert(type.CVRefMode() == CVRefMode::None);

	auto target = typeinfos.find(type);
	if (target != typeinfos.end())
		return;

	typeinfos.emplace_hint(target, type, TypeInfo{ size,alignment });
}

bool ReflMngr::AddField(Type type, Name field_name, FieldInfo fieldinfo) {
	auto ttarget = typeinfos.find(type);
	if (ttarget == typeinfos.end())
		return false;
	auto& typeinfo = ttarget->second;
	auto ftarget = typeinfo.fieldinfos.find(field_name);
	if (ftarget != typeinfo.fieldinfos.end())
		return false; // same name
	typeinfo.fieldinfos.emplace_hint(ftarget, field_name, std::move(fieldinfo));
	return true;
}

bool ReflMngr::AddMethod(Type type, Name method_name, MethodInfo methodinfo) {
	auto ttarget = typeinfos.find(type);
	if (ttarget == typeinfos.end())
		return false;
	auto& typeinfo = ttarget->second;
	auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(method_name);
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (!iter->second.methodptr.IsDistinguishableWith(methodinfo.methodptr))
			return false;
	}
	typeinfo.methodinfos.emplace(method_name, std::move(methodinfo));
	return true;
}

bool ReflMngr::AddBase(Type derived, Type base, BaseInfo baseinfo) {
	auto ttarget = typeinfos.find(derived);
	if (ttarget == typeinfos.end())
		return false;
	auto& typeinfo = ttarget->second;
	auto btarget = typeinfo.baseinfos.find(base);
	if (btarget != typeinfo.baseinfos.end())
		return false;
	typeinfo.baseinfos.emplace_hint(btarget, base, std::move(baseinfo));
	return true;
}

bool ReflMngr::AddAttr(Type type, Attr attr) {
	auto ttarget = typeinfos.find(type);
	if (ttarget == typeinfos.end())
		return false;
	auto& typeinfo = ttarget->second;
	auto atarget = typeinfo.attrs.find(attr);
	if (atarget != typeinfo.attrs.end())
		return false;
	typeinfo.attrs.emplace_hint(atarget, std::move(attr));
	return true;
}

ObjectView ReflMngr::NonArgCopyNew(Type type, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	if (!IsNonCopiedArgConstructible(type, argTypes))
		return nullptr;

	const auto& typeinfo = typeinfos.at(type);

	void* buffer = object_resource.allocate(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectView obj{ type, buffer };
	bool success = NonCopiedArgConstruct(obj, argTypes, argptr_buffer);
	assert(success);

	return obj;
}

ObjectView ReflMngr::New(Type type, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	if (!IsConstructible(type, argTypes))
		return nullptr;

	const auto& typeinfo = typeinfos.at(type);

	void* buffer = object_resource.allocate(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectView obj{ type, buffer };
	bool success = Construct(obj, argTypes, argptr_buffer);
	assert(success);
	
	return obj;
}

bool ReflMngr::Delete(ObjectView obj) const {
	bool dtor_success = Destruct(obj);
	if (!dtor_success)
		return false;

	const auto& typeinfo = typeinfos.at(obj.GetType());

	object_resource.deallocate(obj.GetPtr(), typeinfo.size, typeinfo.alignment);

	return true;
}

SharedObject ReflMngr::MakeShared(Type type, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	ObjectView obj = New(type, argTypes, argptr_buffer);

	if (!obj.GetType().Valid())
		return nullptr;

	return { obj, [type](void* ptr) {
		bool success = Mngr.Delete({type, ptr});
		assert(success);
	} };
}

ObjectView ReflMngr::StaticCast_DerivedToBase(ObjectView obj, Type type) const {
	assert(type);

	const CVRefMode cvref_mode = obj.GetType().CVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		return StaticCast_DerivedToBase(obj.RemoveReference(), type).AddLValueReference();
	case CVRefMode::Right:
		return StaticCast_DerivedToBase(obj.RemoveReference(), type).AddRValueReference();
	case CVRefMode::Const:
		return StaticCast_DerivedToBase(obj.RemoveConst(), type).AddConst();
	case CVRefMode::ConstLeft:
		return StaticCast_DerivedToBase(obj.RemoveConstReference(), type).AddConstLValueReference();
	case CVRefMode::ConstRight:
		return StaticCast_DerivedToBase(obj.RemoveConstReference(), type).AddConstRValueReference();
	default:
		break;
	}

	if (obj.GetType() == type)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { type, nullptr };

	auto target = typeinfos.find(obj.GetType());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
		auto ptr = StaticCast_DerivedToBase(ObjectView{ base, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, type);
		if (ptr.GetType())
			return ptr;
	}

	return nullptr;
}

ObjectView ReflMngr::StaticCast_BaseToDerived(ObjectView obj, Type type) const {
	assert(type);
	
	const CVRefMode cvref_mode = obj.GetType().CVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		return StaticCast_BaseToDerived(obj.RemoveReference(), type).AddLValueReference();
	case CVRefMode::Right:
		return StaticCast_BaseToDerived(obj.RemoveReference(), type).AddRValueReference();
	case CVRefMode::Const:
		return StaticCast_BaseToDerived(obj.RemoveConst(), type).AddConst();
	case CVRefMode::ConstLeft:
		return StaticCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstLValueReference();
	case CVRefMode::ConstRight:
		return StaticCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstRValueReference();
	default:
		break;
	}

	if (obj.GetType() == type)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { type, nullptr };

	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
		auto ptr = StaticCast_BaseToDerived(obj, base);
		if (ptr.GetType())
			return { base, baseinfo.IsVirtual() ? nullptr : baseinfo.StaticCast_BaseToDerived(obj.GetPtr()) };
	}
	
	return nullptr;
}

ObjectView ReflMngr::DynamicCast_BaseToDerived(ObjectView obj, Type type) const {
	assert(type);
	
	const CVRefMode cvref_mode = obj.GetType().CVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		return DynamicCast_BaseToDerived(obj.RemoveReference(), type).AddLValueReference();
	case CVRefMode::Right:
		return DynamicCast_BaseToDerived(obj.RemoveReference(), type).AddRValueReference();
	case CVRefMode::Const:
		return DynamicCast_BaseToDerived(obj.RemoveConst(), type).AddConst();
	case CVRefMode::ConstLeft:
		return DynamicCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstLValueReference();
	case CVRefMode::ConstRight:
		return DynamicCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstRValueReference();
	default:
		break;
	}

	if (obj.GetType() == type)
		return obj;

	if (obj.GetPtr() == nullptr)
		return { type, nullptr };

	auto target = typeinfos.find(obj.GetType());
	if (target == typeinfos.end())
		return nullptr;

	const auto& typeinfo = target->second;

	for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
		auto ptr = DynamicCast_BaseToDerived(ObjectView{ base, baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) }, type);
		if (ptr.GetType())
			return { base, baseinfo.IsPolymorphic() ? baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) : nullptr };
	}

	return nullptr;
}

ObjectView ReflMngr::StaticCast(ObjectView obj, Type type) const {
	auto ptr_d2b = StaticCast_DerivedToBase(obj, type);
	if (ptr_d2b.GetType())
		return ptr_d2b;

	auto ptr_b2d = StaticCast_BaseToDerived(obj, type);
	if (ptr_b2d.GetType())
		return ptr_b2d;

	return nullptr;
}

ObjectView ReflMngr::DynamicCast(ObjectView obj, Type type) const {
	auto ptr_b2d = DynamicCast_BaseToDerived(obj, type);
	if (ptr_b2d.GetType())
		return ptr_b2d;

	auto ptr_d2b = StaticCast_DerivedToBase(obj, type);
	if (ptr_d2b.GetType())
		return ptr_d2b;

	return nullptr;
}

ObjectView ReflMngr::Var(ObjectView obj, Name field_name, FieldFlag flag) {
	const CVRefMode cvref_mode = obj.GetType().CVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left:
		return Var(obj.RemoveReference(), field_name, flag).AddLValueReference();
	case CVRefMode::Right:
		return Var(obj.RemoveReference(), field_name, flag).AddRValueReference();
	case CVRefMode::Const:
		return Var(obj.RemoveConst(), field_name, flag).AddConst();
	case CVRefMode::ConstLeft:
		return Var(obj.RemoveConstReference(), field_name, flag).AddConstLValueReference();
	case CVRefMode::ConstRight:
		return Var(obj.RemoveConstReference(), field_name, flag).AddConstRValueReference();
	default:
		break;
	}

	if (!obj.GetPtr())
		flag = enum_within(flag, FieldFlag::Unowned);

	auto ttarget = typeinfos.find(obj.GetType());
	if (ttarget == typeinfos.end())
		return nullptr;

	auto& typeinfo = ttarget->second;

	auto ftarget = typeinfo.fieldinfos.find(field_name);
	if (ftarget != typeinfo.fieldinfos.end() && enum_contain(flag, ftarget->second.fieldptr.GetFieldFlag()))
		return ftarget->second.fieldptr.Var(obj.GetPtr());

	for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
		auto bptr = Var(ObjectView{ base, baseinfo.StaticCast_DerivedToBase(obj.GetPtr()) }, field_name);
		if (bptr.GetType())
			return bptr;
	}

	return nullptr;
}

ObjectView ReflMngr::Var(ObjectView obj, Type base, Name field_name, FieldFlag flag) {
	auto base_obj = StaticCast_DerivedToBase(obj, base);
	if (!base_obj.GetType())
		return nullptr;
	return Var(base_obj, field_name);
}

bool ReflMngr::IsCompatible(std::span<const Type> params, std::span<const Type> argTypes) const {
	if (params.size() != argTypes.size())
		return false;

	for (size_t i = 0; i < params.size(); i++) {
		if (params[i] == argTypes[i])
			continue;

		const auto& lhs = params[i];
		const auto& rhs = argTypes[i];

		if (lhs.IsLValueReference()) { // &{T} | &{const{T}}
			const auto unref_lhs = lhs.Name_RemoveLValueReference(); // T | const{T}
			if (type_name_is_const(unref_lhs)) { // &{const{T}}
				if (unref_lhs == rhs.Name_RemoveRValueReference())
					continue; // &{const{T}} <- &&{const{T}}

				const auto raw_lhs = type_name_remove_const(unref_lhs); // T

				if (rhs.Is(raw_lhs) || raw_lhs == rhs.Name_RemoveReference())
					continue; // &{const{T}} <- T | &{T} | &&{T}

				if (IsNonCopiedArgConstructible(raw_lhs, std::span<const Type>{&rhs, 1}))
					continue; // &{const{T}} <- T{arg}
			}
		}
		else if (lhs.IsRValueReference()) { // &&{T} | &&{const{T}}
			const auto unref_lhs = lhs.RemoveRValueReference(); // T | const{T}

			if (type_name_is_const(unref_lhs)) { // &&{const{T}}
				const auto raw_lhs = type_name_remove_const(unref_lhs); // T

				if (rhs.Is(raw_lhs))
					continue; // &&{const{T}} <- T

				if (raw_lhs == rhs.Name_RemoveRValueReference()) // &&{const{T}}
					continue; // &&{const{T}} <- &&{T}

				if (IsNonCopiedArgConstructible(raw_lhs, std::span<const Type>{&rhs, 1}))
					continue; // &&{const{T}} <- T{arg}
			}
			else {
				if (rhs.Is(unref_lhs))
					continue; // &&{T} <- T

				if (IsNonCopiedArgConstructible(unref_lhs, std::span<const Type>{&rhs, 1}))
					continue; // &&{T} <- T{arg}
			}
		}
		else { // T
			if (lhs.Is(rhs.Name_RemoveRValueReference()))
				continue; // T <- &&{T}

			if ((lhs.IsPointer() || IsCopyConstructible(lhs))
				&& (
					lhs.Is(rhs.Name_RemoveLValueReference())
					|| lhs.Is(rhs.Name_RemoveCVRef())
				))
			{
				continue; // T <- T{arg} [copy]
			}

			if (IsNonCopiedArgConstructible(lhs, std::span<const Type>{&rhs, 1}))
				continue; // T <- T{arg}
		}

		return false;
	}

	return true;
}

InvocableResult ReflMngr::IsInvocable(Type type, Name method_name, std::span<const Type> argTypes, FuncFlag flag) const {
	const CVRefMode cvref_mode = type.CVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left: [[fallthrough]];
	case CVRefMode::Right:
		return IsInvocable(type.RemoveReference(), method_name, argTypes, flag);
	case CVRefMode::Const: [[fallthrough]];
	case CVRefMode::ConstLeft: [[fallthrough]];
	case CVRefMode::ConstRight:
		return IsInvocable(type.RemoveCVRef(), method_name, argTypes, enum_remove(flag, FuncFlag::Variable));
	default:
		break;
	}

	if (auto priority_rst = details::IsInvocable(true, type, method_name, argTypes, flag))
		return priority_rst;

	return details::IsInvocable(false, type, method_name, argTypes, flag);
}

InvokeResult ReflMngr::Invoke(
	ObjectView obj,
	Name method_name,
	void* result_buffer,
	std::span<const Type> argTypes,
	ArgPtrBuffer argptr_buffer,
	FuncFlag flag) const
{
	ObjectView rawObj;
	const CVRefMode cvref_mode = obj.GetType().CVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left: [[fallthrough]];
	case CVRefMode::Right:
		rawObj = obj.RemoveReference();
		break;
	case CVRefMode::Const:
		rawObj = obj.RemoveConst();
		flag = enum_remove(flag, FuncFlag::Variable);
		break;
	case CVRefMode::ConstLeft: [[fallthrough]];
	case CVRefMode::ConstRight:
		rawObj = obj.RemoveConstReference();
		flag = enum_remove(flag, FuncFlag::Variable);
		break;
	default:
		rawObj = obj;
		break;
	}

	if (!obj.GetPtr())
		flag = enum_within(flag, FuncFlag::Static);

	if (auto priority_rst = details::Invoke(true, &temporary_resource, rawObj, method_name, result_buffer, argTypes, argptr_buffer, flag))
		return priority_rst;

	return details::Invoke(false, &temporary_resource, rawObj, method_name, result_buffer, argTypes, argptr_buffer, flag);
}

SharedObject ReflMngr::MInvoke(
	ObjectView obj,
	Name method_name,
	std::pmr::memory_resource* rst_rsrc,
	std::span<const Type> argTypes,
	ArgPtrBuffer argptr_buffer,
	FuncFlag flag) const
{
	assert(rst_rsrc);

	ObjectView rawObj;
	const CVRefMode cvref_mode = obj.GetType().CVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left: [[fallthrough]];
	case CVRefMode::Right:
		rawObj = obj.RemoveReference();
		break;
	case CVRefMode::Const:
		rawObj = obj.RemoveConst();
		flag = enum_remove(flag, FuncFlag::Variable);
		break;
	case CVRefMode::ConstLeft: [[fallthrough]];
	case CVRefMode::ConstRight:
		rawObj = obj.RemoveConstReference();
		flag = enum_remove(flag, FuncFlag::Variable);
		break;
	default:
		rawObj = obj;
		break;
	}

	if (!obj.GetPtr())
		flag = enum_within(flag, FuncFlag::Static);

	if (auto priority_rst = details::MInvoke(true, &temporary_resource, rawObj, method_name, rst_rsrc, argTypes, argptr_buffer, flag); priority_rst.GetType().Valid())
		return priority_rst;

	return details::MInvoke(false, &temporary_resource, rawObj, method_name, rst_rsrc, argTypes, argptr_buffer, flag);
}

ObjectView ReflMngr::NonArgCopyMNew(Type type, std::pmr::memory_resource* rsrc, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	assert(rsrc);

	if (!IsConstructible(type, argTypes))
		return nullptr;

	const auto& typeinfo = typeinfos.at(type);

	void* buffer = rsrc->allocate(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectView obj{ type, buffer };
	bool success = NonCopiedArgConstruct(obj, argTypes, argptr_buffer);
	assert(success);

	return obj;
}

ObjectView ReflMngr::MNew(Type type, std::pmr::memory_resource* rsrc, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	assert(rsrc);

	if (!IsConstructible(type, argTypes))
		return nullptr;

	const auto& typeinfo = typeinfos.at(type);

	void* buffer = rsrc->allocate(typeinfo.size, typeinfo.alignment);

	if (!buffer)
		return nullptr;

	ObjectView obj{ type, buffer };
	bool success = Construct(obj, argTypes, argptr_buffer);
	assert(success);

	return obj;
}

bool ReflMngr::MDelete(ObjectView obj, std::pmr::memory_resource* rsrc) const {
	assert(rsrc);

	bool dtor_success = Destruct(obj);
	if (!dtor_success)
		return false;

	const auto& typeinfo = typeinfos.at(obj.GetType());

	rsrc->deallocate(obj.GetPtr(), typeinfo.size, typeinfo.alignment);

	return true;
}

bool ReflMngr::IsNonCopiedArgConstructible(Type type, std::span<const Type> argTypes) const {
	assert(type.CVRefMode() == CVRefMode::None);

	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto ctorID = NameIDRegistry::Meta::ctor;
	auto mtarget = typeinfo.methodinfos.find(ctorID);
	size_t num = typeinfo.methodinfos.count(ctorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (details::IsNonCopiedArgConstructCompatible(mtarget->second.methodptr.GetParamList(), argTypes))
			return true;
	}
	return false;
}

bool ReflMngr::IsNonCopiedArgConstructible(Type type, std::span<const TypeID> argTypeIDs) const {
	assert(type.CVRefMode() == CVRefMode::None);

	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto ctorID = NameIDRegistry::Meta::ctor;
	auto mtarget = typeinfo.methodinfos.find(ctorID);
	size_t num = typeinfo.methodinfos.count(ctorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (details::IsNonCopiedArgConstructCompatible(mtarget->second.methodptr.GetParamList(), argTypeIDs))
			return true;
	}
	return false;
}

bool ReflMngr::IsConstructible(Type type, std::span<const Type> argTypes) const {
	assert(type.CVRefMode() == CVRefMode::None);

	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto ctorID = NameIDRegistry::Meta::ctor;
	auto mtarget = typeinfo.methodinfos.find(ctorID);
	size_t num = typeinfo.methodinfos.count(ctorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (IsCompatible(mtarget->second.methodptr.GetParamList(), argTypes))
			return true;
	}
	return false;
}

bool ReflMngr::IsCopyConstructible(Type type) const {
	assert(type.CVRefMode() == CVRefMode::None);
	const TypeID clref_typeID = type.ID_AddConstLValueReference();
	return IsNonCopiedArgConstructible(type, std::span<const TypeID>{&clref_typeID, 1});
}

bool ReflMngr::IsMoveConstructible(Type type) const {
	assert(type.CVRefMode() == CVRefMode::None);
	const TypeID rref_typeID = type.ID_AddRValueReference();
	return IsNonCopiedArgConstructible(type, std::span<const TypeID>{&rref_typeID, 1});
}

bool ReflMngr::IsDestructible(Type type) const {
	assert(type.CVRefMode() == CVRefMode::None);

	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto dtorID = NameIDRegistry::Meta::dtor;

	auto mtarget = typeinfo.methodinfos.find(dtorID);
	size_t num = typeinfo.methodinfos.count(dtorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (!mtarget->second.methodptr.IsMemberVariable()
			&& IsCompatible(mtarget->second.methodptr.GetParamList(), {}))
			return true;
	}
	return false;
}

bool ReflMngr::NonCopiedArgConstruct(ObjectView obj, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	assert(obj.GetType().CVRefMode() == CVRefMode::None);

	if (!obj.GetType().Valid())
		return false;

	auto target = typeinfos.find(obj.GetType());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto ctorID = NameIDRegistry::Meta::ctor;
	auto mtarget = typeinfo.methodinfos.find(ctorID);
	size_t num = typeinfo.methodinfos.count(ctorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.IsMemberVariable()
			&& details::IsNonCopiedArgConstructCompatible(mtarget->second.methodptr.GetParamList(), argTypes))
		{
			mtarget->second.methodptr.Invoke(obj.GetPtr(), nullptr, argptr_buffer);
			return true;
		}
	}
	return false;
}

bool ReflMngr::Construct(ObjectView obj, std::span<const Type> argTypes, ArgPtrBuffer argptr_buffer) const {
	assert(obj.GetType().CVRefMode() == CVRefMode::None);

	if (!obj.GetType().Valid())
		return false;

	auto target = typeinfos.find(obj.GetType());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto ctorID = NameIDRegistry::Meta::ctor;
	auto mtarget = typeinfo.methodinfos.find(ctorID);
	size_t num = typeinfo.methodinfos.count(ctorID);
	for (size_t i = 0; i < num; ++i, ++mtarget) {
		if (mtarget->second.methodptr.IsMemberVariable()) {
			details::NewArgsGuard guard{
				false, &temporary_resource,
				mtarget->second.methodptr.GetParamList(), argTypes, argptr_buffer
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
	assert(obj.GetType().CVRefMode() == CVRefMode::None);

	if (!obj.GetType().Valid())
		return false;

	auto target = typeinfos.find(obj.GetType());
	if (target == typeinfos.end())
		return false;
	const auto& typeinfo = target->second;
	constexpr auto dtorID = NameIDRegistry::Meta::dtor;
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

void ReflMngr::ForEachTypeInfo(Type type, const std::function<bool(TypeRef)>& func) const {
	std::set<TypeID> visitedVBs;
	details::ForEachTypeInfo(type.RemoveReference(), func, visitedVBs);
}

void ReflMngr::ForEachField(
	Type type,
	const std::function<bool(TypeRef, FieldRef)>& func,
	FieldFlag flag) const
{
	ForEachTypeInfo(type, [flag, &func](TypeRef type) {
		for (auto& [field, fieldInfo] : type.info.fieldinfos) {
			if (!enum_contain(flag, fieldInfo.fieldptr.GetFieldFlag()))
				continue;

			if (!func(type, { field, fieldInfo }))
				return false;
		}
		return true;
	});
}

void ReflMngr::ForEachMethod(
	Type type,
	const std::function<bool(TypeRef, MethodRef)>& func,
	FuncFlag flag) const
{
	ForEachTypeInfo(type, [flag, &func](TypeRef type) {
		for (auto& [method_name, methodInfo] : type.info.methodinfos) {
			if (!enum_contain(flag, methodInfo.methodptr.GetFuncFlag()))
				continue;

			if (!func(type, { method_name, methodInfo }))
				return false;
		}
		return true;
	});
}

void ReflMngr::ForEachVar(
	ObjectView obj,
	const std::function<bool(TypeRef, FieldRef, ObjectView)>& func,
	FieldFlag flag) const
{
	if (!obj.GetPtr())
		flag = enum_within(flag, FieldFlag::Unowned);

	std::set<TypeID> visitedVBs;
	const CVRefMode cvref_mode = obj.GetType().CVRefMode();
	assert(!CVRefMode_IsVolatile(cvref_mode));
	switch (cvref_mode)
	{
	case CVRefMode::Left: [[fallthrough]];
	case CVRefMode::Right:
		details::ForEachVar(obj.RemoveReference(), [&func](TypeRef t, FieldRef f, ObjectView o) {
			return func(t, f, o.AddRValueReference());
		}, flag, visitedVBs);
		break;
	case CVRefMode::Const:
		details::ForEachVar(obj.RemoveConst(), [&func](TypeRef t, FieldRef f, ObjectView o) {
			return func(t, f, o.AddConst());
		}, flag, visitedVBs);
		break;
	case CVRefMode::ConstLeft:
		details::ForEachVar(obj.RemoveConstReference(), [&func](TypeRef t, FieldRef f, ObjectView o) {
			return func(t, f, o.AddConstLValueReference());
		}, flag, visitedVBs);
		break;
	case CVRefMode::ConstRight:
		details::ForEachVar(obj.RemoveConstReference(), [&func](TypeRef t, FieldRef f, ObjectView o) {
			return func(t, f, o.AddConstRValueReference());
		}, flag, visitedVBs);
		break;
	default:
		details::ForEachVar(obj, func, flag, visitedVBs);
		break;
	}
}

std::vector<TypeRef> ReflMngr::GetTypes(Type type) {
	std::vector<TypeRef> rst;
	ForEachTypeInfo(type, [&rst](TypeRef type) {
		rst.push_back(type);
		return true;
	});
	return rst;
}

std::vector<TypeFieldRef> ReflMngr::GetTypeFields(Type type, FieldFlag flag) {
	std::vector<TypeFieldRef> rst;
	ForEachField(type, [&rst](TypeRef type, FieldRef field) {
		rst.emplace_back(TypeFieldRef{ type, field });
		return true;
	}, flag);
	return rst;
}

std::vector<FieldRef> ReflMngr::GetFields(Type type, FieldFlag flag) {
	std::vector<FieldRef> rst;
	ForEachField(type, [&rst](TypeRef type, FieldRef field) {
		rst.push_back(field);
		return true;
	}, flag);
	return rst;
}

std::vector<TypeMethodRef> ReflMngr::GetTypeMethods(Type type, FuncFlag flag) {
	std::vector<TypeMethodRef> rst;
	ForEachMethod(type, [&rst](TypeRef type, MethodRef field) {
		rst.emplace_back(TypeMethodRef{ type, field });
		return true;
	}, flag);
	return rst;
}

std::vector<MethodRef> ReflMngr::GetMethods(Type type, FuncFlag flag) {
	std::vector<MethodRef> rst;
	ForEachMethod(type, [&rst](TypeRef type, MethodRef field) {
		rst.push_back(field);
		return true;
	}, flag);
	return rst;
}

std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> ReflMngr::GetTypeFieldVars(ObjectView obj, FieldFlag flag) {
	std::vector<std::tuple<TypeRef, FieldRef, ObjectView>> rst;
	ForEachVar(obj, [&rst](TypeRef type, FieldRef field, ObjectView var) {
		rst.emplace_back(std::tuple{ type, field, var });
		return true;
	}, flag);
	return rst;
}

std::vector<ObjectView> ReflMngr::GetVars(ObjectView obj, FieldFlag flag) {
	std::vector<ObjectView> rst;
	ForEachVar(obj, [&rst](TypeRef type, FieldRef field, ObjectView var) {
		rst.push_back(var);
		return true;
	}, flag);
	return rst;
}

std::optional<TypeRef> ReflMngr::FindType(Type type, const std::function<bool(TypeRef)>& func) const {
	std::optional<TypeRef> rst;
	ForEachTypeInfo(type, [&rst, func](TypeRef type) {
		if (!func(type))
			return true;

		rst.emplace(type);
		return false; // stop
	});
	return rst;
}

std::optional<FieldRef> ReflMngr::FindField(Type type, const std::function<bool(FieldRef)>& func, FieldFlag flag) const {
	std::optional<FieldRef> rst;
	ForEachField(type, [&rst, func](TypeRef type, FieldRef field) {
		if (!func(field))
			return true;

		rst.emplace(field);
		return false; // stop
	}, flag);
	return rst;
}

std::optional<MethodRef> ReflMngr::FindMethod(Type type, const std::function<bool(MethodRef)>& func, FuncFlag flag) const {
	std::optional<MethodRef> rst;
	ForEachMethod(type, [&rst, func](TypeRef type, MethodRef method) {
		if (!func(method))
			return true;

		rst.emplace(method);
		return false; // stop
	}, flag);
	return rst;
}

ObjectView ReflMngr::FindVar(ObjectView obj, const std::function<bool(ObjectView)>& func, FieldFlag flag) const {
	ObjectView rst;
	ForEachVar(obj, [&rst, func](TypeRef type, FieldRef field, ObjectView obj) {
		if (!func(obj))
			return true;

		rst = obj;
		return false; // stop
	}, flag);
	return rst;
}

Type ReflMngr::AddConst(Type type) {
	return tregistry.RegisterAddConst(type);
}

Type ReflMngr::AddLValueReference(Type type) {
	return tregistry.RegisterAddLValueReference(type);
}

Type ReflMngr::AddLValueReferenceWeak(Type type) {
	return tregistry.RegisterAddLValueReferenceWeak(type);
}

Type ReflMngr::AddRValueReference(Type type) {
	return tregistry.RegisterAddRValueReference(type);
}

Type ReflMngr::AddConstLValueReference(Type type) {
	return tregistry.RegisterAddConstLValueReference(type);
}

Type ReflMngr::AddConstRValueReference(Type type) {
	return tregistry.RegisterAddConstRValueReference(type);
}

bool ReflMngr::ContainsBase(Type type, Type base) const {
	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return false;

	const auto& info = target->second;
	if (info.baseinfos.contains(base))
		return true;

	for (const auto& [basetype, baseinfo] : info.baseinfos) {
		bool found = ContainsBase(basetype, base);
		if (found)
			return true;
	}
	return false;
}

bool ReflMngr::ContainsField(Type type, Name field_name, FieldFlag flag) const {
	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return false;

	const auto& info = target->second;
	auto ftarget = info.fieldinfos.find(field_name);
	if (ftarget != info.fieldinfos.end() && enum_contain(flag, ftarget->second.fieldptr.GetFieldFlag()))
		return true;

	for (const auto& [basetype, baseinfo] : info.baseinfos) {
		if (ContainsField(basetype, field_name))
			return true;
	}
	return false;
}

bool ReflMngr::ContainsMethod(Type type, Name method_name, FuncFlag flag) const {
	auto target = typeinfos.find(type);
	if (target == typeinfos.end())
		return false;

	const auto& info = target->second;
	auto [begin_iter, end_iter] = info.methodinfos.equal_range(method_name);
	for (auto iter = begin_iter; iter != end_iter; ++iter) {
		if (enum_contain(flag, iter->second.methodptr.GetFuncFlag()))
			return true;
	}

	for (const auto& [basetype, baseinfo] : info.baseinfos) {
		if (ContainsMethod(basetype, method_name, flag))
			return true;
	}

	return false;
}
