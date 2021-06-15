#include <gtest/gtest.h>

#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

struct Point { float x, y; };

class PointTest : public testing::Test {
public:
	void SetUp() override {
		Mngr.RegisterType<Point>();
		Mngr.AddField<&Point::x>("x");
		Mngr.AddField<&Point::y>("y");
	}
	virtual void TearDown() {
		Mngr.typeinfos.erase(Type_of<Point>);
	}
};

TEST_F(PointTest, Ctor) {
	SharedObject p = Mngr.MakeShared(Type_of<Point>);
	EXPECT_TRUE(p.GetType().Valid());
	EXPECT_EQ(p.GetType(), Type_of<Point>);
	EXPECT_NE(p.GetPtr(), nullptr);
}

TEST_F(PointTest, Var) {
	Point p;
	ObjectView obj{ p };
	obj.Var("x") = 1.f;
	obj.Var("y") = 2.f;
	EXPECT_EQ(obj.Var("x"), 1.f);
	EXPECT_EQ(obj.Var("y"), 2.f);
}

TEST_F(PointTest, GetVars) {
	Point p{ 1.f,2.f };
	ObjectView obj{ p };
	auto vars = obj.GetVars();
	auto iter_begin = vars.begin();
	auto iter_end = vars.end();

	auto cursor = iter_begin;

	{
		EXPECT_NE(cursor, iter_end);
		const auto& [name, var] = *cursor;
		if (name == "x")
			EXPECT_EQ(var, 1.f);
		else if (name == "y")
			EXPECT_EQ(var, 2.f);
		else
			EXPECT_TRUE(false);
	}
	++cursor;
	{
		EXPECT_NE(cursor, iter_end);
		const auto& [name, var] = *cursor;
		if (name == "x")
			EXPECT_EQ(var, 1.f);
		else if (name == "y")
			EXPECT_EQ(var, 2.f);
		else
			EXPECT_TRUE(false);
	}
	++cursor;
	EXPECT_EQ(cursor, iter_end);
}

struct Base {};
struct Derived : Base {};

class InheritanceTest : public testing::Test {
public:
	void SetUp() override {
		Mngr.RegisterType<Base>();
		Mngr.RegisterType<Derived>();
		Mngr.AddBases<Derived, Base>();
		Mngr.RegisterType<InheritanceTest>();
		Mngr.AddStaticMethod(Type_of<InheritanceTest>, "func0", &InheritanceTest::func0);
		Mngr.AddStaticMethod(Type_of<InheritanceTest>, "func1", &InheritanceTest::func1);
		Mngr.AddStaticMethod(Type_of<InheritanceTest>, "func2", &InheritanceTest::func2);
		Mngr.AddStaticMethod(Type_of<InheritanceTest>, "func3", &InheritanceTest::func3);
		Mngr.AddStaticMethod(Type_of<InheritanceTest>, "func4", &InheritanceTest::func4);
	}
	virtual void TearDown() {
		Mngr.typeinfos.erase(Type_of<Derived>);
		Mngr.typeinfos.erase(Type_of<Base>);
		Mngr.typeinfos.erase(Type_of<InheritanceTest>);
	}

