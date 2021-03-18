#include <UDRefl/UDRefl.hpp>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

enum class Color {
	Red,
	Green,
	Blue
};

int main() {
	Mngr.RegisterType<Color>();
	Mngr.AddField<Color::Red>("Red");
	Mngr.AddField<Color::Green>("Green");
	Mngr.AddField<Color::Blue>("Blue");


	for (auto&& [name, color] : VarRange_of<Color>)
		std::cout << name.GetView() << ": " << static_cast<int>(color.As<const Color>()) << std::endl;

	// enumerator -> name
	Color c = Color::Red;
	for (auto&& [color_name, color] : VarRange_of<Color, FieldFlag::Unowned>) {
		if (color == c) {
			std::cout << "name of " << static_cast<int>(c) << " : " << color_name.GetView() << std::endl;
			break;
		}
	}

	// name -> enumerator
	std::string_view name = "Green";
	for (auto&& [color_name, color] : VarRange_of<Color, FieldFlag::Unowned>) {
		if (color_name.Is(name)) {
			std::cout << "value of " << name << " : " << static_cast<int>(color.As<const Color>()) << std::endl;
			break;
		}
	}
}
