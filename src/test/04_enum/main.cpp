#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

enum class Color {
	RED,
	GREEN,
	BLUE
};

int main() {
	ReflMngr::Instance().RegisterTypePro<Color>();
	ReflMngr::Instance().AddField<Color::RED>("RED");
	ReflMngr::Instance().AddField<Color::GREEN>("GREEN");
	ReflMngr::Instance().AddField<Color::BLUE>("BLUE");

	ReflMngr::Instance().ForEachRVar(
		TypeID::of<Color>,
		[](Type type, Field field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << static_cast<size_t>(var.As<Color>())
				<< std::endl;
			return true;
		}
	);

	std::string_view name_red;
	ReflMngr::Instance().ForEachRVar(
		TypeID::of<Color>,
		[&name_red](Type type, Field field, ConstObjectPtr var) mutable {
			if (var.As<Color>() == Color::RED) {
				name_red = ReflMngr::Instance().nregistry.Nameof(field.ID);
				return false;
			}
			return true;
		}
	);
	std::cout << "name of COLOR::RED : " << name_red << std::endl;
}
