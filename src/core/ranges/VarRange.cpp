#include <UDRefl/ranges/VarRange.h>

#include <UDRefl/ReflMngr.h>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

VarRange::iterator::iterator(Type root, void* ptr, bool begin_or_end, CVRefMode cvref_mode, FieldFlag flag) :
	root{ root },
	cvref_mode{ cvref_mode },
	flag{ flag },
	curbase_valid{true},
	type{ root },
	typeinfo{nullptr},
	ptr{ ptr },
	visited_curtype{ true },
	mode{ begin_or_end?0:-1 }
{
	assert(root.GetCVRefMode() == CVRefMode::None);
	assert(!enum_contain_any(cvref_mode, CVRefMode::Volatile));
	if (begin_or_end)
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

	type = root;
	typeinfo = Mngr.GetTypeInfo(root);
	curbase_valid = false;
	visited_curtype = false;

	while (!visited_curtype || !deriveds.empty()) {
		// update type, ptr, typeinfo

		if (visited_curtype) {
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
				ptr = deriveds.back().ptr;
				deriveds.pop_back();
				continue;
			}

			// derived -> base

			type = deriveds.back().curbase->first;

			ptr = deriveds.back().curbase->second.StaticCast_DerivedToBase(ptr);

			if (auto target = Mngr.typeinfos.find(type); target != Mngr.typeinfos.end())
				typeinfo = &target->second;
			else
				typeinfo = nullptr;
		}
		else
			visited_curtype = true;

		// iterate fields

		if (typeinfo) {
			curfield = typeinfo->fieldinfos.begin();
			while (curfield != typeinfo->fieldinfos.end()) {
				if (enum_contain_any(flag, curfield->second.fieldptr.GetFieldFlag())) {
					std::get<Name>(value) = curfield->first;
					{ // set var
						ObjectView var = curfield->second.fieldptr.Var(ptr);
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

			// push derived
			deriveds.emplace_back(type, typeinfo, std::unordered_map<Type, BaseInfo>::iterator{}, ptr);
			curbase_valid = false;
		}
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
	bool operator==(const VarRange::iterator& lhs, const VarRange::iterator& rhs) {
		assert(lhs.root == rhs.root);
		assert(lhs.cvref_mode == rhs.cvref_mode);
		assert(lhs.flag == rhs.flag);

		if (lhs.Valid()) {
			if (rhs.Valid()) {
				if (lhs.deriveds == rhs.deriveds)
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
			return lhs.deriveds == rhs.deriveds;
	}

	bool operator!=(const VarRange::iterator& lhs, const VarRange::iterator& rhs) {
		return !(lhs == rhs);
	}
}
