/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Removes prefixes of C-strings in the file.

#include <cassert>
#include <cstdlib>

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

using Prefix = std::string;
using Prefixes = std::vector<Prefix>;

[[noreturn]] void fail(const std::string& message)
{
	throw std::runtime_error{message};
}

std::string load_file_as_string(const std::string& file_path)
{
	const auto fail_file = [&file_path](const std::string& message)
	{
		auto error_message = std::string{};
		error_message.reserve(file_path.size() + message.size() + 16);
		error_message += "[file_path=\"";
		error_message += file_path;
		error_message += "\"] ";
		error_message += message;
		fail(error_message);
	};

	auto filebuf = std::filebuf{};

	if (filebuf.open(file_path, std::ios_base::in | std::ios_base::binary) == nullptr)
	{
		fail_file("Failed to load.");
	}

	const auto file_size = filebuf.pubseekoff(0, std::ios_base::end);

	if (file_size < 0)
	{
		fail("Failed to set a position to the end.");
	}

	const auto current_position = filebuf.pubseekpos(0);

	if (current_position != 0)
	{
		fail("Failed to rewind.");
	}

	auto string = std::string{};
	string.resize(static_cast<std::size_t>(file_size));

	if (filebuf.sgetn(&string[0], file_size) != file_size)
	{
		fail("Failed to read.");
	}

	return string;
}

void ensure_non_empty_prefixes(const Prefixes& prefixes)
{
	const auto has_empty = std::any_of(
		prefixes.cbegin(),
		prefixes.cend(),
		[](const Prefix& prefix)
		{
			return prefix.empty();
		});

	if (has_empty)
	{
		fail("Empty prefix.");
	}
}

void ensure_unique_prefixes(const Prefixes& prefixes)
{
	const auto set = std::unordered_set<Prefix>{prefixes.cbegin(), prefixes.cend()};

	if (set.size() != prefixes.size())
	{
		fail("Duplicate prefix.");
	}
}

void reorder_prefixes(Prefixes& prefixes)
{
	std::sort(
		prefixes.begin(),
		prefixes.end(),
		[](const Prefix& a, const Prefix& b)
		{
			return a.size() < b.size();
		});
}

void ensure_no_sub_prefixes(const Prefixes& prefixes)
{
	const auto prefix_count = prefixes.size();

	for (auto i = std::size_t{}; i < prefix_count; ++i)
	{
		const auto& sub_prefix = prefixes[i];

		for (auto j = i + 1; j < prefix_count; ++j)
		{
			const auto& prefix = prefixes[j];

			if (prefix.find(sub_prefix) != Prefix::npos)
			{
				auto error_message = std::string{};
				error_message.reserve(prefix.size() + prefix.size() + 32);
				error_message += "Prefix \"";
				error_message += prefix;
				error_message += "\" contains prefix \"";
				error_message += sub_prefix;
				error_message += "\".";
				fail(error_message);
			}
		}
	}
}

void process_prefixes(Prefixes& prefixes)
{
	ensure_non_empty_prefixes(prefixes);
	ensure_unique_prefixes(prefixes);
	reorder_prefixes(prefixes);
	ensure_no_sub_prefixes(prefixes);
}

void move_chars(
	std::size_t old_position,
	std::size_t new_position,
	std::size_t count,
	std::string& file)
{
	if (old_position <= new_position)
	{
		fail("Invalid old position of a prefix.");
	}

	for (auto i = std::size_t{}; i < count; ++i)
	{
		file[new_position + i] = file[old_position + i];
	}
}

int process_file(std::string& file, Prefixes& prefixes)
{
	const auto prefix_count = prefixes.size();

	using Positions = std::vector<std::size_t>;
	auto positions = Positions{};
	positions.resize(prefix_count);

	auto match_count = 0;

	while (true)
	{
		auto found_any_prefix = false;

		for (auto i = std::size_t{}; i < prefix_count; ++i)
		{
			const auto& prefix = prefixes[i];
			auto position = positions[i];
			position = file.find(prefix, position);
			positions[i] = position;

			if (position == Prefix::npos)
			{
				continue;
			}

			++positions[i];

			const auto null_position = file.find('\0', position + prefix.size());

			if (null_position == Prefix::npos)
			{
				continue;
			}

			const auto data_position = position + prefix.size();
			const auto data_size = null_position - data_position;

			if (data_size > 0)
			{
				move_chars(data_position, position, data_size, file);

				std::fill(
					file.begin() + position + data_size,
					file.begin() + null_position,
					'\0');

				++match_count;
			}

			found_any_prefix = true;
		}

		if (!found_any_prefix)
		{
			break;
		}
	}

	return match_count;
}

void save_file(const std::string& file, const std::string& file_path)
{
	const auto fail_file = [&file_path](const std::string& message)
	{
		auto error_message = std::string{};
		error_message.reserve(file_path.size() + message.size() + 16);
		error_message += "[file_path=\"";
		error_message += file_path;
		error_message += "\"] ";
		error_message += message;
		fail(error_message);
	};

	auto filebuf = std::filebuf{};

	if (filebuf.open(file_path, std::ios_base::out | std::ios_base::binary) == nullptr)
	{
		fail_file("Failed to open.");
	}

	const auto stream_size = static_cast<std::streamsize>(file.size());

	if (filebuf.sputn(file.data(), stream_size) != stream_size)
	{
		fail_file("Failed to write.");
	}
}

int main(int argc, char* argv[])
try
{
	std::cout << "BSTONE C-string prefix trimmer." << std::endl;

	if (argc < 3)
	{
		std::cerr << "Usage: app <file> [<prefix> [<prefix> ...]]" << std::endl;
		return EXIT_FAILURE;
	}


	std::cout << "File: \"" << argv[1] << "\"" << std::endl;

	for (auto i = 2; i < argc; ++i)
	{
		std::cout << "Prefix #" << (i - 1) << ": \"" << argv[i] << "\"" << std::endl;
	}

	const auto exe_file_path = std::string{argv[1]};
	auto prefixes = Prefixes{&argv[2], &argv[argc]};
	std::cout << "Prepare prefixes." << std::endl;
	process_prefixes(prefixes);
	std::cout << "Load a file." << std::endl;
	auto file = load_file_as_string(exe_file_path);
	std::cout << "Process the file." << std::endl;
	const auto match_count = process_file(file, prefixes);
	std::cout << "Processed " << match_count << " strings." << std::endl;
	std::cout << "Save the file." << std::endl;
	save_file(file, exe_file_path);

	return EXIT_SUCCESS;
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
