#pragma once

namespace Ubpa::UDRefl {
	template<typename T>
	T ReflMngr::Invoke(TypeID typeID, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer) const {
		std::uint8_t result_buffer[sizeof(T)];
		auto result = Invoke(typeID, methodID, argTypeIDs, args_buffer, result_buffer);
		if (!result.success)
			return {};

		T rst = std::move(buffer_as<T>(result_buffer));
		if (result.destructor)
			result.destructor(result_buffer);
		
		return rst;
	}

	template<typename T>
	T ReflMngr::Invoke(ConstObjectPtr obj, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer) const {
		std::uint8_t result_buffer[sizeof(T)];
		auto result = Invoke(obj, methodID, argTypeIDs, args_buffer, result_buffer);
		if (!result.success)
			return {};

		T rst = std::move(buffer_as<T>(result_buffer));
		if (result.destructor)
			result.destructor(result_buffer);

		return rst;
	}

	template<typename T>
	T ReflMngr::Invoke(ObjectPtr obj, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer) const {
		std::uint8_t result_buffer[sizeof(T)];
		auto result = Invoke(obj, methodID, argTypeIDs, args_buffer, result_buffer);
		if (!result.success)
			return {};

		T rst = std::move(buffer_as<T>(result_buffer));
		if (result.destructor)
			result.destructor(result_buffer);

		return rst;
	}
}
