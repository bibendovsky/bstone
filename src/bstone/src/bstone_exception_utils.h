/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Exception utils.

#ifndef BSTONE_EXCEPTION_UTILS_INCLUDED
#define BSTONE_EXCEPTION_UTILS_INCLUDED

#include <deque>
#include <string>

namespace bstone {

using ExceptionMessages = std::deque<std::string>;

ExceptionMessages extract_exception_messages();

std::string get_nested_message();

} // namespace bstone


#endif // BSTONE_EXCEPTION_UTILS_INCLUDED
