/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Process utils.

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <shellapi.h>

#include "bstone_assert.h"
#include "bstone_char_traits.h"
#include "bstone_exception.h"
#include "bstone_span.h"
#include "bstone_string_view.h"
#include "bstone_utf.h"

#include "bstone_memory_resource.h"
#include "bstone_process.h"

#include "bstone_win32_unique_resources.h"
#include "bstone_win32_wstring.h"

namespace bstone {
namespace process {

namespace {

Win32WString win32_make_command_line(const CreateAndWaitForExitParam& param, std::intptr_t u16_cl_max_size)
try {
	auto u16_cl = Win32WString{u16_cl_max_size, get_new_delete_memory_resource()};
	auto u16_cl_iter = u16_cl.get_data();
	const auto cl_end = u16_cl.get_data() + u16_cl_max_size;

	for (auto i_arg = std::intptr_t{}; i_arg < param.argc; ++i_arg)
	{
		if (i_arg != 0)
		{
			if (u16_cl_iter + 1 >= cl_end)
			{
				BSTONE_THROW_STATIC_SOURCE("Command-line buffer too small.");
			}

			*u16_cl_iter++ = L' ';
		}

		const auto u8_arg = param.argv[i_arg];
		const auto arg_sv = StringView{u8_arg};
		const auto has_space = arg_sv.contains(' ');
		const auto should_quote = has_space || arg_sv.is_empty();

		if (should_quote)
		{
			BSTONE_ASSERT(u16_cl_iter + 1 < cl_end);
			*u16_cl_iter++ = L'"';
		}

		u16_cl_iter = utf::u8_to_u16(arg_sv.cbegin(), arg_sv.cend(), u16_cl_iter, cl_end);

		if (should_quote)
		{
			BSTONE_ASSERT(u16_cl_iter + 1 < cl_end);
			*u16_cl_iter++ = L'"';
		}
	}

	BSTONE_ASSERT(u16_cl_iter + 1 < cl_end);
	*u16_cl_iter = L'\0';

	return u16_cl;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

CreateAndWaitForExitResult create_and_wait_for_exit(const CreateAndWaitForExitParam& param)
try {
	BSTONE_ASSERT(param.argc >= 0);

	if (param.argc == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("No arguments.");
	}

	// Calculate max length of command line.
	//
	auto u16_cl_max_size = std::intptr_t{};

	for (auto i_arg = std::intptr_t{}; i_arg < param.argc; ++i_arg)
	{
		const auto u8_arg = param.argv[i_arg];
		const auto u16_arg_size = char_traits::get_size(u8_arg);

		u16_cl_max_size += 1; // Argument separator.
		u16_cl_max_size += 2; // Quotation marks.
		u16_cl_max_size += u16_arg_size;
	}

	u16_cl_max_size += 1; // Trailing NUL.

	// Make command line.
	//
	auto u16_cl = win32_make_command_line(param, u16_cl_max_size);

	// Make a working directory.
	//
	auto u16_working_directory = Win32WString{param.working_directory, get_new_delete_memory_resource()};

	// Create a process.
	//
	auto win32_startupinfo = STARTUPINFOW{};
	win32_startupinfo.cb = static_cast<DWORD>(sizeof(STARTUPINFOW));

	auto win32_process_information = PROCESS_INFORMATION{};

	const auto win32_result = CreateProcessW(
		nullptr, // application name
		u16_cl.get_data(), // command line
		nullptr, // process security attributes
		nullptr, // thread security attributes
		FALSE, // inherit handles?
		0, // flags
		nullptr, // environment
		u16_working_directory.get_data(), // working directory
		&win32_startupinfo, // startup info
		&win32_process_information // process info
	);

	if (win32_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create a process.");
	}

	const auto win32_process_handle = Win32HandleUResource{win32_process_information.hProcess};
	const auto win32_thread_handle = Win32HandleUResource{win32_process_information.hThread};

	const auto win32_wait_result = WaitForSingleObject(win32_process_information.hProcess, INFINITE);

	if (win32_wait_result != WAIT_OBJECT_0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to wait for process.");
	}

	auto result = CreateAndWaitForExitResult{};
	auto win32_exit_code = DWORD{};

	if (GetExitCodeProcess(win32_process_handle.get(), &win32_exit_code) != FALSE)
	{
		if (win32_exit_code != STILL_ACTIVE)
		{
			result.exit_code = win32_exit_code;
			result.has_exit_code = true;
		}
	}

	return result;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ===========================================================================

void open_file_or_url(const char* url)
try {
	BSTONE_ASSERT(url != nullptr);

	const auto u16_url = Win32WString{url, get_new_delete_memory_resource()};

	const auto win32_result = ShellExecuteW(
		nullptr, // handle to the parent window
		L"open", // a verb
		u16_url.get_data(), // url
		nullptr, // parameters
		nullptr, // working directory
		SW_SHOWNORMAL // display flags
	);

	if (reinterpret_cast<INT_PTR>(win32_result) <= 32)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to perform an operation.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace process
} // namespace bstone

#endif // _WIN32
