/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if defined(_WIN32)

#if !defined(UNICODE)
#define UNICODE
#endif

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#define BSTONE_ENTRY_POINT_IMPLEMENTATION

#include <cstdlib>
#include <windows.h>
#include <processenv.h>
#include <shellapi.h>
#include <memory>
#include "bstone_assert.h"
#include "bstone_entry_point.h"

namespace bstone {
namespace {

auto g_process_heap = HANDLE{};

struct LocalMemoryDeleter
{
	void operator()(void* block) noexcept
	{
		const auto local_free = [](void* block)
		{
			return LocalFree(block);
		};

#if defined(NDEBUG)
		local_free(block);
#else
		const auto result = local_free(block);
		BSTONE_ASSERT(result == nullptr);
#endif
	}
};

using WArgvUPtr = std::unique_ptr<LPWSTR[], LocalMemoryDeleter>;

struct HeapMemoryDeleter
{
	void operator()(void* block) noexcept
	{
		const auto heap_free = [](void* block)
		{
			return HeapFree(g_process_heap, 0, block);
		};

		BSTONE_ASSERT(g_process_heap != nullptr);

#if defined(NDEBUG)
		heap_free(block);
#else
		const auto result = heap_free(block);
		BSTONE_ASSERT(result == TRUE);
#endif
	}
};

using HeapUPtr = std::unique_ptr<void, HeapMemoryDeleter>;

int get_utf16_to_utf8_size_with_null(LPCWSTR u16_string)
{
	return WideCharToMultiByte(CP_UTF8, 0, u16_string, -1, nullptr, 0, nullptr, nullptr);
}

void show_error_message_box(LPCWSTR message)
{
	const auto message_box_w = [](LPCWSTR message)
	{
		return MessageBoxW(nullptr, message, L"BSTONE_ENTRY_POINT", MB_OK | MB_ICONERROR);
	};

#if defined(NDEBUG)
	message_box_w(message);
#else
	const auto result = message_box_w(message);
	BSTONE_ASSERT(result != 0);
#endif
}

void show_error_message_box()
{
	const auto error_code = GetLastError();

	constexpr auto max_message_chars = DWORD{2048};
	WCHAR message[max_message_chars]{};

	const auto format_message_w_result = FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		message,
		max_message_chars,
		nullptr);

	if (format_message_w_result == 0)
	{
		lstrcpyW(message, L"Generic failure.");
	}

	show_error_message_box(message);
}

int windows_entry_point(LPWSTR lpCmdLine)
{
	// Convert command-line string into UTF-16 arguments.
	//
	int wargc = 0;
	auto wargv = WArgvUPtr{CommandLineToArgvW(lpCmdLine, &wargc)};

	if (wargv == nullptr)
	{
		show_error_message_box();
		return EXIT_FAILURE;
	}

	// Calculate the size of UTF-8 arguments block.
	//
	const auto args_ptrs_size = (wargc + 1) * sizeof(void*); // argv[argc] == null
	auto args_block_size = args_ptrs_size;

	for (auto i = 0; i < wargc; ++i)
	{
		const auto u8_size_with_null = get_utf16_to_utf8_size_with_null(wargv[i]);

		if (u8_size_with_null == 0)
		{
			show_error_message_box();
			return EXIT_FAILURE;
		}

		args_block_size += static_cast<SIZE_T>(u8_size_with_null);
	}

	// Allocate the UTF-8 arguments block.
	//
	g_process_heap = GetProcessHeap();

	const auto argv_block = HeapUPtr{HeapAlloc(g_process_heap, 0, args_block_size)};

	if (argv_block == nullptr)
	{
		show_error_message_box();
		return EXIT_FAILURE;
	}

	// Initialize the UTF-8 block.
	//
	const auto argv_ptrs_begin = static_cast<char**>(argv_block.get());
	const auto argv_args_begin = static_cast<char*>(argv_block.get()) + args_ptrs_size;

	auto argv_ptrs = argv_ptrs_begin;
	auto argv_args = argv_args_begin;

	for (auto i = 0; i < wargc; ++i)
	{
		const auto u8_size_with_null = get_utf16_to_utf8_size_with_null(wargv[i]);

		if (u8_size_with_null == 0)
		{
			show_error_message_box();
			return EXIT_FAILURE;
		}

		// UTF-16 -> UTF-8
		const auto u8_used_size = WideCharToMultiByte(
			CP_UTF8,
			0,
			wargv[i],
			-1,
			argv_args,
			u8_size_with_null,
			nullptr,
			nullptr);

		if (u8_used_size != u8_size_with_null)
		{
			show_error_message_box(L"UTF-16 to UTF-8 conversion mismatch size.");
			return EXIT_FAILURE;
		}

		(*argv_ptrs++) = argv_args;
		argv_args += u8_size_with_null;
	}

	*argv_ptrs = nullptr; // argv[argc] = null

	// Release a memory allocated by the CommandLineToArgvW.
	//
	wargv.reset();

	// Call the user's entry point.
	//
	return bstone_entry_point(wargc, argv_ptrs_begin);
}

int windows_entry_point()
{
	return windows_entry_point(GetCommandLineW());
}

} // namespace
} // namespace bstone

int main(int, char**)
{
	return bstone::windows_entry_point();
}

#if !defined(_In_)
#define _In_
#endif

#if !defined(_In_opt_)
#define _In_opt_
#endif

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	return bstone::windows_entry_point();
}

#endif // _WIN32
