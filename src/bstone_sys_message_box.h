/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_MESSAGE_BOX_INCLUDED)
#define BSTONE_SYS_MESSAGE_BOX_INCLUDED

#include "bstone_enum_flags.h"
#include "bstone_span.h"

namespace bstone {
namespace sys {

enum class MessageBoxType
{
	none = 0,
	error,
	information,
	warning,
};

enum class MessageBoxButtonFlags : unsigned int
{
	none = 0U,
	default_for_escape_key = 1U << 0,
	default_for_return_key = 1U << 1,
};

BSTONE_ENABLE_ENUM_CLASS_BITWISE_OPS_FOR(MessageBoxButtonFlags)

struct MessageBoxButtonDescriptor
{
	int id;
	MessageBoxButtonFlags flags;
	const char* text;
};

struct MessageBoxDescriptor
{
	MessageBoxType type;
	const char* title;
	const char* message;
	Span<const MessageBoxButtonDescriptor> buttons;
};

void show_message_box(const char* title, const char* message, MessageBoxType type);
int show_message_box(const MessageBoxDescriptor& descriptor);

} // namespace sys
} // namespace bstone

#endif // !BSTONE_SYS_MESSAGE_BOX_INCLUDED
