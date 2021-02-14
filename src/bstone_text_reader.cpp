/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// A text reader for a stream.
//


#include "bstone_text_reader.h"


namespace bstone
{


TextReader::TextReader()
	:
	stream_{},
	is_eos_{},
	buffer_offset_{},
	buffer_size_{},
	buffer_{},
	char_buffer_{}
{
}

TextReader::TextReader(
	TextReader&& rhs)
	:
	stream_{std::move(rhs.stream_)},
	is_eos_{std::move(rhs.is_eos_)},
	buffer_offset_{std::move(rhs.buffer_offset_)},
	buffer_size_{std::move(rhs.buffer_size_)},
	buffer_{std::move(rhs.buffer_)},
	char_buffer_{std::move(rhs.char_buffer_)}
{
	rhs.stream_ = nullptr;
}

TextReader::TextReader(
	Stream* stream)
	:
	TextReader{}
{
	static_cast<void>(open(stream));
}

TextReader::~TextReader()
{
	close();
}

bool TextReader::open(
	Stream* stream)
{
	close();

	if (!stream)
	{
		return false;
	}

	if (!stream->is_readable())
	{
		return false;
	}

	stream_ = stream;

	return true;
}

void TextReader::close()
{
	stream_ = {};
	is_eos_ = {};
	buffer_offset_ = {};
	buffer_size_ = {};
	buffer_ = {};
	char_buffer_ = -1;
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
		return {};
	}

	if (is_eos())
	{
		return {};
	}

	auto line = std::string{};
	line.reserve(max_buffer_size);

	while (true)
	{
		const auto ch = peek_char();

		if (ch < 0)
		{
			break;
		}

		if (ch != '\r' && ch != '\n')
		{
			line += static_cast<char>(ch);
		}
		else
		{
			if (ch == '\n')
			{
				break;
			}
			else if (ch == '\r')
			{
				const auto next_ch = peek_char();

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

int TextReader::peek_char()
{
	if (is_eos())
	{
		return -1;
	}

	if (char_buffer_ >= 0)
	{
		const auto result = char_buffer_;
		char_buffer_ = -1;
		return result;
	}

	if (buffer_offset_ == buffer_size_)
	{
		buffer_offset_ = 0;
		buffer_size_ = stream_->read(buffer_.data(), max_buffer_size);

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


} // bstone
