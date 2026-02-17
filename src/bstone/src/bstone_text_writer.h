/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A text writer for a stream

#ifndef BSTONE_TEXT_WRITER_INCLUDED
#define BSTONE_TEXT_WRITER_INCLUDED

#include "bstone_stream.h"
#include <string>

namespace bstone {

class TextWriter
{
public:
	TextWriter() = default;
	explicit TextWriter(Stream* stream);

	bool open(Stream* stream);
	void close();
	bool is_open() const;
	bool write(const std::string& string);

private:
	Stream* stream_{};
};

} // namespace bstone

#endif // BSTONE_TEXT_WRITER_INCLUDED
