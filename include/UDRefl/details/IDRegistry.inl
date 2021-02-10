#pragma once

#include <cassert>
#include <cstring>

namespace Ubpa::UDRefl {
	template<typename T, typename U>
	IDRegistry<T, U>::IDRegistry() :
		id2name{&resource}
#ifndef NDEBUG
		, unmanagedIDs{&resource}
#endif // !NDEBUG
	{}

	template<typename T, typename U>
	void IDRegistry<T, U>::RegisterUnmanaged(T ID, std::string_view name) {
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

	template<typename T, typename U>
	T IDRegistry<T, U>::RegisterUnmanaged(std::string_view name) {
		T ID{ name };
		RegisterUnmanaged(ID, name);
		return ID;
	}

	template<typename T, typename U>
	std::string_view IDRegistry<T, U>::Register(T ID, std::string_view name) {
		assert(!name.empty());

		auto target = id2name.find(ID);
		if (target != id2name.end()) {
			assert(target->second == name);
			return target->second;
		}

		assert(name.data() && name.data()[name.size()] == 0);

		auto buffer = reinterpret_cast<char*>(resource.allocate(name.size() + 1, alignof(char)));
		std::memcpy(buffer, name.data(), name.size());
		buffer[name.size()] = 0;

		std::string_view new_name{ buffer, name.size() };

		id2name.emplace_hint(target, ID, new_name);

#ifndef NDEBUG
		unmanagedIDs.erase(ID);
#endif // !NDEBUG

		return new_name;
	}

	template<typename T, typename U>
	U IDRegistry<T, U>::Register(std::string_view name) {
		T ID{ name };
		auto new_name = Register(ID, name);
		return { new_name, ID };
	}

	template<typename T, typename U>
	void IDRegistry<T, U>::UnregisterUnmanaged(T ID) {
		auto target = id2name.find(ID);
		if (target == id2name.end())
			return;

		assert(IsUnmanaged(ID));

		id2name.erase(target);
	}

	template<typename T, typename U>
	void IDRegistry<T, U>::Clear() noexcept {
		id2name.clear();
#ifndef NDEBUG
		unmanagedIDs.clear();
#endif // !NDEBUG
		resource.release();
	}

#ifndef NDEBUG
	template<typename T, typename U>
	bool IDRegistry<T, U>::IsUnmanaged(T ID) const {
		return unmanagedIDs.find(ID) != unmanagedIDs.end();
	}

	template<typename T, typename U>
	void IDRegistry<T, U>::ClearUnmanaged() noexcept {
		for (const auto& ID : unmanagedIDs)
			id2name.erase(ID);
		unmanagedIDs.clear();
	}
#endif // !NDEBUG


	template<typename T, typename U>
	bool IDRegistry<T, U>::IsRegistered(T ID) const {
		return id2name.contains(ID);
	}

	template<typename T, typename U>
	std::string_view IDRegistry<T, U>::Nameof(T ID) const {
		if(auto target = id2name.find(ID); target != id2name.end())
			return target->second;
		
		return {};
	}

	template<typename T>
	void TypeIDRegistry::Register() {
		IDRegistry<TypeID, Type>::RegisterUnmanaged(TypeID_of<T>, type_name<T>());
	}

	template<typename T>
	bool TypeIDRegistry::IsRegistered() const {
		static_assert(!std::is_volatile_v<T>);
		return IDRegistry<TypeID, Type>::IsRegistered(TypeID_of<T>);
	}
}
