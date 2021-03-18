#include <UDRefl/UDRefl.hpp>

#include <iostream>
#include <cmath>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Vec {
	float x;
	float y;

	float Norm2() const noexcept {
		return x * x + y * y;
	}

	void NormalizeSelf() noexcept {
		float n = std::sqrt(Norm2());
		assert(n != 0);
		x /= n;
		y /= n;
	}

	Vec& operator+=(const Vec& p) noexcept {
		x += p.x;
		y += p.y;
		return *this;
	}
};

int main() {
	{ // register Vec
		Mngr.RegisterType<Vec>();
		Mngr.AddConstructor<Vec, float, float>();
		Mngr.AddField<&Vec::x>("x");
		Mngr.AddField<&Vec::y>("y");
		Mngr.AddMethod<&Vec::Norm2>("Norm2");
		Mngr.AddMethod<&Vec::NormalizeSelf>("NormalizeSelf");
		Mngr.AddMethod<&Vec::operator+= >(NameIDRegistry::Meta::operator_assignment_add);
	}

	auto v = Mngr.MakeShared(Type_of<Vec>, TempArgsView{ 1.f, 2.f });

	v.Invoke("NormalizeSelf");
	std::cout << v.Var("x") << ", " << v.Var("y") << std::endl;

	std::cout << v.Invoke("Norm2") << std::endl;

	auto w = v += Vec{ 10.f,10.f };
	std::cout << w.Var("x") << ", " << w.Var("y") << std::endl;
	
	return 0;
}
