#include <UDRefl/UDRefl.h>
#include <iostream>
#include <cmath>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Object {
	void f(double) {
		std::cout << "f(double)" << std::endl;
	}
	void f(std::uint8_t&) {
		std::cout << "f(std::uint8_t&)" << std::endl;
	}
	void f(const std::uint8_t&) {
		std::cout << "f(const std::uint8_t&)" << std::endl;
	}
	void f(std::uint8_t&&) {
		std::cout << "f(std::uint8_t&&)" << std::endl;
	}
	void f(const std::uint8_t&&) {
		std::cout << "f(const std::uint8_t&&)" << std::endl;
	}
};

int main() {
	ReflMngr::Instance().RegisterType<Object>();
	ReflMngr::Instance().AddMethod<MemFuncOf<void(double)>::get(&Object::f)>("f");
	ReflMngr::Instance().AddMethod<MemFuncOf<void(std::uint8_t&)>::get(&Object::f)>("f");
	ReflMngr::Instance().AddMethod<MemFuncOf<void(const std::uint8_t&)>::get(&Object::f)>("f");
	ReflMngr::Instance().AddMethod<MemFuncOf<void(std::uint8_t&&)>::get(&Object::f)>("f");
	ReflMngr::Instance().AddMethod<MemFuncOf<void(const std::uint8_t&&)>::get(&Object::f)>("f");

	SharedObject obj = ReflMngr::Instance().MakeShared(TypeID_of<Object>);

	std::uint8_t i = 1;
	const std::uint8_t ci = 1;
	obj->Invoke<void>("f", 1.);
	obj->Invoke<void>("f", 1.f);
	obj->Invoke<void>("f", 1);
	obj->Invoke<void>("f", i);
	obj->Invoke<void>("f", ci);
	obj->Invoke<void>("f", std::move(i));
	obj->Invoke<void>("f", static_cast<const std::uint8_t&&>(ci));
}
