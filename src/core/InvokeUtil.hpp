#pragma once

#include <UDRefl/Util.hpp>
#include <UDRefl/config.hpp>

#include <UTemplate/Type.hpp>

#include <span>

namespace Ubpa::UDRefl::details {
	// parameter <- argument
	// - same
	// - reference
	// > - 0 (invalid), 1 (convertible)
	// > - table
	//     |    -     | T | T & | const T & | T&& | const T&& | const T |
	//     |      T   | - |  0  |     0     |  1  |     0     |    0    |
	//     |      T & | 0 |  -  |     0     |  0  |     0     |    0    |
	//     |const T & | 0 |  0  |     -     |  0  |     0     |    1    |
	//     |      T&& | 1 |  0  |     0     |  -  |     0     |    0    |
	//     |const T&& | 0 |  0  |     0     |  0  |     -     |    0    |
	bool IsPriorityCompatible(std::span<const Type> params, std::span<const Type> argTypes);

	// parameter <- argument
	// - same
	// - reference
	// > - 0 (invalid), 1 (convertible)
	// > - table
	//     |     -     | T | T & | const T & | T&& | const T&& | const T |
	//     |       T   | - |  0  |     0     |  1  |     0     |    0    |
	//     |       T & | 0 |  -  |     0     |  0  |     0     |    0    |
	//     | const T & | 1 |  1  |     -     |  1  |     1     |    1    |
	//     |       T&& | 1 |  0  |     0     |  -  |     0     |    0    |
	//     | const T&& | 1 |  0  |     0     |  1  |     -     |    1    |
	bool IsRefCompatible(std::span<const Type> paramTypes, std::span<const Type> argTypes);

	// parameter <- argument
	// - same
	// - reference
	// > - 0 (invalid), 1 (convertible)
	// > - table
	//     |     -     | T | T & | const T & | T&& | const T&& | const T |
	//     |       T   | - |  0  |     0     |  1  |     0     |    0    |
	//     |       T & | 0 |  -  |     0     |  0  |     0     |    0    |
	//     | const T & | 1 |  1  |     -     |  1  |     1     |    1    |
	//     |       T&& | 1 |  0  |     0     |  -  |     0     |    0    |
	//     | const T&& | 1 |  0  |     0     |  1  |     -     |    1    |
	bool IsRefCompatible(std::span<const Type> paramTypes, std::span<const TypeID> argTypeIDs);

	bool IsRefConstructible(Type paramType, std::span<const Type> argTypes);
	bool RefConstruct(ObjectView obj, ArgsView args);

	class BufferGuard {
	public:
		BufferGuard() : rsrc{ nullptr }, size{ 0 }, alignment{ 0 }, buffer{ nullptr }{}
		BufferGuard(std::pmr::memory_resource* rsrc, std::size_t size, std::size_t alignment) :
			rsrc{ rsrc },
			size{ size },
			alignment{ alignment },
			buffer{ rsrc->allocate(size, alignment) } {}

		~BufferGuard() {
			if (rsrc)
				rsrc->deallocate(buffer, size, alignment);
		}

		void* Get() const noexcept { return buffer; }
		operator void* () const noexcept { return Get(); }

		BufferGuard(const BufferGuard&) = delete;
		BufferGuard& operator=(BufferGuard&&) noexcept = delete;
	private:
		std::pmr::memory_resource* rsrc;
		std::size_t size;
		std::size_t alignment;
		void* buffer;
	};

	class NewArgsGuard {
		struct ArgInfo {
			const char* name;
			std::size_t name_hash;
			std::uint32_t offset;
			std::uint16_t name_size;
			std::uint8_t idx;
			bool is_pointer_or_array;
			Type GetType() const noexcept {
				return { std::string_view{name,name_size}, TypeID{name_hash} };
			}
		}; // 24 bytes
		// MaxArgNum <= 2^8
		static_assert(sizeof(ArgInfo)* MaxArgNum < 16384);
	public:
		NewArgsGuard(
			bool is_priority,
			std::pmr::memory_resource* rsrc,
			std::span<const Type> paramTypes,
			ArgsView args);

		~NewArgsGuard();

		NewArgsGuard(const NewArgsGuard&) = delete;
		NewArgsGuard& operator=(NewArgsGuard&&) noexcept = delete;

		bool IsCompatible() const noexcept { return is_compatible; }

		ArgsView GetArgsView() const noexcept {
			assert(IsCompatible());
			return new_args;
		}

	private:
		bool is_compatible{ false };
		BufferGuard buffer;
		std::span<ArgInfo> nonptr_arg_infos;
		ArgsView new_args;
		BufferGuard type_buffer;
	};
}
