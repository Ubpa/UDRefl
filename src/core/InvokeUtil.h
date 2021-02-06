#pragma once

#include <UDRefl/Util.h>
#include <UDRefl/config.h>

#include <UTemplate/Type.h>

#include <span>

namespace Ubpa::UDRefl::details {
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
	bool IsPriorityCompatible(std::span<const Type> params, std::span<const Type> argTypes);

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
	bool IsNonCopiedArgConstructCompatible(std::span<const Type> params, std::span<const Type> argTypes);

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
	bool IsNonCopiedArgConstructCompatible(std::span<const Type> params, std::span<const TypeID> argTypeIDs);

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
		static_assert(sizeof(ArgInfo)* MaxArgNum < 16384);
	public:
		NewArgsGuard(
			bool is_priority,
			std::pmr::memory_resource* rsrc,
			std::span<const Type> paramTypeIDs,
			std::span<const Type> argTypes,
			ArgPtrBuffer orig_argptr_buffer);

		~NewArgsGuard();

		NewArgsGuard(const NewArgsGuard&) = delete;
		NewArgsGuard& operator=(NewArgsGuard&&) noexcept = delete;

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
}
