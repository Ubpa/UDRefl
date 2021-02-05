#include <UDRefl/FieldPtr.h>

using namespace Ubpa::UDRefl;

ObjectView FieldPtr::Var() noexcept {
	return std::visit([this]<typename T>(T& value) -> ObjectView {
		if constexpr (std::is_same_v<T, size_t>) {
			assert(false);
			return nullptr;
		}
		else if constexpr (std::is_same_v<T, Offsetor>) {
			assert(false);
			return nullptr;
		}
		else if constexpr (std::is_same_v<T, void*>) {
			return { valueID, value };
		}
		else if constexpr (std::is_same_v<T, SharedBuffer>) {
			return { valueID, value.get() };
		}
		else if constexpr (std::is_same_v<T, Buffer>) {
			return { valueID, &value };
		}
		else
			static_assert(always_false<T>);
	}, data);
}

ObjectView FieldPtr::Var(void* obj) {
	return std::visit([obj, this]<typename T>(T& value) -> ObjectView {
		if constexpr (std::is_same_v<T, size_t>) {
			assert(obj);
			return { valueID, forward_offset(obj, value) };
		}
		else if constexpr (std::is_same_v<T, Offsetor>) {
			assert(obj);
			return { valueID, value(obj) };
		}
		else if constexpr (std::is_same_v<T, void*>) {
			return { valueID, value };
		}
		else if constexpr (std::is_same_v<T, SharedBuffer>) {
			return { valueID, value.get() };
		}
		else if constexpr (std::is_same_v<T, Buffer>) {
			return { valueID, &value };
		}
		else
			static_assert(always_false<T>);
	}, data);
}
