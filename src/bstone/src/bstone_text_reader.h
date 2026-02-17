/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A text reader for a stream

#ifndef BSTONE_TEXT_READER_INCLUDED
#define BSTONE_TEXT_READER_INCLUDED

#include "bstone_stream.h"
#include <array>
#include <string>

namespace bstone {

class TextReader
{
public:
	TextReader() = default;
	explicit TextReader(Stream* stream);

	bool open(Stream* stream);
	void close();
	bool is_open() const;
	bool is_eos() const;
	std::string read_line();

private:
	static constexpr int max_buffer_size = 1024;

	using Buffer = std::array<char, max_buffer_size>;

	Buffer buffer_{};
	Stream* stream_{};
	int buffer_offset_{};
	int buffer_size_{};
	int char_buffer_{};
	bool is_eos_{};

	int fetch_char();
};

} // namespace bstone

#endif // BSTONE_TEXT_READER_INCLUDED
