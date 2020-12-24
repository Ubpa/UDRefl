#include <UDRefl/UDRefl.h>

#include <iostream>
#include <array>
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

	Vec&& move() noexcept {
		return std::move(*this);
	}
};

int main() {
	{ // register Vec
		ReflMngr::Instance().RegisterTypeAuto<Vec>();
		ReflMngr::Instance().AddConstructor<Vec, float, float>();
		ReflMngr::Instance().AddField<&Vec::x>("x");
		ReflMngr::Instance().AddField<&Vec::y>("y");
		ReflMngr::Instance().AddMethod<&Vec::Norm2>("Norm2");
		ReflMngr::Instance().AddMethod<&Vec::NormalizeSelf>("NormalizeSelf");
		ReflMngr::Instance().AddMethod<&Vec::operator+= >(StrIDRegistry::Meta::operator_assign_add);
		ReflMngr::Instance().AddMethod<&Vec::move >("move");
	}

	auto v = ReflMngr::Instance().MakeShared(TypeID::of<Vec>, 1.f, 2.f);

	ReflMngr::Instance().Invoke(v.AsObjectPtr(), StrID{ "NormalizeSelf" });
	std::cout << ReflMngr::Instance().RVar(v.AsObjectPtr(), StrID{"x"}).As<float>() << ", "
		<< ReflMngr::Instance().RVar(v.AsObjectPtr(), StrID{ "y" }).As<float>() << std::endl;

	std::cout << ReflMngr::Instance().Invoke<float>(v.AsObjectPtr(), StrID{ "Norm2" }) << std::endl;

	auto& w = ReflMngr::Instance().Invoke<Vec&, const Vec&>(
		v.AsObjectPtr(),
		StrIDRegistry::MetaID::operator_assign_add,
		Vec{ 10.f,10.f }
	);
	std::cout << w.x << ", " << w.y << std::endl;
	v->DMInvoke("move");
	
	return 0;
}
