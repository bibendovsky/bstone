/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// A text writer for a stream.
//


#include "bstone_text_writer.h"


namespace bstone
{


TextWriter::TextWriter()
	:
	stream_{}
{
}

TextWriter::TextWriter(
	Stream* stream)
	:
	TextWriter{}
{
	static_cast<void>(open(stream));
}

TextWriter::TextWriter(
	TextWriter&& rhs) noexcept
	:
	stream_{std::move(rhs.stream_)}
{
	rhs.stream_ = nullptr;
}

bool TextWriter::open(
	Stream* stream)
{
	close();

	if (!stream)
	{
		return false;
	}

	stream_ = stream;

	return true;
}

void TextWriter::close()
{
	stream_ = nullptr;
}

bool TextWriter::is_initialized() const
{
	return stream_ != nullptr;
}

bool TextWriter::write(
	const std::string& string)
{
	if (!is_initialized())
	{
		return false;
	}

	if (string.empty())
	{
		return true;
	}

	const auto to_write_size = static_cast<std::intptr_t>(string.length());
	return stream_->write(string.data(), to_write_size) == to_write_size;
}


} // bstone
