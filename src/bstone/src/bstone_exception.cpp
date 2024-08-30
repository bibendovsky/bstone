/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Exception classes.

#include <cstdint>

#include <algorithm>
#include <limits>
#include <memory>
#include <type_traits>

#include "bstone_assert.h"
#include "bstone_char_traits.h"
#include "bstone_exception.h"
#include "bstone_utility.h"

namespace bstone {

Exception::~Exception() = default;

// ==========================================================================

SourceException::~SourceException() = default;

// ==========================================================================

StaticSourceException::StaticSourceException(const SourceLocation& source_location, const char* message)
	:
	source_location_{source_location},
	message_{message != nullptr ? message : ""}
{
	BSTONE_ASSERT(message != nullptr);
}

StaticSourceException::StaticSourceException(const SourceLocation& source_location) noexcept
	:
	source_location_{source_location}
{}

StaticSourceException::StaticSourceException(const StaticSourceException& rhs) noexcept
	:
	source_location_{rhs.source_location_},
	message_{rhs.message_}
{}

StaticSourceException& StaticSourceException::operator=(const StaticSourceException& rhs) noexcept
{
	source_location_ = rhs.source_location_;
	message_ = rhs.message_;
	return *this;
}

StaticSourceException::~StaticSourceException() = default;

const SourceLocation& StaticSourceException::get_source_location() const noexcept
{
	return source_location_;
}

const char* StaticSourceException::what() const noexcept
{
	return message_;
}

void StaticSourceException::swap(StaticSourceException& rhs) noexcept
{
	source_location_.swap(rhs.source_location_);
	bstone::swop(message_, rhs.message_);
}

[[noreturn]] void StaticSourceException::fail(const SourceLocation& source_location, const char* message)
{
	throw StaticSourceException{source_location, message};
}

[[noreturn]] void StaticSourceException::fail_nested(const SourceLocation& source_location)
{
	std::throw_with_nested(StaticSourceException{source_location});
}

// ==========================================================================

DynamicSourceException::DynamicSourceException(const SourceLocation& source_location, const char* message)
	:
	source_location_{source_location}
{
	static_assert(std::is_trivial<ControlBlock>::value, "Expected a trivial class.");

	if (message == nullptr)
	{
		BSTONE_ASSERT(false && "Null message.");
		return;
	}

	const auto message_size_with_null = char_traits::get_size(message) + 1;
	const auto control_block_size = static_cast<std::intptr_t>(sizeof(ControlBlock));
	const auto storage_size = control_block_size + message_size_with_null;
	auto storage = std::make_unique<char[]>(storage_size);
	auto control_block = reinterpret_cast<ControlBlock*>(storage.get());
	control_block->counter = 1;
	control_block->message = reinterpret_cast<char*>(&control_block[1]);
	std::copy_n(message, message_size_with_null, control_block->message);
	control_block_ = reinterpret_cast<ControlBlock*>(storage.release());
}

DynamicSourceException::DynamicSourceException(const DynamicSourceException& rhs)
	:
	source_location_{rhs.source_location_},
	control_block_{rhs.control_block_}
{
	if (control_block_ == nullptr)
	{
		return;
	}

	auto& counter = control_block_->counter;
#if !defined(NDEBUG)
	constexpr auto max_counter_value = std::numeric_limits<Counter>::max();
	BSTONE_ASSERT(counter < max_counter_value);
#endif
	++counter;
}

DynamicSourceException& DynamicSourceException::operator=(const DynamicSourceException& rhs)
{
	auto copy = rhs;
	copy.swap(*this);
	return *this;
}

DynamicSourceException::~DynamicSourceException()
{
	if (control_block_ == nullptr)
	{
		return;
	}

	auto& counter = control_block_->counter;

	BSTONE_ASSERT(counter > 0);
	--counter;

	if (counter == 0)
	{
		delete[] reinterpret_cast<char*>(control_block_);
	}
}

const SourceLocation& DynamicSourceException::get_source_location() const noexcept
{
	return source_location_;
}

const char* DynamicSourceException::what() const noexcept
{
	return control_block_ != nullptr ? control_block_->message : "";
}

void DynamicSourceException::swap(DynamicSourceException& rhs) noexcept
{
	source_location_.swap(rhs.source_location_);
	bstone::swop(control_block_, rhs.control_block_);
}

[[noreturn]] void DynamicSourceException::fail(const SourceLocation& source_location, const char* message)
{
	throw DynamicSourceException{source_location, message};
}

} // namespace bstone
