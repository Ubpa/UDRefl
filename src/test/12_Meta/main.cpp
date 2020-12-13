#include <UDRefl/UDRefl.h>

#include <iostream>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Vec {
	float x;
	float y;
	float norm() const noexcept {
		return std::sqrt(x * x + y * y);
	}
	Vec copy() const noexcept {
		return *this;
	}
	Vec operator+(SharedConstObject v) const noexcept {
		Vec rst;
		rst.x = x + v.As<Vec>().x;
		rst.y = y + v.As<Vec>().y;
		return rst;
	}
	Vec operator+(ConstObjectPtr v) const noexcept {
		Vec rst;
		rst.x = x + v.As<Vec>().x;
		rst.y = y + v.As<Vec>().y;
		return rst;
	}
	Vec operator+(float k) const noexcept {
		Vec rst;
		rst.x = x + k;
		rst.y = y + k;
		return rst;
	}
};

int main() {
	ReflMngr::Instance().RegisterTypeAuto<Vec>();
	ReflMngr::Instance().AddField<&Vec::x>("x");
	ReflMngr::Instance().AddField<&Vec::y>("y");
	ReflMngr::Instance().AddMethod<&Vec::norm>("norm");
	ReflMngr::Instance().AddMethod<&Vec::copy>("copy");
	ReflMngr::Instance().AddMethod<MemFuncOf<Vec(ConstObjectPtr)const noexcept>::run(&Vec::operator+)>(StrIDRegistry::Meta::operator_add);
	ReflMngr::Instance().AddMethod<MemFuncOf<Vec(SharedConstObject)const noexcept>::run(&Vec::operator+)>(StrIDRegistry::Meta::operator_add);
	ReflMngr::Instance().AddMethod<MemFuncOf<Vec(float)const noexcept>::run(&Vec::operator+)>(StrIDRegistry::Meta::operator_add);

	// [ or ]
	// ObjectPtr v = ReflMngr::Instance().New(TypeID::of<Vec>);
	// // do something
	// ReflMngr::Instance().Delete(v);
	SharedObject v = ReflMngr::Instance().MakeShared(TypeID::of<Vec>);

	v->RWVar(StrID{ "x" }).As<float>() = 3.f;
	v->RWVar(StrID{ "y" }).As<float>() = 4.f;

	std::cout << "x: " << v->RVar(StrID{ "x" }).As<float>() << std::endl;
	std::cout << "norm: " << v->Invoke<float>(StrID{ "norm" }) << std::endl;
	
	ReflMngr::Instance().ForEachField(
		TypeID::of<Vec>,
		[](TypeRef type, FieldRef field) {
			std::cout << ReflMngr::Instance().nregistry.Nameof(field.ID) << std::endl;
			return true;
		}
	);

	ReflMngr::Instance().ForEachMethod(
		TypeID::of<Vec>,
		[](TypeRef type, Method method) {
			std::cout << ReflMngr::Instance().nregistry.Nameof(method.ID) << std::endl;
			return true;
		}
	);

	v->ForEachRVar(
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
			return true;
		}
	);

	ObjectPtr pv = v;
	auto w0 = v + v;
	auto w1 = v + pv;
	auto w2 = v + 1.f;

	w0->ForEachRVar(
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
			return true;
		}
	);

	w1->ForEachRVar(
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
			return true;
		}
	);

	w2->ForEachRVar(
		[](TypeRef type, FieldRef field, ConstObjectPtr var) {
			std::cout
				<< ReflMngr::Instance().nregistry.Nameof(field.ID)
				<< ": " << var.As<float>()
				<< std::endl;
			return true;
		}
	);
}
