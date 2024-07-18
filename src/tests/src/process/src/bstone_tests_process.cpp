/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifdef _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
#endif

#include <cstdlib>

#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[])
{
	for (auto i = 1; i < argc; ++i)
	{
		const auto arg = std::string{argv[i]};

		if (false) {}
		else if (arg == "abort")
		{
#ifdef _WIN32
			if (TerminateProcess(GetCurrentProcess(), STILL_ACTIVE) == FALSE)
			{
				std::abort();
			}
#else
			std::abort();
#endif
		}
		else if (arg == "with space")
		{
			return 43;
		}
	}

	return 42;
}
