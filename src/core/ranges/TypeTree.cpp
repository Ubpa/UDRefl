#include <UDRefl/ranges/TypeTree.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

void TypeTree::iterator::update() {
	switch (mode)
	{
	case 0:
		goto mode_0;
	case 1:
		goto mode_1;
	case 2:
		goto mode_2;
	default:
		assert(false);
		return;
	}

mode_0:
	mode = 1;

	std::get<Type>(value) = root;
	std::get<TypeInfo*>(value) = Mngr.GetTypeInfo(root);
	return; // yield
	
mode_1:
	mode = 2;

	if (!std::get<TypeInfo*>(value)) {
		mode = -1;
		return; // stop
	}

	deriveds.emplace_back(std::get<Type>(value), std::get<TypeInfo*>(value));
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

		std::get<Type>(value) = deriveds.back().curbase->first;
		if (auto target = Mngr.typeinfos.find(std::get<Type>(value)); target != Mngr.typeinfos.end())
			std::get<TypeInfo*>(value) = &target->second;
		else
			std::get<TypeInfo*>(value) = nullptr;

		return; // yield

mode_2:
		// push derived
		if (std::get<TypeInfo*>(value)) {
			deriveds.emplace_back(std::get<Type>(value), std::get<TypeInfo*>(value));
			curbase_valid = false;
		}
	}

	mode = -1;
	return; // stop
}

TypeTree::iterator::iterator(Type root, bool begin_or_end) :
	root{ root },
	mode{ begin_or_end? 0:-1 },
	curbase_valid{ true }
{
	if(begin_or_end)
		update();
}

TypeTree::iterator& TypeTree::iterator::operator++() {
	update();
	return *this;
}

TypeTree::iterator TypeTree::iterator::operator++(int) {
	TypeTree::iterator iter = *this;
	(void)operator++();
	return iter;
}

namespace Ubpa::UDRefl {
	bool operator==(const TypeTree::iterator& lhs, const TypeTree::iterator& rhs) {
		assert(lhs.root == rhs.root);
		return lhs.deriveds == rhs.deriveds && lhs.mode == rhs.mode;
	}

	bool operator!=(const TypeTree::iterator& lhs, const TypeTree::iterator& rhs) {
		return !(lhs == rhs);
	}
}
