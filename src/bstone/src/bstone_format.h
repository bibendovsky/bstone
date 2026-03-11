/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Formatting utilities

#ifndef BSTONE_FORMAT_INCLUDED
#define BSTONE_FORMAT_INCLUDED

#include <string>

namespace bstone {

class StdStringFormatIterator
{
public:
	using difference_type = std::string::iterator::difference_type;

	explicit StdStringFormatIterator(std::string& storage)
		:
		storage_{&storage}
	{}

	void operator=(char value)
	{
		storage_->push_back(value);
	}

	StdStringFormatIterator& operator*()
	{
		return *this;
	}

	StdStringFormatIterator& operator++()
	{
		return *this;
	}

	StdStringFormatIterator& operator++(int)
	{
		return *this;
	}

private:
	std::string* storage_;
};

} // namespace bstone

#endif // BSTONE_FORMAT_INCLUDED
