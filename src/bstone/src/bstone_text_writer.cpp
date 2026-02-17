/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A text writer for a stream

#include "bstone_text_writer.h"

namespace bstone {

TextWriter::TextWriter(Stream* stream)
	:
	TextWriter{}
{
	open(stream);
}

bool TextWriter::open(Stream* stream)
{
	close();
	stream_ = stream;
	return is_open();
}

void TextWriter::close()
{
	stream_ = nullptr;
}

bool TextWriter::is_open() const
{
	return stream_ != nullptr;
}

bool TextWriter::write(const std::string& string)
{
	if (!is_open())
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

} // namespace bstone
