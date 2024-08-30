/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cstdlib>

#include <exception>
#include <iostream>

#include "bstone_tester.h"

int main(int, char*[])
try
{
	auto tester = bstone::Tester{};
	const auto is_succeeded = tester.test();
	return is_succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
}
catch (const std::exception& exception)
{
	std::cerr << "[ERROR] " << exception.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...)
{
	std::cerr << "[ERROR] Generic failure." << std::endl;
	return EXIT_FAILURE;
}
