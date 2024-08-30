/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// A text reader for a stream.
//


#ifndef BSTONE_TEXT_READER_INCLUDED
#define BSTONE_TEXT_READER_INCLUDED


#include <array>
#include <string>

#include "bstone_stream.h"


namespace bstone
{


class TextReader final
{
public:
	TextReader();

	TextReader(
		Stream* stream);

	TextReader(
		const TextReader& rhs) = delete;

	TextReader(
		TextReader&& rhs) noexcept;

	TextReader& operator=(
		const TextReader& rhs) = delete;

	~TextReader();


	bool open(
		Stream* stream);

	void close();

	bool is_open() const;


	bool is_eos() const;


	std::string read_line();


private:
	static constexpr auto max_buffer_size = 4096;


	using Buffer = std::array<char, max_buffer_size>;


	Stream* stream_;
	bool is_eos_;
	int buffer_offset_;
	int buffer_size_;
	Buffer buffer_;
	int char_buffer_;


	int peek_char();
}; // TextReader


} // bstone


#endif // !BSTONE_TEXT_READER_INCLUDED
