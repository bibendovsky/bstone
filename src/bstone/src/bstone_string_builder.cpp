/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// String builder.

#include "bstone_string_builder.h"
#include <cstddef>

namespace bstone {

const std::string& StringBuilder::get_string() const
{
	return message_;
}

void StringBuilder::reserve(int capacity)
{
	message_.reserve(static_cast<std::size_t>(capacity));
}

void StringBuilder::increase_indent()
{
	indent_.resize(indent_.size() + indent_step, ' ');
}

void StringBuilder::decrease_indent()
{
	if (!indent_.empty())
	{
		indent_.resize(indent_.size() - indent_step, ' ');
	}
}

void StringBuilder::reset_indent()
{
	indent_.clear();
}

void StringBuilder::add_indent()
{
	add(indent_);
}

void StringBuilder::add_line()
{
	add('\n');
}

} // namespace bstone
