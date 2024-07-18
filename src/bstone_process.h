/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Process utils.

#ifndef BSTONE_PROCESS_INCLUDED
#define BSTONE_PROCESS_INCLUDED

#include <cstdint>

namespace bstone {
namespace process {

struct CreateAndWaitForExitParam
{
	std::intptr_t argc;
	const char** argv;

	const char* working_directory; // (optional)
};

struct CreateAndWaitForExitResult
{
	bool has_exit_code;
	std::int64_t exit_code;
};

// ==========================================================================

CreateAndWaitForExitResult create_and_wait_for_exit(const CreateAndWaitForExitParam& param);

void open_file_or_url(const char* url);

} // namespace process
} // namespace bstone

#endif // BSTONE_PROCESS_INCLUDED
