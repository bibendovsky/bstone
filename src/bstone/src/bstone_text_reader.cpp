/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A text reader for a stream

#include "bstone_text_reader.h"

namespace bstone {

TextReader::TextReader(Stream* stream)
{
	open(stream);
}

bool TextReader::open(Stream* stream)
{
	close();
	stream_ = stream;
	return is_open();
}

void TextReader::close()
{
	stream_ = nullptr;
	buffer_offset_ = 0;
	buffer_size_ = 0;
	char_buffer_ = -1;
	is_eos_ = false;
}

bool TextReader::is_open() const
{
	return stream_ != nullptr;
}

bool TextReader::is_eos() const
{
	return is_eos_;
}

std::string TextReader::read_line()
{
	if (!is_open())
	{
		return std::string{};
	}
	std::string line{};
	line.reserve(max_buffer_size);
	while (true)
	{
		const int ch = fetch_char();
		if (ch < 0)
		{
			break;
		}
		if (ch != '\r' && ch != '\n')
		{
			line.push_back(static_cast<char>(ch));
		}
		else
		{
			if (ch == '\n')
			{
				break;
			}
			else if (ch == '\r')
			{
				const int next_ch = fetch_char();
				if (next_ch == '\n')
				{
					break;
				}
				else
				{
					char_buffer_ = next_ch;
				}
			}
		}
	}
	return line;
}

int TextReader::fetch_char()
{
	if (is_eos())
	{
		return -1;
	}
	if (char_buffer_ >= 0)
	{
		const int result = char_buffer_;
		char_buffer_ = -1;
		return result;
	}
	if (buffer_offset_ == buffer_size_)
	{
		buffer_offset_ = 0;
		buffer_size_ = static_cast<int>(stream_->read(buffer_.data(), max_buffer_size));
		if (buffer_size_ == 0)
		{
			is_eos_ = true;
			return -1;
		}
	}
	const int result = buffer_[buffer_offset_];
	buffer_offset_ += 1;
	return result;
}

} // namespace bstone
