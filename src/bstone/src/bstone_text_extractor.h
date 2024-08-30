/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_TEXT_EXTRACTOR_INCLUDED
#define BSTONE_TEXT_EXTRACTOR_INCLUDED

#include <string>
#include <vector>
#include "jm_cio.h"

namespace bstone {

class TextExtractor
{
public:
	TextExtractor();

	void extract_text(const std::string& dst_dir);

private:
	[[noreturn]] void fail(int number, const char* message);

private:
	struct TextNumber
	{
		bool is_compressed_;
		int number_;
	};

	using TextNumbers = std::vector<TextNumber>;
	using Buffer = std::vector<std::uint8_t>;

	TextNumbers text_numbers_;
	Buffer buffer_;

private:
	void initialize_text();
	CompHeader_t deserialize_header(int number, const std::uint8_t* data);
	void extract_text(const std::string& dst_dir, const TextNumber& number);
};

} // namespace bstone

#endif // BSTONE_TEXT_EXTRACTOR_INCLUDED
