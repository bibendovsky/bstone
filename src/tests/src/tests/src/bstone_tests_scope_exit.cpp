#include<utility>

#include "bstone_tester.h"

#include "bstone_scope_exit.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// ScopeExit(TFunctor&&)
void test_rwporsiu1oplbx1f()
{
	auto flag = false;

	{
		auto functor = [&flag]()
		{
			flag = true;
		};

		bstone::ScopeExit<decltype(functor)> scope_exit{std::move(functor)};
	}

	tester.check(flag);
}

// ScopeExit(ScopeExit&&)
void test_92b6hm7ci1fyeqr7()
{
	auto flag = false;

	{
		auto functor = [&flag]()
		{
			flag = true;
		};

		auto scope_exit_1 = bstone::ScopeExit<decltype(functor)>{std::move(functor)};
		auto scope_exit = std::move(scope_exit_1);
	}

	tester.check(flag);
}

// release()
void test_fgjatoojvmbaaqzl()
{
	auto flag = true;

	{
		auto functor = [&flag]()
		{
			flag = false;
		};

		bstone::ScopeExit<decltype(functor)> scope_exit{std::move(functor)};
		scope_exit.release();
	}

	tester.check(flag);
}

// ==========================================================================

// make_scope_exit(TFunctor&&)
void test_zgxhthepnqrzpmjc()
{
	auto flag = false;

	{
		const auto scope_exit = bstone::make_scope_exit(
			[&flag]()
			{
				flag = true;
			});
	}

	tester.check(flag);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_scope_exit();
		register_make_scope_exit();
	}

private:
	void register_scope_exit()
	{
		tester.register_test("scope_exit#rwporsiu1oplbx1f", test_rwporsiu1oplbx1f);
		tester.register_test("scope_exit#92b6hm7ci1fyeqr7", test_92b6hm7ci1fyeqr7);
		tester.register_test("scope_exit#fgjatoojvmbaaqzl", test_fgjatoojvmbaaqzl);
	}

	void register_make_scope_exit()
	{
		tester.register_test("make_scope_exit#zgxhthepnqrzpmjc", test_zgxhthepnqrzpmjc);
	}
};

auto registrator = Registrator{};

} // namespace
