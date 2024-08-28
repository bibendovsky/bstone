/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Message box.

#ifndef BSTONE_SYS_MESSAGE_BOX_INCLUDED
#define BSTONE_SYS_MESSAGE_BOX_INCLUDED

#include "bstone_enum_flags.h"
#include "bstone_span.h"

namespace bstone {
namespace sys {

enum class MessageBoxType
{
	none,
	error,
	information,
	warning,
};

// ==========================================================================

enum class MessageBoxButtonFlags : unsigned int
{
	none = 0U,
	default_for_escape_key = 1U << 0,
	default_for_return_key = 1U << 1,
};

BSTONE_ENABLE_ENUM_CLASS_BITWISE_OPS_FOR(MessageBoxButtonFlags)

// ==========================================================================

struct MessageBoxButton
{
	int id;
	MessageBoxButtonFlags flags;
	const char* text;
};

// ==========================================================================

struct MessageBoxInitParam
{
	const char* title;
	const char* message;
	MessageBoxType type;
	Span<const MessageBoxButton> buttons;
};

// ==========================================================================

struct MessageBox
{
	static void show_simple(const char* title, const char* message, MessageBoxType type);

	static int show(const MessageBoxInitParam& param);
};


} // namespace sys
} // namespace bstone

#endif // !BSTONE_SYS_MESSAGE_BOX_INCLUDED
