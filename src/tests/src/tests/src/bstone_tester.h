/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_TESTER_INCLUDED
#define BSTONE_TESTER_INCLUDED

#include <deque>

namespace bstone {

using TesterFunc = void (*)();

class Tester
{
public:
	Tester()
	{
		get_registry();
	}

	void register_test(const char* description, TesterFunc func);

	[[noreturn]] void fail(const char* message);
	void check(bool value, const char* message);
	void check(bool value);

	bool test();

private:
	struct RegistryItem
	{
		const char* description;
		TesterFunc tester_func;
	};

	using Registry = std::deque<RegistryItem>;

	static Registry& get_registry();

	static void log_exception();
};

} // namespace bstone

#endif // BSTONE_TESTER_INCLUDED
