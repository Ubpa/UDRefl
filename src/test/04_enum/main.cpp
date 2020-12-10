#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa::UDRefl;

enum class Color {
	RED,
	GREEN,
	BLUE
};

int main() {
	auto ID_Color = ReflMngr::Instance().tregistry.Register<Color>();
	auto ID_RED = ReflMngr::Instance().nregistry.Register("RED");
	auto ID_GREEN = ReflMngr::Instance().nregistry.Register("GREEN");
	auto ID_BLUE = ReflMngr::Instance().nregistry.Register("BLUE");

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
	std::cout << ReflMngr::Instance().enuminfos.at(ID_Color).enumeratorinfos.at(ReflMngr::Instance().nregistry.Register("GREEN")).value.data_int32 << std::endl;

	// value -> name
	std::cout << ReflMngr::Instance().nregistry.Nameof(ReflMngr::Instance().enuminfos.at(ID_Color).GetEnumeratorStrID(Color::GREEN)) << std::endl;
}