	static void func0(Base) {}
	static void func1(Base&) {}
	static void func2(Base&&) {}
	static void func3(const Base&) {}
	static void func4(const Base&&) {}
};
TEST_F(InheritanceTest, IsCompatible) {
	EXPECT_TRUE(Mngr.IsCompatible(Types_of<const Base&>, Types_of<Derived>));
	EXPECT_TRUE(Mngr.IsCompatible(Types_of<const Base&>, Types_of<Derived&>));
	EXPECT_TRUE(Mngr.IsCompatible(Types_of<const Base&>, Types_of<Derived&&>));
	EXPECT_TRUE(Mngr.IsCompatible(Types_of<const Base&>, Types_of<const Derived&>));
	EXPECT_TRUE(Mngr.IsCompatible(Types_of<const Base&>, Types_of<const Derived&&>));

	EXPECT_FALSE(Mngr.IsCompatible(Types_of<Base&>, Types_of<Derived>));
	EXPECT_TRUE(Mngr.IsCompatible(Types_of<Base&>, Types_of<Derived&>));
	EXPECT_FALSE(Mngr.IsCompatible(Types_of<Base&>, Types_of<Derived&&>));
	EXPECT_FALSE(Mngr.IsCompatible(Types_of<Base&>, Types_of<const Derived&>));
	EXPECT_FALSE(Mngr.IsCompatible(Types_of<Base&>, Types_of<const Derived&&>));

	EXPECT_TRUE(Mngr.IsCompatible(Types_of<const Base&&>, Types_of<Derived>));
	EXPECT_FALSE(Mngr.IsCompatible(Types_of<const Base&&>, Types_of<Derived&>));
	EXPECT_TRUE(Mngr.IsCompatible(Types_of<const Base&&>, Types_of<Derived&&>));
	EXPECT_FALSE(Mngr.IsCompatible(Types_of<const Base&&>, Types_of<const Derived&>));
	EXPECT_TRUE(Mngr.IsCompatible(Types_of<const Base&&>, Types_of<const Derived&&>));

	EXPECT_TRUE(Mngr.IsCompatible(Types_of<Base&&>, Types_of<Derived>));
	EXPECT_FALSE(Mngr.IsCompatible(Types_of<Base&&>, Types_of<Derived&>));
	EXPECT_TRUE(Mngr.IsCompatible(Types_of<Base&&>, Types_of<Derived&&>));
	EXPECT_FALSE(Mngr.IsCompatible(Types_of<Base&&>, Types_of<const Derived&>));
	EXPECT_FALSE(Mngr.IsCompatible(Types_of<Base&&>, Types_of<const Derived&&>));

	EXPECT_TRUE(Mngr.IsCompatible(Types_of<Base>, Types_of<Derived>));
	EXPECT_FALSE(Mngr.IsCompatible(Types_of<Base>, Types_of<Derived&>));
	EXPECT_TRUE(Mngr.IsCompatible(Types_of<Base>, Types_of<Derived&&>));
	EXPECT_FALSE(Mngr.IsCompatible(Types_of<Base>, Types_of<const Derived&>));
	EXPECT_FALSE(Mngr.IsCompatible(Types_of<Base>, Types_of<const Derived&&>));
}
TEST_F(InheritanceTest, Invoke) {
	Derived derived;
	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func0", TempArgsView{ static_cast<Derived>(derived) }).GetType().Valid());
	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func0", TempArgsView{ static_cast<Derived&>(derived) }).GetType().Valid());
	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func0", TempArgsView{ static_cast<const Derived&>(derived) }).GetType().Valid());
	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func0", TempArgsView{ static_cast<Derived&&>(derived) }).GetType().Valid());
	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func0", TempArgsView{ static_cast<const Derived&&>(derived) }).GetType().Valid());

	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func1", TempArgsView{ static_cast<Derived>(derived) }).GetType().Valid());
	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func1", TempArgsView{ static_cast<Derived&>(derived) }).GetType().Valid());
	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func1", TempArgsView{ static_cast<const Derived&>(derived) }).GetType().Valid());
	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func1", TempArgsView{ static_cast<Derived&&>(derived) }).GetType().Valid());
	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func1", TempArgsView{ static_cast<const Derived&&>(derived) }).GetType().Valid());

	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func2", TempArgsView{ static_cast<Derived>(derived) }).GetType().Valid());
	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func2", TempArgsView{ static_cast<Derived&>(derived) }).GetType().Valid());
	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func2", TempArgsView{ static_cast<const Derived&>(derived) }).GetType().Valid());
	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func2", TempArgsView{ static_cast<Derived&&>(derived) }).GetType().Valid());
	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func2", TempArgsView{ static_cast<const Derived&&>(derived) }).GetType().Valid());

	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func3", TempArgsView{ static_cast<Derived>(derived) }).GetType().Valid());
	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func3", TempArgsView{ static_cast<Derived&>(derived) }).GetType().Valid());
	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func3", TempArgsView{ static_cast<const Derived&>(derived) }).GetType().Valid());
	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func3", TempArgsView{ static_cast<Derived&&>(derived) }).GetType().Valid());
	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func3", TempArgsView{ static_cast<const Derived&&>(derived) }).GetType().Valid());

	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func4", TempArgsView{ static_cast<Derived>(derived) }).GetType().Valid());
	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func4", TempArgsView{ static_cast<Derived&>(derived) }).GetType().Valid());
	EXPECT_FALSE(ObjectView_of<InheritanceTest>.Invoke("func4", TempArgsView{ static_cast<const Derived&>(derived) }).GetType().Valid());
	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func4", TempArgsView{ static_cast<Derived&&>(derived) }).GetType().Valid());
	EXPECT_TRUE(ObjectView_of<InheritanceTest>.Invoke("func4", TempArgsView{ static_cast<const Derived&&>(derived) }).GetType().Valid());
}