#include <UDRefl/ranges/MethodRange.hpp>

#include <UDRefl/ReflMngr.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

MethodRange::iterator::iterator(ObjectTree::iterator typeiter, MethodFlag flag) :
	typeiter{ std::move(typeiter) }, flag{ flag }, mode{ typeiter.Valid() ? 0 : -1 }
{
	if (typeiter.Valid())
		update();
}

void MethodRange::iterator::update() {
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
	while (typeiter.Valid()) {
		if (!std::get<TypeInfo*>(*typeiter)) {
			++typeiter;
			continue;
		}

		curmethod = std::get<TypeInfo*>(*typeiter)->methodinfos.begin();
		while (curmethod != std::get<TypeInfo*>(*typeiter)->methodinfos.end()) {
			if (enum_contain_any(flag, curmethod->second.methodptr.GetMethodFlag())) {
				return; // yield
			mode_1:
				;
			}
			++curmethod;
		}

		++typeiter;
	}

	mode = -1;
	return; // stop
}

MethodRange::iterator& MethodRange::iterator::operator++() {
	update();
	return *this;
}

MethodRange::iterator MethodRange::iterator::operator++(int) {
	MethodRange::iterator iter = *this;
	(void)operator++();
	return iter;
}

namespace Ubpa::UDRefl {
	UDRefl_core_API bool operator==(const MethodRange::iterator& lhs, const MethodRange::iterator& rhs) {
		assert(lhs.flag == rhs.flag);
		if (lhs.Valid()) {
			if (rhs.Valid()) {
				if (lhs.typeiter == rhs.typeiter)
					return lhs.curmethod == rhs.curmethod;
				else
					return false;
			}
			else
				return false;
		}
		else if (rhs.Valid())
			return false;
		else
			return lhs.typeiter == rhs.typeiter;
	}

	UDRefl_core_API bool operator!=(const MethodRange::iterator& lhs, const MethodRange::iterator& rhs) {
		return !(lhs == rhs);
	}
}
