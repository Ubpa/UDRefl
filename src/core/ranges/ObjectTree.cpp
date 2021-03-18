#include <UDRefl/ranges/ObjectTree.hpp>

#include <UDRefl/ReflMngr.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void ObjectTree::iterator::update() {
	switch (mode)
	{
	case 0:
		goto mode_0;
	case 1:
		goto mode_1;
	default:
		assert(false);
		return;
	}

mode_0:
	mode = 1;

	if (!std::get<TypeInfo*>(value)) {
		mode = -1;
		return; // stop
	}

	deriveds.push_back({
		.obj = std::get<ObjectView>(value),
		.typeinfo = std::get<TypeInfo*>(value)
	});

	curbase_valid = false;

	while (!deriveds.empty()) {
		// update curbase

		if (curbase_valid)
			++deriveds.back().curbase;
		else {
			deriveds.back().curbase = deriveds.back().typeinfo->baseinfos.begin();
			curbase_valid = true;
		}

		// get not visited base

		while (deriveds.back().curbase != deriveds.back().typeinfo->baseinfos.end()) {
			if (!deriveds.back().curbase->second.IsVirtual())
				break;

			if (std::find(visitedVBs.begin(), visitedVBs.end(), deriveds.back().curbase->first) == visitedVBs.end()) {
				visitedVBs.push_back(deriveds.back().curbase->first);
				break;
			}

			++deriveds.back().curbase;
		}

		// check base

		if (deriveds.back().curbase == deriveds.back().typeinfo->baseinfos.end()) {
			deriveds.pop_back();
			continue;
		}

		// get result
		
		std::get<ObjectView>(value) = {
			deriveds.back().curbase->first,
			deriveds.back().obj.GetPtr() ? deriveds.back().curbase->second.StaticCast_DerivedToBase(deriveds.back().obj.GetPtr()) : nullptr
		};

		if (auto target = Mngr.typeinfos.find(std::get<ObjectView>(value).GetType()); target != Mngr.typeinfos.end())
			std::get<TypeInfo*>(value) = &target->second;
		else
			std::get<TypeInfo*>(value) = nullptr;

		return; // yield

mode_1:
		// push derived
		if (std::get<TypeInfo*>(value)) {
			deriveds.push_back({
				.obj = std::get<ObjectView>(value),
				.typeinfo = std::get<TypeInfo*>(value)
			});
			curbase_valid = false;
		}
	}

	mode = -1;
	return; // stop
}

ObjectTree::iterator::iterator(ObjectView obj, bool begin_or_end) :
	mode{ begin_or_end? 0:-1 },
	curbase_valid{ true }
{
	if (begin_or_end) {
		auto target = Mngr.typeinfos.find(obj.GetType());
		value = { target == Mngr.typeinfos.end() ? nullptr : &target->second, obj };
	}
}

ObjectTree::iterator& ObjectTree::iterator::operator++() {
	update();
	return *this;
}

ObjectTree::iterator ObjectTree::iterator::operator++(int) {
	ObjectTree::iterator iter = *this;
	(void)operator++();
	return iter;
}

namespace Ubpa::UDRefl {
	UDRefl_core_API bool operator==(const ObjectTree::iterator& lhs, const ObjectTree::iterator& rhs) {
		return lhs.deriveds == rhs.deriveds && lhs.mode == rhs.mode;
	}

	UDRefl_core_API bool operator!=(const ObjectTree::iterator& lhs, const ObjectTree::iterator& rhs) {
		return !(lhs == rhs);
	}
}
