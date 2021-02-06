#pragma once

namespace Ubpa::UDRefl {
	constexpr FieldFlag FieldPtr::GetFieldFlag() const noexcept {
		switch (data.index())
		{
		case 0:
			return FieldFlag::Basic;
		case 1:
			return FieldFlag::Virtual;
		case 2:
			return FieldFlag::Static;
		case 3:
			return FieldFlag::DynamicShared;
		case 4:
			return FieldFlag::DynamicBuffer;
		default:
			return FieldFlag::None;
		}
	}
}
