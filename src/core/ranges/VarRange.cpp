#include <UDRefl/ranges/VarRange.hpp>

#include <UDRefl/ReflMngr.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

VarRange::iterator::iterator(ObjectTree::iterator typeiter, CVRefMode cvref_mode, FieldFlag flag) :
	typeiter{ typeiter },
	cvref_mode{ cvref_mode },
	flag{ flag },
	mode{ typeiter.Valid()?0:-1 }
{
	assert(!enum_contain_any(cvref_mode, CVRefMode::Volatile));
	if (typeiter.Valid())
		update();
}

void VarRange::iterator::update() {
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

		curfield = std::get<TypeInfo*>(*typeiter)->fieldinfos.begin();
		while (curfield != std::get<TypeInfo*>(*typeiter)->fieldinfos.end()) {
			if (enum_contain_any(flag, curfield->second.fieldptr.GetFieldFlag())) {
				std::get<Name>(value) = curfield->first;
				{ // set var
					ObjectView var = curfield->second.fieldptr.Var(std::get<ObjectView>(*typeiter).GetPtr());
					switch (cvref_mode)
					{
					case Ubpa::CVRefMode::Left:
						std::get<ObjectView>(value) = var.AddLValueReference();
						break;
					case Ubpa::CVRefMode::Right:
						std::get<ObjectView>(value) = var.AddRValueReference();
						break;
					case Ubpa::CVRefMode::Const:
						std::get<ObjectView>(value) = var.AddConst();
						break;
					case Ubpa::CVRefMode::ConstLeft:
						std::get<ObjectView>(value) = var.AddConstLValueReference();
						break;
					case Ubpa::CVRefMode::ConstRight:
						std::get<ObjectView>(value) = var.AddConstRValueReference();
						break;
					default:
						std::get<ObjectView>(value) = var;
						break;
					}
				}
				return; // yield
			mode_1:
				;
			}
			++curfield;
		}

		++typeiter;
	}

	mode = -1;
	return; // stop
}

VarRange::iterator& VarRange::iterator::operator++() {
	update();
	return *this;
}

VarRange::iterator VarRange::iterator::operator++(int) {
	VarRange::iterator iter = *this;
	(void)operator++();
	return iter;
}

namespace Ubpa::UDRefl {
	UDRefl_core_API bool operator==(const VarRange::iterator& lhs, const VarRange::iterator& rhs) {
		assert(lhs.flag == rhs.flag);
		assert(lhs.cvref_mode == rhs.cvref_mode);
		if (lhs.Valid()) {
			if (rhs.Valid()) {
				if (lhs.typeiter == rhs.typeiter)
					return lhs.curfield == rhs.curfield;
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

	UDRefl_core_API bool operator!=(const VarRange::iterator& lhs, const VarRange::iterator& rhs) {
		return !(lhs == rhs);
	}
}
