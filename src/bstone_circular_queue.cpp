/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "bstone_circular_queue.h"

#include "bstone_exception.h"


namespace bstone
{
namespace detail
{


class CircularQueueException :
	public Exception
{
public:
	explicit CircularQueueException(
		const char* message) noexcept
		:
		Exception{"CIRCULAR_QUEUE", message}
	{
	}
}; // CircularQueueException


[[noreturn]]
void circular_queue_fail(
	const char* message)
{
	throw CircularQueueException{message};
}

[[noreturn]]
void circular_queue_fail_nested(
	const char* message)
{
	std::throw_with_nested(CircularQueueException{message});
}


} // detail
} // bstone
