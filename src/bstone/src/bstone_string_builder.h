/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// String builder.

#ifndef BSTONE_STRING_BUILDER_INCLUDED
#define BSTONE_STRING_BUILDER_INCLUDED

#include <format>
#include <iterator>
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
	void add(const std::format_string<TArgs...> format_string, TArgs&&... args)
		requires (sizeof...(args) > 0)
	{
		std::format_to(std::back_inserter(message_), format_string, std::forward<TArgs>(args)...);
	}

	template<typename... TArgs>
	void add_line(const std::format_string<TArgs...> format_string, TArgs&&... args)
		requires (sizeof...(args) > 0)
	{
		add(format_string, std::forward<TArgs>(args)...);
		add_line();
	}

	template<typename... TArgs>
	void add_indented(const std::format_string<TArgs...> format_string, TArgs&&... args)
		requires (sizeof...(args) > 0)
	{
		add_indent();
		add(format_string, std::forward<TArgs>(args)...);
	}
	
	template<typename... TArgs>
	void add_indented_line(const std::format_string<TArgs...> format_string, TArgs&&... args)
		requires (sizeof...(args) > 0)
	{
		add_indent();
		add(format_string, std::forward<TArgs>(args)...);
		add_line();
	}

private:
	constinit inline static int indent_step = 2;

	std::string message_{};
	std::string indent_{};
};

} // namespace bstone

#endif // BSTONE_STRING_BUILDER_INCLUDED
