#include <UDRefl/Method.h>

using namespace Ubpa::UDRefl;

ParamList::ParamList(std::vector<Parameter> params) :
	params{ params }
{
	offsets.reserve(params.size());

	size_t curOffset = 0;
	for (const auto& param : params) {
		if (param.alignment > alignment)
			alignment = param.alignment;

		curOffset = (curOffset + param.alignment - 1) / param.alignment;
		offsets.push_back(curOffset);
		curOffset += param.size;
	}

	size = curOffset;
}
