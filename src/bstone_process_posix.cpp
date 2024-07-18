/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Process utils.

#ifndef _WIN32

//#define BSTONE_PROCESS_DEBUG

#include "bstone_process.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <memory>

#ifdef BSTONE_PROCESS_DEBUG
#include <exception>
#include <iostream>
#endif // BSTONE_PROCESS_DEBUG

#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "bstone_assert.h"
#include "bstone_char_traits.h"
#include "bstone_exception.h"
#include "bstone_fs.h"
#include "bstone_memory_resource.h"
#include "bstone_span.h"

#include "bstone_posix_string.h"

extern char** environ;

namespace bstone {
namespace process {

namespace {

constexpr auto posix_url_command_name = "xdg-open";

PosixString posix_get_url_command_path(const char* url_command_name)
try
{
	BSTONE_ASSERT(url_command_name != nullptr && url_command_name[0] != '\0');
	const auto path_env = getenv("PATH");

	if (!path_env)
	{
		BSTONE_THROW_STATIC_SOURCE("No \"PATH\" environment variable.");
	}

	const auto url_command_name_span = Span<const char>
	{
		url_command_name,
		char_traits::get_size(url_command_name)
	};

	auto path_env_it = path_env;

	while (true)
	{
		auto is_nul = false;
		auto is_separator = false;
		auto path_env_end_it = path_env_it;

		while (!is_nul && !is_separator)
		{
			switch (*path_env_end_it)
			{
				case ':':
					is_separator = true;
					break;

				case '\0':
					is_nul = true;
					is_separator = true;
					break;

				default:
					path_env_end_it += 1;
					break;
			}
		}

		const auto env_path_span = make_span(path_env_it, path_env_end_it - path_env_it);

		if (!env_path_span.is_empty())
		{
			const auto path_size = url_command_name_span.get_size() + env_path_span.get_size() + 2;
			auto path = PosixString{path_size, get_new_delete_memory_resource()};
			auto path_it = path.get_data();
			path_it = std::copy(env_path_span.begin(), env_path_span.end(), path_it);

			if (!env_path_span.is_empty() && env_path_span.get_back() != '/')
			{
				*path_it++ = '/';
			}

			path_it = std::copy(url_command_name_span.begin(), url_command_name_span.end(), path_it);
			*path_it = '\0';

			struct stat posix_stat;
			const auto posix_stat_result = stat(path.get_data(), &posix_stat);

			if (posix_stat_result == 0)
			{
				return path;
			}
		}

		if (is_nul)
		{
			break;
		}

		path_env_it = path_env_end_it + 1;
	}

	BSTONE_THROW_STATIC_SOURCE("URL command not found.");
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void posix_close_pipes(int (&pipe_fds)[2])
{
	close(pipe_fds[0]);
	close(pipe_fds[1]);
	pipe_fds[0] = 0;
	pipe_fds[1] = 0;
}

void posix_create_auto_close_pipe(int (&pipe_fds)[2])
try
{
	pipe_fds[0] = 0;
	pipe_fds[1] = 0;
	int result[2];

	if (pipe(result) != 0)
	{
		constexpr auto message = "Failed to create a pipe.";
#ifdef BSTONE_PROCESS_DEBUG
		std::cerr << message << std::endl;
#endif // #if BSTONE_PROCESS_DEBUG
		BSTONE_THROW_STATIC_SOURCE(message);
	}

	// Set flag to close pipe automatically on successful exec*.
	//
	for (auto i = 0; i < 2; ++i)
	{
		// Get current flags.
		//
		auto fd_flags = fcntl(result[i], F_GETFD);

		if (fd_flags < 0)
		{
			posix_close_pipes(result);

			constexpr auto message = "Failed to get fd flags.";
#ifdef BSTONE_PROCESS_DEBUG
			std::cerr << message << std::endl;
#endif // #if BSTONE_PROCESS_DEBUG
			BSTONE_THROW_STATIC_SOURCE(message);
		}

		// Set new flags if the flag is not set.
		//
		if ((fd_flags & FD_CLOEXEC) == 0)
		{
			fd_flags |= FD_CLOEXEC;
			const auto fcntl_result = fcntl(result[i], F_SETFD, fd_flags);

			if (fcntl_result < 0)
			{
				posix_close_pipes(result);

				constexpr auto message = "Failed to set FD flags.";
#ifdef BSTONE_PROCESS_DEBUG
				std::cerr << message << std::endl;
#endif // #if BSTONE_PROCESS_DEBUG
				BSTONE_THROW_STATIC_SOURCE(message);
			}
		}
	}

	pipe_fds[0] = result[0];
	pipe_fds[1] = result[1];
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

CreateAndWaitForExitResult create_and_wait_for_exit(const CreateAndWaitForExitParam& param)
{
#ifdef BSTONE_PROCESS_DEBUG
	std::cout << "[process::create_and_wait_for_exit]" << std::endl;
#endif // BSTONE_PROCESS_DEBUG

	if (param.argc == 0)
	{
		constexpr auto error_message = "No arguments.";
#ifdef BSTONE_PROCESS_DEBUG
		std::cerr << error_message << std::endl;
#endif // BSTONE_PROCESS_DEBUG
		BSTONE_THROW_STATIC_SOURCE(error_message);
	}

	// Get maximum size of the arguments and environment variables.
	//
	const auto arg_max = sysconf(_SC_ARG_MAX);

	if (arg_max <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get ARG_MAX limit.");
	}

	// Calculate arguments size.
	//
	auto max_arguments_size = std::intptr_t{};

	for (auto i_arg = 0; i_arg < param.argc; ++i_arg)
	{
		const auto arg = param.argv[i_arg];
		max_arguments_size += char_traits::get_size(arg) + 1;
	}

	max_arguments_size += 1; // Terminating NUL.

	// Calculate environment variables size.
	//
	BSTONE_ASSERT(environ != nullptr);
	auto max_env_size = std::intptr_t{};

	for (auto i = std::intptr_t{}; ; ++i)
	{
		const auto env = environ[i];

		if (env == nullptr)
		{
			break;
		}

		max_env_size += char_traits::get_size(env) + 1;
	}

	max_env_size += 1; // Terminating NULL.

	if ((max_arguments_size + max_env_size) > arg_max)
	{
		constexpr auto message = "Combined size of arguments and environment too big.";
#ifdef BSTONE_PROCESS_DEBUG
		std::cerr << message << std::endl;
#endif // BSTONE_PROCESS_DEBUG
		BSTONE_THROW_STATIC_SOURCE(message);
	}

	// Build arguments.
	//
	using HeapArgsItem = char*;
	using HeapArgs = std::unique_ptr<HeapArgsItem[]>;

	constexpr auto max_stack_args = 127;
	char* stack_argv[max_stack_args + 1];
	auto heap_argv = HeapArgs{};
	auto argv = static_cast<char**>(nullptr);

	if (param.argc <= max_stack_args)
	{
		argv = stack_argv;
	}
	else
	{
		heap_argv = std::make_unique<HeapArgsItem[]>(param.argc + 1);
		argv = heap_argv.get();
	}

	for (auto i = std::intptr_t{}; i < param.argc; ++i)
	{
		argv[i] = const_cast<char*>(param.argv[i]);
	}

	argv[param.argc] = nullptr;

	// Create pipes for IPC.
	//
	int pipe_fds[2];
	posix_create_auto_close_pipe(pipe_fds);

	// Fork the process.
	//
	const auto pid = fork();

	if (pid < 0)
	{
		// Fork failure.
		//
		posix_close_pipes(pipe_fds);

		constexpr auto message = "Failed to fork.";
#ifdef BSTONE_PROCESS_DEBUG
		std::cerr << message << std::endl;
#endif // BSTONE_PROCESS_DEBUG
		BSTONE_THROW_STATIC_SOURCE(message);
	}
	else if (pid == 0)
	{
		// Child process.
		//

		// Close a read end.
		//
		close(pipe_fds[0]);
		char child_failure_flag = 1;

		// Change working directory.
		//
		if (param.working_directory && param.working_directory[0] != '\0')
		{
			auto is_working_directory_set = false;

			try
			{
				fs::set_working_directory(param.working_directory);
				is_working_directory_set = true;
			}
#ifdef BSTONE_PROCESS_DEBUG
			catch (const std::exception& exception)
			{
				std::cerr << "Failed to change working directory." << std::endl;
				std::cerr << exception.what() << std::endl;
			}
#endif // BSTONE_PROCESS_DEBUG
			catch (...)
			{
#ifdef BSTONE_PROCESS_DEBUG
				std::cerr << "Failed to change working directory." << std::endl;
#endif // BSTONE_PROCESS_DEBUG
			}

			if (!is_working_directory_set)
			{
				// Notify the parent about failure.
				//
				const auto write_result = write(pipe_fds[1], &child_failure_flag, 1);

				if (write_result != 1)
				{
#ifdef BSTONE_PROCESS_DEBUG
					std::cerr << "\"write\" failed." << std::endl;
#endif // BSTONE_PROCESS_DEBUG
				}

				std::abort();
			}
		}

		// Replace the image.
		//
		execve(param.argv[0], argv, environ);

		// Failed to execute.
		//
#ifdef BSTONE_PROCESS_DEBUG
		std::cerr << "execve failed." << std::endl;
#endif // BSTONE_PROCESS_DEBUG

		// Notify the parent about failure.
		//
		const auto write_result = write(pipe_fds[1], &child_failure_flag, 1);

		if (write_result != 1)
		{
#ifdef BSTONE_PROCESS_DEBUG
			std::cerr << "\"write\" failed." << std::endl;
#endif // BSTONE_PROCESS_DEBUG
		}

		std::abort();
	}
	else
	{
		// Parent process.
		//

		// Close a write end.
		//
		close(pipe_fds[1]);

		// Wait for exit the child process.
		//
		// TODO Exit code.
		int waitpid_status = 0;
		const auto waitpid_result = waitpid(pid, &waitpid_status, 0);

		// Get notification about failure (if exists).
		//
		auto child_failure_flag = '\0';
		const auto child_read_result = read(pipe_fds[0], &child_failure_flag, 1);
		close(pipe_fds[0]);

		if (child_read_result > 0 && child_failure_flag != '\0')
		{
			constexpr auto message = "Child failed.";
#ifdef BSTONE_PROCESS_DEBUG
			std::cerr << message << std::endl;
#endif // BSTONE_PROCESS_DEBUG
			BSTONE_THROW_STATIC_SOURCE(message);
		}

		auto result = CreateAndWaitForExitResult{};

		if (waitpid_result == pid)
		{
			if (WIFEXITED(waitpid_status))
			{
				result.exit_code = WEXITSTATUS(waitpid_status);
				result.has_exit_code = true;
			}
#ifdef BSTONE_PROCESS_DEBUG
			else
			{
				std::cerr << "Child process exited abnormally." << std::endl;
			}
#endif // BSTONE_PROCESS_DEBUG
		}
#ifdef BSTONE_PROCESS_DEBUG
		else
		{
			std::cerr << "waitpid result pid mismatch." << std::endl;
		}
#endif // BSTONE_PROCESS_DEBUG

#ifdef BSTONE_PROCESS_DEBUG
		std::cout << "[process::create_and_wait_for_exit] Succeed." << std::endl;
#endif // BSTONE_PROCESS_DEBUG

		return result;
	}
}

// ==========================================================================

void open_file_or_url(const char* url_utf8)
{
#ifdef BSTONE_PROCESS_DEBUG
	std::cout << "[process::open_file_or_url]" << std::endl;
#endif // BSTONE_PROCESS_DEBUG

	BSTONE_ASSERT(url_utf8 != nullptr);

	// Find the command.
	//
	auto command_path = posix_get_url_command_path(posix_url_command_name);

	// Create pipes for IPC.
	//
	int pipe_fds[2] = {};
	posix_create_auto_close_pipe(pipe_fds);
	// Fork the process.
	//
	const auto pid = fork();

	if (pid < 0)
	{
		// Fork failure.
		//
		posix_close_pipes(pipe_fds);

		constexpr auto message = "Failed to fork.";
#ifdef BSTONE_PROCESS_DEBUG
		std::cerr << "Failed to fork." << std::endl;
#endif // BSTONE_PROCESS_DEBUG
		BSTONE_THROW_STATIC_SOURCE(message);
	}
	else if (pid == 0)
	{
		// Child process.
		//

		// Close a read end.
		//
		close(pipe_fds[0]);
		char child_failure_flag = 1;

		// Replace the image.
		//
		execle(command_path.get_data(), posix_url_command_name, url_utf8, nullptr, environ);

		// Failed to execute.
		//
#ifdef BSTONE_PROCESS_DEBUG
		std::cerr << "execle failed." << std::endl;
#endif // BSTONE_PROCESS_DEBUG

		// Notify the parent about failure.
		//
		const auto write_result = write(pipe_fds[1], &child_failure_flag, 1);

		if (write_result != 1)
		{
#ifdef BSTONE_PROCESS_DEBUG
			std::cerr << "write failed." << std::endl;
#endif // BSTONE_PROCESS_DEBUG
		}

		std::abort();
	}
	else
	{
		// Parent process.
		//

		// Close a write end.
		//
		close(pipe_fds[1]);

		// Get notification about failure (if exists).
		//
		auto child_failure_flag = '\0';
		const auto child_read_result = read(pipe_fds[0], &child_failure_flag, 1);
		close(pipe_fds[0]);

		if (child_read_result > 0 && child_failure_flag)
		{
			constexpr auto message = "Child failed.";
#ifdef BSTONE_PROCESS_DEBUG
			std::cerr << message << std::endl;
#endif // BSTONE_PROCESS_DEBUG
			BSTONE_THROW_STATIC_SOURCE(message);
		}

#ifdef BSTONE_PROCESS_DEBUG
		std::cout << "[process::open_file_or_url] Succeed." << std::endl;
#endif // BSTONE_PROCESS_DEBUG
	}
}

} // namespace process
} // namespace bstone

#endif // _WIN32
