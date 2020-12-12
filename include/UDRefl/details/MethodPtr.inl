#pragma once

namespace Ubpa::UDRefl {
	inline ObjectPtr ArgsView::At(size_t idx) const noexcept {
		assert(idx < paramList.GetParameters().size());
		return {
			paramList.GetParameters()[idx].typeID,
			forward_offset(buffer, paramList.GetOffsets()[idx])
		};
	}
}
