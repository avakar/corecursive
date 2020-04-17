#include <algorithm>
#include <avakar/corecursive.h>
#include <catch2/catch.hpp>
using avakar::corecursive;

TEST_CASE("simple non-recursive int function")
{
	auto fn1 = []() -> corecursive<int> {
		co_return 42;
	};

	REQUIRE(fn1().get() == 42);
	REQUIRE((fn1() == 42));
}

TEST_CASE("simple non-recursive void function")
{
	bool called = false;
	auto fn1 = [&called]() -> corecursive<void> {
		called = true;
		co_return;
	};

	fn1();
	REQUIRE(called);
}

TEST_CASE("simple non-recursive throwing void function")
{
	bool called = false;
	auto fn1 = [&called]() -> corecursive<void> {
		called = true;
		throw 42;
	};

	REQUIRE_THROWS_AS(fn1(), int);
	REQUIRE(called);
	called = false;

	try
	{
		fn1();
	}
	catch (int i)
	{
		REQUIRE(i == 42);
	}

	REQUIRE(called);
}

static corecursive<int> fact(int n)
{
	if (n <= 1)
		co_return 1;

	co_return n * co_await fact(n - 1);
}

TEST_CASE("recursive factorial")
{
	REQUIRE((fact(0) == 1));
	REQUIRE((fact(1) == 1));
	REQUIRE((fact(2) == 2));
	REQUIRE((fact(3) == 6));
	REQUIRE((fact(4) == 24));
	REQUIRE((fact(5) == 120));
	REQUIRE((fact(6) == 720));
}

static corecursive<int> fibo(int n)
{
	if (n <= 1)
		co_return 1;
	int r = (co_await fibo(n - 2)) + (co_await fibo(n - 1));
	co_return r;
}


TEST_CASE("recursive fibonacci")
{
	REQUIRE(fibo(1).get() == 1);
	REQUIRE(fibo(2).get() == 2);
	REQUIRE(fibo(3).get() == 3);
	REQUIRE(fibo(4).get() == 5);
	REQUIRE(fibo(5).get() == 8);
	REQUIRE(fibo(6).get() == 13);
}

template <typename T>
static corecursive<void> quicksort(T * first, T * last)
{
	auto size = last - first;
	if (size <= 1)
		co_return;

	auto pivot = first[(size - 1) / 2];
	auto p = first;
	auto q = last;

	for (;;)
	{
		while (*p < pivot)
			++p;

		while (q[-1] > pivot)
			--q;

		if (p >= q - 1)
			break;

		std::swap(*p, q[-1]);
	}

	co_await quicksort(first, q);
	co_await quicksort(q, last);
}

TEST_CASE("quicksort")
{
	std::vector<int> v = { 7, 79, 90, 32, 30, 50, 55, 23, 78, 96, 28, 17, 57, 64, 70, 11 };
	quicksort(v.data(), v.data() + v.size());
	REQUIRE(std::is_sorted(v.begin(), v.end()));
}
