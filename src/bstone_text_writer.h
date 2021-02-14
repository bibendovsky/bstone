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
// A text writer for a stream.
//


#ifndef BSTONE_TEXT_WRITER_INCLUDED
#define BSTONE_TEXT_WRITER_INCLUDED


#include <string>

#include "bstone_stream.h"


namespace bstone
{


// A binary reader for a block of memory.
class TextWriter final
{
public:
	TextWriter();

	TextWriter(
		Stream* stream);

	TextWriter(
		TextWriter&& rhs);


	// Opens the writer.
	bool open(
		Stream* stream);

	// Closes the writer.
	void close();

	// Returns true if the writer is initialized or
	// false otherwise.
	bool is_initialized() const;

	// Writes a string.
	//
	// Returns true on success or false otherwise.
	bool write(
		const std::string& string);


private:
	Stream* stream_;
}; // TextWriter


} // bstone


#endif // !BSTONE_TEXT_WRITER_INCLUDED
