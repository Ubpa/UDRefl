#pragma once

#include <cassert>
#include <cstring>

namespace Ubpa::UDRefl {
	template<typename T>
	IDRegistry<T>::IDRegistry() :
		id2name{&resource}
#ifndef NDEBUG
		, unmanagedIDs{&resource}
#endif // !NDEBUG
	{}

	template<typename T>
	void IDRegistry<T>::RegisterUnmanaged(T ID, std::string_view name) {
		assert(!name.empty());

		auto target = id2name.find(ID);
		if (target != id2name.end()) {
			assert(target->second == name);
			return;
		}

		assert(name.data() && name.data()[name.size()] == 0);
		id2name.emplace_hint(target, ID, name);

#ifndef NDEBUG
		unmanagedIDs.insert(ID);
#endif // !NDEBUG
	}

	template<typename T>
	T IDRegistry<T>::RegisterUnmanaged(std::string_view name) {
		T ID{ name };
		RegisterUnmanaged(ID, name);
		return ID;
	}

	template<typename T>
	void IDRegistry<T>::Register(T ID, std::string_view name) {
		assert(!name.empty());

		auto target = id2name.find(ID);
		if (target != id2name.end()) {
			assert(target->second == name);
			return;
		}

		assert(name.data() && name.data()[name.size()] == 0);

		auto buffer = reinterpret_cast<char*>(resource.allocate(name.size() + 1, alignof(char)));
		std::memcpy(buffer, name.data(), name.size());
		buffer[name.size()] = 0;

		id2name.emplace_hint(target, ID, std::string_view{ buffer, name.size() });

#ifndef NDEBUG
		unmanagedIDs.erase(ID);
#endif // !NDEBUG
	}

	template<typename T>
	T IDRegistry<T>::Register(std::string_view name) {
		T ID{ name };
		Register(ID, name);
		return ID;
	}

	template<typename T>
	void IDRegistry<T>::UnregisterUnmanaged(T ID) {
		auto target = id2name.find(ID);
		if (target == id2name.end())
			return;

		assert(IsUnmanaged(ID));

		id2name.erase(target);
	}

	template<typename T>
	void IDRegistry<T>::Clear() noexcept {
		id2name.clear();
#ifndef NDEBUG
		unmanagedIDs.clear();
#endif // !NDEBUG
		resource.release();
	}

#ifndef NDEBUG
	template<typename T>
	bool IDRegistry<T>::IsUnmanaged(T ID) const {
		return unmanagedIDs.find(ID) != unmanagedIDs.end();
	}

	template<typename T>
	void IDRegistry<T>::ClearUnmanaged() noexcept {
		for (const auto& ID : unmanagedIDs)
			id2name.erase(ID);
		unmanagedIDs.clear();
	}
#endif // !NDEBUG


	template<typename T>
	bool IDRegistry<T>::IsRegistered(T ID) const {
		return id2name.contains(ID);
	}

	template<typename T>
	std::string_view IDRegistry<T>::Nameof(T ID) const {
		if(auto target = id2name.find(ID); target != id2name.end())
			return target->second;
		
		return {};
	}

	template<typename T>
	void TypeIDRegistry::Register() {
		static_assert(!std::is_volatile_v<T>);
		using U = std::remove_cvref_t<T>;
		IDRegistry<TypeID>::RegisterUnmanaged(TypeID_of<U>, type_name<U>());
		IDRegistry<TypeID>::RegisterUnmanaged(TypeID_of<const U>, type_name<const U>());
		IDRegistry<TypeID>::RegisterUnmanaged(TypeID_of<U&>, type_name<U&>());
		IDRegistry<TypeID>::RegisterUnmanaged(TypeID_of<U&&>, type_name<U&&>());
		IDRegistry<TypeID>::RegisterUnmanaged(TypeID_of<const U&>, type_name<const U&>());
		IDRegistry<TypeID>::RegisterUnmanaged(TypeID_of<const U&&>, type_name<const U&&>());
	}

	template<typename T>
	bool TypeIDRegistry::IsRegistered() const {
		static_assert(!std::is_volatile_v<T>);
		return IDRegistry<TypeID>::IsRegistered(TypeID_of<T>);
	}
}
