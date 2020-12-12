#pragma once

#include <cassert>

namespace Ubpa::UDRefl {
	template<typename T>
	void IDRegistry<T>::RegisterUnmanaged(T ID, std::string_view name) {
		id2name[ID] = name;
#ifndef NDEBUG
		unmanagedIDs.insert(ID);
#endif // !NDEBUG
	}

	template<typename T>
	void IDRegistry<T>::Register(T ID, std::string_view name) {
		auto target = id2name.find(ID);
		if (target != id2name.end()) {
			assert(target->second == name);
			return;
		}

		if (name.empty()) {
			id2name[ID] = {};
			return;
		}

		assert(name.data());

		auto buffer = reinterpret_cast<char*>(resource.allocate(name.size(), 1));
		memcpy(buffer, name.data(), name.size());

		id2name[ID] = std::string_view{ buffer, name.size() };

#ifndef NDEBUG
		unmanagedIDs.erase(ID);
#endif // !NDEBUG
	}

	template<typename T>
	void IDRegistry<T>::UnregisterUnmanaged(T ID) noexcept {
		auto target = id2name.find(ID);
		if (target == id2name.end())
			return;

		assert(IsUnmanaged(ID));

		id2name.erase(ID);
	}

	template<typename T>
	void IDRegistry<T>::Clear() {
		id2name.clear();
		unmanagedIDs.clear();
		resource.release();
	}

#ifndef NDEBUG
	template<typename T>
	bool IDRegistry<T>::IsUnmanaged(T ID) const noexcept {
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
	bool IDRegistry<T>::IsRegistered(T ID) const noexcept {
		return id2name.find(ID) != id2name.end();
	}

	template<typename T>
	std::string_view IDRegistry<T>::Nameof(T ID) const noexcept {
		auto target = id2name.find(ID);
		if (target != id2name.end())
			return target->second;
		else
			return {};
	}
}
