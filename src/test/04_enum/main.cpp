#include <UDRefl/UDRefl.h>

#include <iostream>
#include <cassert>

using namespace Ubpa::UDRefl;

enum class Color {
	RED,
	GREEN,
	BLUE
};

int main() {
	size_t ID_Color = ReflMngr::Instance().registry.Register("Color");
	size_t ID_RED = ReflMngr::Instance().registry.Register("RED");
	size_t ID_GREEN = ReflMngr::Instance().registry.Register("GREEN");
	size_t ID_BLUE = ReflMngr::Instance().registry.Register("BLUE");

	{ // register Color
		EnumeratorInfo einfo_RED{ Color::RED };
		EnumeratorInfo einfo_GREEN{ Color::GREEN };
		EnumeratorInfo einfo_BLUE{ Color::BLUE };
		EnumInfo enuminfo{
			Enumerator::UnderlyingTypeOf<Color>(),
			{ // enumerators
				{ID_RED  , einfo_RED},
				{ID_GREEN, einfo_GREEN},
				{ID_BLUE , einfo_BLUE},
			}
		};

		ReflMngr::Instance().enuminfos.emplace(ID_Color, std::move(enuminfo));
	}
	
	// name -> value
	static_assert(std::is_same_v<std::underlying_type_t<Color>, std::int32_t>);
	std::cout << ReflMngr::Instance().enuminfos.at(ID_Color).enumeratorinfos.at(ReflMngr::Instance().registry.GetID("GREEN")).value.data_int32 << std::endl;

	// value -> name
	std::cout << ReflMngr::Instance().registry.Nameof(ReflMngr::Instance().enuminfos.at(ID_Color).GetID(Color::GREEN)) << std::endl;
}
