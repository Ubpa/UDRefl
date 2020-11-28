#pragma once

#include "Enumerator.h"

namespace Ubpa::UDRefl {
	struct EnumInfo {
		Enumerator::UnderlyingType underlyingType;
		std::unordered_map<NameID, EnumeratorInfo> enumeratorinfos;
		std::unordered_map<TypeID, Attr> attrs;

		Enumerator GetEnumerator(size_t enumeratorID) const { return { underlyingType, enumeratorinfos.at(enumeratorID).value }; }
		NameID GetEnumeratorNameID(Enumerator::Value value) const {
			for (const auto& [ID, info] : enumeratorinfos) {
				if (value.data_uint64 == info.value.data_uint64)
					return ID;
			}
			return {};
		}
		template<typename E>
		NameID GetEnumeratorNameID(E e) const {
			assert(underlyingType == Enumerator::UnderlyingTypeOf<E>());
			return GetEnumeratorNameID(Enumerator::ValueOf(e));
		}
	};
}
