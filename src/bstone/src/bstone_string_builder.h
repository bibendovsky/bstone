/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// String builder.

#ifndef BSTONE_STRING_BUILDER_INCLUDED
#define BSTONE_STRING_BUILDER_INCLUDED

#include "bstone_format.h"
#include <format>
#include <string>
#include <type_traits>
#include <utility>

namespace bstone {

class StringBuilder
{
public:
	const std::string& get_string() const;
	void reserve(int capacity);
	void increase_indent();
	void decrease_indent();
	void reset_indent();

	template<typename T>
	void add(T&& value)
	{
		message_ += std::forward<T>(value);
	}

	void add_line();

	template<typename T>
	void add_line(T&& value)
	{
		add(std::forward<T>(value));
		add_line();
	}

	void add_indent();

	template<typename T>
	void add_indented(T&& value)
	{
		add_indent();
		add(std::forward<T>(value));
	}

	template<typename T>
	void add_indented_line(T&& value)
	{
		add_indent();
		add(std::forward<T>(value));
		add_line();
	}

	template<typename... TArgs>
	void add(std::format_string<TArgs...> format_string, TArgs&&... args)
	requires (sizeof...(args) > 0)
	{
		add_internal(format_string.get(), std::make_format_args(args...));
	}

	template<typename... TArgs>
	void add_line(std::format_string<TArgs...> format_string, TArgs&&... args)
	requires (sizeof...(args) > 0)
	{
		add_internal(format_string.get(), std::make_format_args(args...));
		add_line();
	}

	template<typename... TArgs>
	void add_indented(std::format_string<TArgs...> format_string, TArgs&&... args)
	requires (sizeof...(args) > 0)
	{
		add_indent();
		add_internal(format_string.get(), std::make_format_args(args...));
	}
	
	template<typename... TArgs>
	void add_indented_line(std::format_string<TArgs...> format_string, TArgs&&... args)
	requires (sizeof...(args) > 0)
	{
		add_indent();
		add_internal(format_string.get(), std::make_format_args(args...));
		add_line();
	}

private:
	constinit inline static int indent_step = 2;

	std::string message_{};
	std::string indent_{};
	StdStringFormatIterator iterator_{message_};

	void add_internal(std::string_view string_view, std::format_args format_args)
	{
		std::vformat_to(iterator_, string_view, format_args);
	}
};

} // namespace bstone

#endif // BSTONE_STRING_BUILDER_INCLUDED
