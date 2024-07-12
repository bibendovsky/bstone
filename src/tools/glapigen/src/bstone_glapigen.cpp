/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL API generator.

#ifndef CMAKE_PROJECT_NAME
#error CMAKE_PROJECT_NAME not defined.
#endif

#ifndef CMAKE_PROJECT_VERSION
#error CMAKE_PROJECT_VERSION not defined.
#endif

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

using Strings = std::vector<std::string>;

// ===========================================================================

class SourceLocation
{
public:
	SourceLocation(const char* function_name, int line_number) noexcept;

	const char* get_function_name() const noexcept;
	int get_line_number() const noexcept;

private:
	const char* function_name_{};
	int line_number_{};
};

// ---------------------------------------------------------------------------

SourceLocation::SourceLocation(const char* function_name, int line_number) noexcept
	:
	function_name_{function_name},
	line_number_{line_number}
{}

const char* SourceLocation::get_function_name() const noexcept
{
	return function_name_;
}

int SourceLocation::get_line_number() const noexcept
{
	return line_number_;
}

// ===========================================================================

#ifndef MAKE_SOURCE_LOCATION
#define MAKE_SOURCE_LOCATION() SourceLocation{__func__, __LINE__}
#endif

// ===========================================================================

class Exception : public std::runtime_error
{
public:
	Exception(const SourceLocation& source_location, const std::string& message);
	~Exception() override = default;

	template<typename... TArgs>
	[[noreturn]] static void fail(
		const SourceLocation& source_location,
		const std::string& format,
		TArgs&&... args);

private:
	static std::string make_error_message(
		const SourceLocation& source_location,
		const std::string& message);

	static void fail_internal(
		std::ostream&,
		const std::string&,
		std::size_t);

	template<typename TArg, typename... TArgs>
	static void fail_internal(
		std::ostream& stream,
		const std::string& format,
		std::size_t& format_index,
		TArg&& arg,
		TArgs&&... args);
};

// ---------------------------------------------------------------------------

Exception::Exception(const SourceLocation& source_location, const std::string& message)
	:
	std::runtime_error{make_error_message(source_location, message)}
{}

// ===========================================================================

std::string Exception::make_error_message(
	const SourceLocation& source_location,
	const std::string& message)
{
	const auto function_name_size = std::string::traits_type::length(
		source_location.get_function_name());

	auto result = std::string{};
	result.reserve(function_name_size + message.size() + 16);
	result += '[';
	result += source_location.get_function_name();
	result += ':';
	result += std::to_string(source_location.get_line_number());
	result += "] ";
	result += message;

	return result;
}

void Exception::fail_internal(std::ostream&, const std::string&, std::size_t) {}

template<typename TArg, typename... TArgs>
void Exception::fail_internal(
	std::ostream& stream,
	const std::string& format,
	std::size_t& format_index,
	TArg&& arg,
	TArgs&&... args)
{
	const auto format_size = format.size();

	while (true)
	{
		if (format_index == format_size)
		{
			return;
		}

		const auto ch_0 = format[format_index];
		const auto ch_1 = format_index + 1 < format_size ? format[format_index + 1] : -1;

		if (false) {}
		else if (ch_0 == '{' && ch_1 == '{')
		{
			stream << '{';
			format_index += 2;
		}
		else if (ch_0 == '}' && ch_1 == '}')
		{
			stream << '}';
			format_index += 2;
		}
		else if (ch_0 == '{' && ch_1 == '}')
		{
			stream << arg;
			format_index += 2;
			break;
		}
		else
		{
			stream << ch_0;
			++format_index;
		}
	}

	fail_internal(stream, format, format_index, std::forward<TArgs>(args)...);
}

template<typename... TArgs>
[[noreturn]] void Exception::fail(
	const SourceLocation& source_location,
	const std::string& format,
	TArgs&&... args)
{
	auto stream = std::ostringstream{};
	auto format_index = std::size_t{};
	fail_internal(stream, format, format_index, std::forward<TArgs>(args)...);
	stream.write(format.c_str() + format_index, format.size() - format_index);
	throw Exception{source_location, stream.str()};
}

// ===========================================================================

#ifndef BSTONE_GLAPIGEN_FAIL
#define BSTONE_GLAPIGEN_FAIL(...) Exception::fail(MAKE_SOURCE_LOCATION(), __VA_ARGS__)
#endif

// ===========================================================================

bool ascii_is_digit(char ch) noexcept
{
	return ch >= '0' && ch <= '9';
}

std::string ascii_to_upper(const std::string& string)
{
	auto result = string;

	for (auto& ch : result)
	{
		if (ch < 'a' || ch > 'z')
		{
			continue;
		}

		ch = static_cast<char>(ch + 'A' - 'a');
	}

	return result;
}

// ===========================================================================

bool starts_with(const std::string& string, const std::string& substring) noexcept
{
	if (string.size() < substring.size())
	{
		return false;
	}

	return std::equal(substring.cbegin(), substring.cend(), string.cbegin());
}

bool ends_with(const std::string& string, char ch) noexcept
{
	return !string.empty() && string.back() == ch;
}

// ===========================================================================

Strings split(const std::string& string, char separator)
{
	if (string.empty())
	{
		return Strings{};
	}

	const auto word_count = std::count(string.cbegin(), string.cend(), separator) + 1;

	auto strings = Strings{};
	strings.reserve(word_count);

	auto position = std::string::size_type{};

	while (true)
	{
		const auto new_position = string.find(separator, position);
		strings.emplace_back(string.substr(position, new_position - position));

		if (new_position == std::string::npos)
		{
			break;
		}

		position = new_position + 1;
	}

	return strings;
}

// ===========================================================================

template<
	typename TKey,
	typename THash,
	typename TKeyEqual,
	typename TAllocator>
bool contains_key(const std::unordered_map<TKey, THash, TKeyEqual, TAllocator>& map, const TKey& key) noexcept
{
	return map.find(key) != map.cend();
}

// ===========================================================================

template<
	typename T,
	typename TAllocator>
bool contains(const std::vector<T, TAllocator>& vector, const T& value) noexcept
{
	const auto iter_end = vector.cend();

	return std::find(vector.cbegin(), iter_end, value) != iter_end;
}

// ===========================================================================

struct XmlAttribute
{
	std::string name;
	std::string value;
};

using XmlAttributes = std::vector<XmlAttribute>;

// ===========================================================================

struct XmlNode
{
	std::string name;
	std::string value;
	XmlAttributes attributes;

	XmlNode* sibling{};
	XmlNode* child{};
	XmlNode* parent{};

	bool is_text() const noexcept;
};

// ---------------------------------------------------------------------------

bool XmlNode::is_text() const noexcept
{
	return name.empty();
}

// ===========================================================================

class XmlNodes
{
public:
	XmlNodes() = default;
	explicit XmlNodes(std::size_t capacity);
	XmlNodes(const XmlNodes&) = delete;
	XmlNodes& operator=(const XmlNodes&) = delete;
	XmlNodes(XmlNodes&& rhs) noexcept = default;
	XmlNodes& operator=(XmlNodes&& rhs) noexcept = default;

	const XmlNode& get_first() const;

	XmlNode& allocate();
	void swap(XmlNodes& rhs) noexcept;

private:
	using Nodes = std::vector<XmlNode>;

private:
	Nodes nodes_{};
};

// ---------------------------------------------------------------------------

XmlNodes::XmlNodes(std::size_t capacity)
{
	nodes_.reserve(capacity);
}

const XmlNode& XmlNodes::get_first() const
{
	if (nodes_.empty())
	{
		BSTONE_GLAPIGEN_FAIL("No nodes.");
	}

	return nodes_.front();
}

XmlNode& XmlNodes::allocate()
{
	if (nodes_.size() == nodes_.capacity())
	{
		BSTONE_GLAPIGEN_FAIL("Out of nodes.");
	}

	nodes_.emplace_back();
	return nodes_.back();
}

void XmlNodes::swap(XmlNodes& rhs) noexcept
{
	nodes_.swap(rhs.nodes_);
}

// ===========================================================================

class XmlParser
{
public:
	XmlParser(const char* chars, std::size_t count);
	explicit XmlParser(const std::string& string);

	void swap_nodes(XmlNodes& rhs) noexcept;

private:
	static constexpr std::size_t max_depth = 8;

private:
	using OpenNodes = std::vector<XmlNode*>;

private:
	static std::string empty_string;

private:
	std::intptr_t line_{1};
	std::intptr_t column_{1};
	const char* chars_{};
	std::size_t char_count_{};
	std::size_t char_index_{};
	XmlNodes nodes_{};
	OpenNodes open_nodes_{};
	XmlNode* current_node_{};
	bool has_text_declaration_{};

private:
	template<typename... TArgs>
	[[noreturn]] void fail_at(
		const SourceLocation& source_location,
		const std::string& format,
		TArgs&&... args) const;

	void add_node(const std::string* name, const std::string* value, const XmlAttributes* attributes);
	void add_text_element(const std::string& value);
	void add_empty_element(const std::string& name, const XmlAttributes& attributes);
	void open_element(const std::string& name, const XmlAttributes& attributes);
	void close_element(const std::string& name);
	int peek_char(std::size_t offset) const;
	int peek_char() const;
	void move_cursor_to_next_line();
	static bool is_whitespace(int ch);
	static bool is_text_char(int ch);
	static bool is_name_start_char(int ch);
	static bool is_name_char(int ch);
	static bool is_attribute_value_char(int ch);
	bool is_eol(std::intptr_t& eol_length) const;
	bool is_eof() const;
	bool is_comment_begin() const;
	bool is_comment_end() const;
	bool is_text_declaration_end() const;
	void advance_char(std::size_t count);
	void advance_char();
	std::intptr_t skip_whitespaces();
	void skip_comment();
	void parse_text();
	std::string parse_attribute_name();
	std::string parse_attribute_value();
	XmlAttribute parse_attribute();
	void parse_text_declaration();
	std::string parse_name();
	void parse_start_tag();
	void parse_end_tag();
	void parse_markup();
};

// ---------------------------------------------------------------------------

#define XML_FAIL_AT(...) fail_at(MAKE_SOURCE_LOCATION(), __VA_ARGS__)

std::string XmlParser::empty_string{};

XmlParser::XmlParser(const char* chars, std::size_t count)
	:
	chars_{chars},
	char_count_{count}
{
	assert(chars != nullptr);

	const auto max_nodes = static_cast<std::size_t>(std::count(chars, chars + count, '<')) * 2;
	nodes_ = XmlNodes{max_nodes};

	open_nodes_.reserve(max_depth);

	while (true)
	{
		const auto ch = peek_char();

		if (ch < 0)
		{
			break;
		}
		else if (ch == '<')
		{
			if (is_comment_begin())
			{
				parse_text();
			}
			else
			{
				parse_markup();
			}
		}
		else
		{
			parse_text();
		}
	}

	if (!open_nodes_.empty())
	{
		XML_FAIL_AT("Missing end-tag. ({})", open_nodes_.back()->name);
	}
}

XmlParser::XmlParser(const std::string& string)
	:
	XmlParser{string.c_str(), string.size()}
{}

void XmlParser::swap_nodes(XmlNodes& rhs) noexcept
{
	nodes_.swap(rhs);
}

template<typename... TArgs>
[[noreturn]] void XmlParser::fail_at(
	const SourceLocation& source_location,
	const std::string& format,
	TArgs&&... args) const
{
	const auto new_format = "[XML][{}:{}] " + format;
	Exception::fail(source_location, new_format, line_, column_, std::forward<TArgs>(args)...);
}

void XmlParser::add_node(const std::string* name, const std::string* value, const XmlAttributes* attributes)
{
	assert((name == nullptr && value != nullptr) || (name != nullptr && value == nullptr));

	auto& node = nodes_.allocate();

	if (name != nullptr)
	{
		node.name = *name;
	}

	if (value != nullptr)
	{
		node.value = *value;
	}

	if (attributes != nullptr)
	{
		node.attributes = *attributes;
	}

	node.parent = current_node_;

	if (node.parent != nullptr)
	{
		if (!open_nodes_.empty() && open_nodes_.back() == current_node_)
		{
			assert(node.parent->sibling == nullptr);

			node.parent->child = &node;
		}
		else
		{
			node.parent->sibling = &node;
		}
	}

	current_node_ = &node;
}

void XmlParser::add_text_element(const std::string& value)
{
	add_node(nullptr, &value, nullptr);
}

void XmlParser::add_empty_element(const std::string& name, const XmlAttributes& attributes)
{
	add_node(&name, nullptr, attributes.empty() ? nullptr : &attributes);
}

void XmlParser::open_element(const std::string& name, const XmlAttributes& attributes)
{
	add_node(&name, nullptr, attributes.empty() ? nullptr : &attributes);
	open_nodes_.emplace_back(current_node_);
}

void XmlParser::close_element(const std::string& name)
{
	if (open_nodes_.empty() || open_nodes_.back()->name != name)
	{
		XML_FAIL_AT("No matching start-tag.");
	}

	current_node_ = open_nodes_.back();
	open_nodes_.pop_back();
}

int XmlParser::peek_char(std::size_t offset) const
{
	assert(offset <= 3);

	const auto new_index = char_index_ + offset;

	if (new_index >= char_count_)
	{
		return -1;
	}

	return chars_[new_index];
}

int XmlParser::peek_char() const
{
	return peek_char(0);
}

void XmlParser::move_cursor_to_next_line()
{
	++line_;
	column_ = 1;
}

bool XmlParser::is_whitespace(int ch)
{
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

bool XmlParser::is_text_char(int ch)
{
	return (ch >= '\x21' && ch <= '\x7E') || is_whitespace(ch);
}

bool XmlParser::is_name_start_char(int ch)
{
	return ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool XmlParser::is_name_char(int ch)
{
	return
		ch == '_' ||
		(ch >= '0' && ch <= '9') ||
		(ch >= 'a' && ch <= 'z') ||
		(ch >= 'A' && ch <= 'Z');
}

bool XmlParser::is_attribute_value_char(int ch)
{
	return ch >= '\x20' && ch <= '\x7E';
}

bool XmlParser::is_eol(std::intptr_t& eol_length) const
{
	switch (peek_char())
	{
		case '\r':
			eol_length = 1 + peek_char(1) == '\n';
			return true;

		case '\n':
			eol_length = 1;
			return true;

		default:
			eol_length = 0;
			return false;
	}
}

bool XmlParser::is_eof() const
{
	return peek_char() < 0;
}

bool XmlParser::is_comment_begin() const
{
	return peek_char() == '<' && peek_char(1) == '!' && peek_char(2) == '-' && peek_char(3) == '-';
}

bool XmlParser::is_comment_end() const
{
	return peek_char() == '-' && peek_char(1) == '-' && peek_char(2) == '>';
}

bool XmlParser::is_text_declaration_end() const
{
	return peek_char() == '?' && peek_char(1) == '>';
}

void XmlParser::advance_char(std::size_t count)
{
	assert(count <= 4);

	char_index_ += count;
	column_ += count;
}

void XmlParser::advance_char()
{
	advance_char(1);
}

std::intptr_t XmlParser::skip_whitespaces()
{
	const auto char_index_begin = char_index_;

	while (true)
	{
		const auto ch = peek_char();

		switch (ch)
		{
			case ' ':
			case '\t':
				advance_char(1);
				break;

			case '\r':
				advance_char(1 + peek_char(1) == '\n');
				move_cursor_to_next_line();
				break;

			case '\n':
				advance_char(1);
				move_cursor_to_next_line();
				break;

			default: return char_index_ - char_index_begin;
		}
	}
}

void XmlParser::skip_comment()
{
	advance_char(4);

	while (true)
	{
		if (is_comment_end())
		{
			break;
		}

		if (peek_char() == '-' && peek_char(1) == '-')
		{
			XML_FAIL_AT("Double dash in comment.");
		}

		if (is_eof())
		{
			XML_FAIL_AT("Unclosed comment.");
		}

		auto eol_size = std::intptr_t{};

		if (is_eol(eol_size))
		{
			advance_char(eol_size);
			move_cursor_to_next_line();
		}
		else
		{
			advance_char();
		}
	}

	advance_char(3);
}

void XmlParser::parse_text()
{
	auto value = std::string{};

	while (true)
	{
		if (is_comment_begin())
		{
			skip_comment();
		}

		const auto ch = peek_char();
		auto eol_size = std::intptr_t{};

		if (false) {}
		else if (ch < 0 || ch == '<')
		{
			break;
		}
		else if (is_eol(eol_size))
		{
			value += '\n';
			advance_char(eol_size);
			move_cursor_to_next_line();
		}
		else if (ch == '&')
		{
			if (false) {}
			else if (peek_char(1) == 'l' && peek_char(2) == 't' && peek_char(3) == ';')
			{
				value += '<';
			}
			else if (peek_char(1) == 'g' && peek_char(2) == 't' && peek_char(3) == ';')
			{
				value += '>';
			}
			else
			{
				XML_FAIL_AT("Unknown escape sequence.");
			}

			advance_char(4);
		}
		else if (is_text_char(ch))
		{
			value += static_cast<char>(ch);
			advance_char();
		}
		else
		{
			XML_FAIL_AT("Unsupported text character.");
		}
	}

	if (value.empty())
	{
		return;
	}

	add_text_element(value);
}

std::string XmlParser::parse_attribute_name()
{
	auto name = std::string{};
	const auto first_ch = peek_char();

	if (!is_name_start_char(first_ch))
	{
		XML_FAIL_AT("Invalid start character of the name.");
	}

	name += static_cast<char>(first_ch);
	advance_char();

	while (true)
	{
		const auto ch = peek_char();

		if (ch == '=' || is_whitespace(ch))
		{
			break;
		}

		if (!is_name_char(ch))
		{
			XML_FAIL_AT("Unsupported attribute name char.");
		}

		name += static_cast<char>(ch);
		advance_char();
	}

	return name;
}

std::string XmlParser::parse_attribute_value()
{
	auto value = std::string{};

	const auto quotation_mark = peek_char();

	switch (quotation_mark)
	{
		case '\'':
		case '"':
			advance_char();
			break;

		default: XML_FAIL_AT("Expected an opening quotation mark.");
	}

	skip_whitespaces();

	while (true)
	{
		const auto ch = peek_char();
		auto eol_size = std::intptr_t{};

		if (false) {}
		else if (ch == quotation_mark)
		{
			break;
		}
		else if (is_eol(eol_size) || is_whitespace(ch))
		{
			if (value.empty() || (!value.empty() && value.back() != ' '))
			{
				value += ' ';
			}

			advance_char(std::max(eol_size, std::intptr_t{1}));
		}
		else
		{
			if (!is_attribute_value_char(ch))
			{
				XML_FAIL_AT("Unsupported attribute value char.");
			}

			value += static_cast<char>(ch);
			advance_char();
		}
	}

	if (!value.empty() && value.back() == ' ')
	{
		value.pop_back();
	}

	advance_char();

	return value;
}

XmlAttribute XmlParser::parse_attribute()
{
	const auto name = parse_attribute_name();
	skip_whitespaces();

	if (peek_char() != '=')
	{
		XML_FAIL_AT("Expected an equality sign.");
	}

	advance_char();

	skip_whitespaces();
	const auto value = parse_attribute_value();

	auto xml_attribute = XmlAttribute{};
	xml_attribute.name = name;
	xml_attribute.value = value;

	return xml_attribute;
}

void XmlParser::parse_text_declaration()
{
	if (has_text_declaration_)
	{
		XML_FAIL_AT("Text declaration already defined.");
	}

	if (char_index_ != 0)
	{
		XML_FAIL_AT("Text declaration not at the beginning.");
	}

	advance_char(2);

	if (peek_char() != 'x' || peek_char(1) != 'm' || peek_char(2) != 'l')
	{
		XML_FAIL_AT("Invalid text declaration.");
	}

	advance_char(3);

	auto was_version = false;
	auto was_encoding = false;

	while (true)
	{
		const auto whitespace_count = skip_whitespaces();

		if (is_text_declaration_end())
		{
			break;
		}

		if (whitespace_count == 0)
		{
			XML_FAIL_AT("Expected at least one whitespace.");
		}

		const auto attribute = parse_attribute();

		if (false) {}
		else if (attribute.name == "version")
		{
			if (attribute.value != "1.0")
			{
				XML_FAIL_AT("Unsupported version.");
			}

			was_version = true;
		}
		else if (attribute.name == "encoding")
		{
			if (attribute.value != "UTF-8")
			{
				XML_FAIL_AT("Unsupported encoding.");
			}

			was_encoding = true;
		}
		else
		{
			XML_FAIL_AT("Unknown attribute.");
		}
	}

	if (!was_version)
	{
		XML_FAIL_AT("No version.");
	}

	if (!was_encoding)
	{
		XML_FAIL_AT("No encoding.");
	}

	advance_char(2);

	has_text_declaration_ = true;
}

std::string XmlParser::parse_name()
{
	advance_char();
	const auto first_ch = peek_char();

	if (!is_name_start_char(first_ch))
	{
		XML_FAIL_AT("Unsupported first character of name.");
	}

	auto name = std::string{};
	name += static_cast<char>(first_ch);
	advance_char();

	while (true)
	{
		const auto ch = peek_char();

		if (is_whitespace(ch) || ch == '/' || ch == '>')
		{
			break;
		}

		if (!is_name_char(ch))
		{
			XML_FAIL_AT("Unsupported character of name.");
		}

		name += static_cast<char>(ch);
		advance_char();
	}

	return name;
}

void XmlParser::parse_start_tag()
{
	const auto name = parse_name();
	auto attributes = XmlAttributes{};

	while (true)
	{
		skip_whitespaces();
		const auto ch = peek_char();

		if (ch == '/' || ch == '>')
		{
			break;
		}

		const auto attribute = parse_attribute();
		attributes.emplace_back(attribute);
	}

	auto last_ch = peek_char();
	auto is_empty_element = false;

	if (last_ch == '/')
	{
		is_empty_element = true;
		advance_char();
		last_ch = peek_char();
	}

	if (last_ch != '>')
	{
		XML_FAIL_AT("Unexpected end of tag.");
	}

	advance_char();

	std::sort(
		attributes.begin(),
		attributes.end(),
		[](const XmlAttribute& lhs, const XmlAttribute& rhs)
		{
			return lhs.name < rhs.name;
		});

	if (is_empty_element)
	{
		add_empty_element(name, attributes);
	}
	else
	{
		open_element(name, attributes);
	}
}

void XmlParser::parse_end_tag()
{
	advance_char(2);
	const auto first_ch = peek_char();

	if (!is_name_start_char(first_ch))
	{
		XML_FAIL_AT("Unsupported first char of name.");
	}

	auto name = std::string{};
	name += static_cast<char>(first_ch);
	advance_char();

	while (true)
	{
		const auto ch = peek_char();

		if (is_whitespace(ch) || ch == '>')
		{
			break;
		}

		if (!is_name_char(ch))
		{
			XML_FAIL_AT("Invalid name char.");
		}

		name += static_cast<char>(ch);
		advance_char();
	}

	skip_whitespaces();
	const auto last_ch = peek_char();

	if (last_ch != '>')
	{
		XML_FAIL_AT("Expected closing tag.");
	}

	advance_char();
	close_element(name);
}

void XmlParser::parse_markup()
{
	switch (peek_char(1))
	{
		case '?': parse_text_declaration(); break;
		case '/': parse_end_tag(); break;
		default: parse_start_tag(); break;
	}
}

#undef XML_FAIL_AT

// ===========================================================================

enum class RegistryApi
{
	none,
	gl,
	glcore,
	gles1,
	gles2,
	glsc2,
};

enum class RegistryProfile
{
	none,
	undefined,
	compatibility,
	core,
	common,
};

enum class RegistryCTypeSuffix
{
	none,
	u,
	ull,
};

// ===========================================================================

struct RegistryType
{
	bool is_header;
	std::string name;
	std::string value;
	std::size_t apientry_offset; // Zero value for none.

	bool has_apientry() const noexcept;
};

// ---------------------------------------------------------------------------

bool RegistryType::has_apientry() const noexcept
{
	return apientry_offset != 0;
}

// ===========================================================================

using RegistryTypeMap = std::unordered_map<std::string, RegistryType>;
using RegistryTypeNames = std::vector<std::string>;

// ===========================================================================

struct RegistryEnum
{
	bool is_bitmask;
	RegistryApi api;
	std::string name;
	std::string alias;
	std::string value;
	RegistryCTypeSuffix c_type_suffix;
};

using RegistryApiToEnumMap = std::unordered_map<RegistryApi, RegistryEnum>;
using RegistryEnumMap = std::unordered_map<std::string, RegistryApiToEnumMap>;
using RegistryEnumNames = std::vector<std::string>;

// ===========================================================================

struct RegistryCommandParam
{
	std::string name;
	std::string type;
};

using RegistryCommandParams = std::vector<RegistryCommandParam>;

// ===========================================================================

struct RegistryCommand
{
	RegistryCommandParam proto;
	RegistryCommandParams params;
};

// ===========================================================================

using RegistryCommandMap = std::unordered_map<std::string, RegistryCommand>;
using RegistryCommandNames = std::vector<std::string>;

// ===========================================================================

using RegistryFeatureCommands = std::vector<std::string>;

struct RegistryFeature
{
	RegistryApi api;
	std::string name;
	std::string number;
	RegistryFeatureCommands commands;
};

using RegistryFeatureMap = std::unordered_map<std::string, RegistryFeature>;
using RegistryFeatureNames = std::vector<std::string>;
using RegistryFeatureRemovedNames = std::unordered_set<std::string>;

// ===========================================================================

using RegistryExtensionApiSet = std::unordered_set<RegistryApi>;
using RegistryExtensionCommands = std::vector<std::string>;

struct RegistryExtension
{
	std::string name;
	RegistryExtensionApiSet api_set;
	RegistryExtensionCommands commands;
};

using RegistryExtensionMap = std::unordered_map<std::string, RegistryExtension>;
using RegistryExtensionNames = std::vector<std::string>;

// ===========================================================================

struct Registry
{
	RegistryTypeMap type_map;
	RegistryTypeNames type_names;
	RegistryEnumMap enum_map;
	RegistryEnumNames enum_names;
	RegistryCommandMap command_map;
	RegistryCommandNames command_names;
	RegistryFeatureMap feature_map;
	RegistryFeatureNames feature_names;
	RegistryFeatureRemovedNames feature_removed_names;
	RegistryExtensionMap extension_map;
	RegistryExtensionNames extension_names;

	void swap(Registry& rhs) noexcept;
};

// ---------------------------------------------------------------------------

void Registry::swap(Registry& rhs) noexcept
{
	type_map.swap(rhs.type_map);
	type_names.swap(rhs.type_names);
	enum_map.swap(rhs.enum_map);
	enum_names.swap(rhs.enum_names);
	command_map.swap(rhs.command_map);
	command_names.swap(rhs.command_names);
	feature_map.swap(rhs.feature_map);
	feature_names.swap(rhs.feature_names);
	feature_removed_names.swap(rhs.feature_removed_names);
	extension_map.swap(rhs.extension_map);
	extension_names.swap(rhs.extension_names);
}

// ===========================================================================

const char* to_string(RegistryApi api)
{
	switch (api)
	{
		case RegistryApi::gl: return "gl";
		case RegistryApi::glcore: return "glcore";
		case RegistryApi::gles1: return "gles1";
		case RegistryApi::gles2: return "gles2";
		case RegistryApi::glsc2: return "glsc2";
		default: BSTONE_GLAPIGEN_FAIL("Unknown API. ({})", static_cast<int>(api));
	}
}

// ===========================================================================

class RegistryParser
{
public:
	explicit RegistryParser(const std::string& xml_path);

	Registry& get_registry() noexcept;

private:
	static constexpr auto max_file_size = 4'000'000;

private:
	static const std::string glapientry_string;

	XmlNodes xml_nodes_{};
	Registry registry_{};

private:
	static std::string load_xml(const std::string& file_path);
	static RegistryApi map_api(const std::string& api_string);
	static RegistryExtensionApiSet map_extension_api_set(const std::string& api_set_string);
	static RegistryProfile map_profile(const std::string& api_string);
	static RegistryCTypeSuffix map_c_type_suffix(const std::string& value_string);
	static void extract_full_value(const XmlNode& first_xml_node, RegistryType& registry_type);
	const XmlNode& xml_find_required_node(const XmlNode& start_node, const std::string& name) const;
	void register_type(const RegistryType& type);
	void register_enum(const RegistryEnum& registry_enum);
	void register_command(const RegistryCommand& command);
	void register_feature(const RegistryFeature& feature);
	void register_extension(const RegistryExtension& extention);
	void parse_type_include(const std::string& name, const std::string& value);
	void parse_type_alias(const std::string& name, const XmlNode& first_xml_node);
	void parse_type_struct(const std::string& name, const XmlNode& first_xml_node);
	std::string parse_type_name(const XmlNode& type_xml_node) const;
	void parse_type(const XmlNode& first_xml_node);
	void parse_types(const XmlNode& first_xml_node);
	void parse_enum(const XmlNode& first_xml_node, bool is_bitmask);
	void parse_enums(const XmlNode& first_xml_node);
	RegistryCommandParam parse_command_proto_or_param(const XmlNode& first_xml_node) const;
	RegistryCommandParam parse_command_proto(const XmlNode& first_xml_node);
	RegistryCommandParam parse_command_param(const XmlNode& first_xml_node);
	static void set_command_proto(const RegistryCommandParam& proto, RegistryCommand& command);
	static void add_command_param(const RegistryCommandParam& param, RegistryCommand& command);
	void parse_command(const XmlNode& first_xml_node);
	void parse_commands(const XmlNode& first_xml_node);
	void parse_feature_require_command(const XmlNode& first_xml_node, RegistryFeature& feature);
	void parse_feature_requires(const XmlNode& first_xml_node, RegistryFeature& feature);
	void parse_feature_remove_command(const XmlNode& first_xml_node);
	void parse_feature_removes(const XmlNode& first_xml_node);
	static void validate_feature_number(const std::string& number);
	void parse_feature(const XmlNode& first_xml_node);
	void parse_features(const XmlNode& first_xml_node);
	void parse_extension_require_command(const XmlNode& first_xml_node, RegistryExtension& extension);
	void parse_extension_require(const XmlNode& first_xml_node, RegistryExtension& extension);
	void parse_extension(const XmlNode& first_xml_node);
	void parse_extensions(const XmlNode& first_xml_node);
	void parse();
};

// ---------------------------------------------------------------------------

RegistryParser::RegistryParser(const std::string& xml_path)
{
	{
		const auto xml_string = load_xml(xml_path);
		auto xml_parser = XmlParser{xml_string};
		xml_parser.swap_nodes(xml_nodes_);
	}

	constexpr auto types_capacity = 512;
	registry_.type_map.reserve(types_capacity);
	registry_.type_names.reserve(types_capacity);

	constexpr auto enums_capacity = 16384;
	registry_.enum_map.reserve(enums_capacity);
	registry_.enum_names.reserve(enums_capacity);

	constexpr auto commands_capacity = 8192;
	registry_.command_map.reserve(commands_capacity);
	registry_.command_names.reserve(commands_capacity);

	constexpr auto features_capacity = 32;
	registry_.feature_map.reserve(features_capacity);
	registry_.feature_names.reserve(features_capacity);

	constexpr auto extensions_capacity = 1024;
	registry_.extension_map.reserve(extensions_capacity);
	registry_.extension_names.reserve(extensions_capacity);

	parse();
}

Registry& RegistryParser::get_registry() noexcept
{
	return registry_;
}

const std::string RegistryParser::glapientry_string = "GLAPIENTRY";

std::string RegistryParser::load_xml(const std::string& file_path)
{
	auto file = std::filebuf{};
	file.open(file_path.c_str(), std::ios_base::in | std::ios_base::binary);

	if (!file.is_open())
	{
		BSTONE_GLAPIGEN_FAIL("Failed to open a file.");
	}

	const auto std_file_size = file.pubseekoff(0, std::ios_base::end);

	if (std_file_size < 0 || std_file_size > max_file_size)
	{
		BSTONE_GLAPIGEN_FAIL("File is too big.");
	}

	file.pubseekpos(0);

	auto xml_string = std::string{};
	xml_string.resize(std_file_size);

	if (file.sgetn(&xml_string.front(), std_file_size) != std_file_size)
	{
		BSTONE_GLAPIGEN_FAIL("Failed to read a whole file.");
	}

	return xml_string;
}

RegistryApi RegistryParser::map_api(const std::string& api_string)
{
	if (false) {}
	else if (api_string == "gl")
	{
		return RegistryApi::gl;
	}
	else if (api_string == "glcore")
	{
		return RegistryApi::glcore;
	}
	else if (api_string == "gles1")
	{
		return RegistryApi::gles1;
	}
	else if (api_string == "gles2")
	{
		return RegistryApi::gles2;
	}
	else if (api_string == "glsc2")
	{
		return RegistryApi::glsc2;
	}
	else
	{
		BSTONE_GLAPIGEN_FAIL("Unknown API. ({})", api_string);
	}
}

RegistryExtensionApiSet RegistryParser::map_extension_api_set(const std::string& api_set_string)
{
	if (api_set_string.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Empty API set.");
	}

	auto result = RegistryExtensionApiSet{};
	const auto& api_strings = split(api_set_string, '|');

	for (const auto& api_string : api_strings)
	{
		const auto api = map_api(api_string);
		result.emplace(api);
	}

	return result;
}

RegistryProfile RegistryParser::map_profile(const std::string& profile_string)
{
	if (false) {}
	else if (profile_string == "undefined")
	{
		return RegistryProfile::undefined;
	}
	else if (profile_string == "compatibility")
	{
		return RegistryProfile::compatibility;
	}
	else if (profile_string == "core")
	{
		return RegistryProfile::core;
	}
	else if (profile_string == "common")
	{
		return RegistryProfile::common;
	}
	else
	{
		BSTONE_GLAPIGEN_FAIL("Unknown profile. ({})", profile_string);
	}
}

RegistryCTypeSuffix RegistryParser::map_c_type_suffix(const std::string& value_string)
{
	if (false) {}
	else if (value_string == "u")
	{
		return RegistryCTypeSuffix::u;
	}
	else if (value_string == "ull")
	{
		return RegistryCTypeSuffix::ull;
	}
	else
	{
		BSTONE_GLAPIGEN_FAIL("Unknown C-type suffix. ({})", value_string);
	}
}

void RegistryParser::extract_full_value(const XmlNode& first_xml_node, RegistryType& registry_type)
{
	auto& value = registry_type.value;
	value.reserve(256);

	auto& apientry_offset = registry_type.apientry_offset;
	apientry_offset = 0;

	for (auto xml_node = &first_xml_node; xml_node != nullptr; xml_node = xml_node->sibling)
	{
		if (xml_node->is_text())
		{
			value += xml_node->value;
		}
		else
		{
			if (xml_node->name == "apientry")
			{
				if (xml_node->child != nullptr)
				{
					BSTONE_GLAPIGEN_FAIL("Unexpected child XML node.");
				}

				if (value.empty())
				{
					BSTONE_GLAPIGEN_FAIL("Unexpected API entry position.");
				}

				if (apientry_offset > 0)
				{
					BSTONE_GLAPIGEN_FAIL("API entry already defined.");
				}

				apientry_offset = value.size();
			}
			else if (xml_node->child != nullptr)
			{
				value += xml_node->child->value;
			}
		}
	}
}

const XmlNode& RegistryParser::xml_find_required_node(const XmlNode& start_node, const std::string& name) const
{
	for (auto node = &start_node; node != nullptr; node = node->sibling)
	{
		if (node->name == name)
		{
			return *node;
		}
	}

	BSTONE_GLAPIGEN_FAIL("Not found. ({})", name);
}

void RegistryParser::register_type(const RegistryType& type)
{
	if (type.name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Empty type name.");
	}

	if (type.value.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Empty value. ({}).", type.name);
	}

	if (contains_key(registry_.type_map, type.name))
	{
		BSTONE_GLAPIGEN_FAIL("Already registered. ({})", type.name);
	}

	registry_.type_map[type.name] = type;
	registry_.type_names.emplace_back(type.name);
}

void RegistryParser::register_enum(const RegistryEnum& registry_enum)
{
	if (registry_enum.name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Empty name.");
	}

	if (registry_enum.value.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Empty value.");
	}

	if (!registry_enum.alias.empty() && !contains_key(registry_.enum_map, registry_enum.alias))
	{
		BSTONE_GLAPIGEN_FAIL("Unregistered alias. ({})", registry_enum.alias);
	}

	if (contains_key(registry_.enum_map, registry_enum.name))
	{
		if (contains_key(registry_.enum_map[registry_enum.name], registry_enum.api))
		{
			BSTONE_GLAPIGEN_FAIL(
				"Already registered. (name: {}; api: {})",
				registry_enum.name,
				to_string(registry_enum.api));
		}
	}

	auto& value_map = registry_.enum_map[registry_enum.name];
	value_map[registry_enum.api] = registry_enum;
	registry_.enum_map[registry_enum.name] = value_map;

	if (!contains(registry_.enum_names, registry_enum.name))
	{
		registry_.enum_names.emplace_back(registry_enum.name);
	}
}

void RegistryParser::register_command(const RegistryCommand& command)
{
	const auto& name = command.proto.name;

	if (name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Empty proto name.");
	}

	for (const auto& param : command.params)
	{
		if (param.name.empty())
		{
			BSTONE_GLAPIGEN_FAIL("Empty param name.");
		}

		if (param.type.empty())
		{
			BSTONE_GLAPIGEN_FAIL("Empty param type.");
		}
	}

	if (contains_key(registry_.command_map, name))
	{
		BSTONE_GLAPIGEN_FAIL("Already registered. ({})", name);
	}

	registry_.command_map[name] = command;
	registry_.command_names.emplace_back(name);
}

void RegistryParser::register_feature(const RegistryFeature& feature)
{
	if (feature.name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Empty name.");
	}

	if (contains_key(registry_.feature_map, feature.name))
	{
		BSTONE_GLAPIGEN_FAIL("Already registered. ({})", feature.name);
	}

	registry_.feature_map[feature.name] = feature;
	registry_.feature_names.emplace_back(feature.name);
}

void RegistryParser::register_extension(const RegistryExtension& extension)
{
	if (extension.name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Empty name.");
	}

	if (contains_key(registry_.extension_map, extension.name))
	{
		BSTONE_GLAPIGEN_FAIL("Already registered. ({})", extension.name);
	}

	registry_.extension_map[extension.name] = extension;
	registry_.extension_names.emplace_back(extension.name);
}

void RegistryParser::parse_type_include(const std::string& name, const std::string& value)
{
	auto type = RegistryType{};
	type.is_header = true;
	type.name = name;
	type.value = value;

	register_type(type);
}

void RegistryParser::parse_type_alias(const std::string& name, const XmlNode& first_xml_node)
{
	auto type = RegistryType{};
	type.name = name;

	extract_full_value(first_xml_node, type);
	register_type(type);
}

void RegistryParser::parse_type_struct(const std::string& name, const XmlNode& first_xml_node)
{
	auto type = RegistryType{};
	type.name = name;

	extract_full_value(first_xml_node, type);
	register_type(type);
}

std::string RegistryParser::parse_type_name(const XmlNode& type_xml_node) const
{
	const auto name_string = std::string{"name"};
	auto name = std::string{};

	for (const auto& xml_attribute : type_xml_node.attributes)
	{
		if (xml_attribute.name != name_string)
		{
			continue;
		}

		name = xml_attribute.value;
	}

	for (auto xml_node = type_xml_node.child; xml_node != nullptr; xml_node = xml_node->sibling)
	{
		if (xml_node->is_text() ||
			xml_node->name != name_string ||
			xml_node->child == nullptr)
		{
			continue;
		}

		name = xml_node->child->value;
	}

	if (name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Undefined name.");
	}

	return name;
}

void RegistryParser::parse_type(const XmlNode& type_xml_node)
{
	const auto name = parse_type_name(type_xml_node);

	auto xml_node = type_xml_node.child;

	if (xml_node == nullptr)
	{
		BSTONE_GLAPIGEN_FAIL("Expected XML child text node.");
	}

	const auto& text_1 = xml_node->value;

	if (false) {}
	else if (starts_with(text_1, "#include <") && ends_with(text_1, '>'))
	{
		parse_type_include(name, text_1);
	}
	else if (starts_with(text_1, "typedef ") || starts_with(text_1, "#ifdef "))
	{
		parse_type_alias(name, *xml_node);
	}
	else if (starts_with(name, "struct "))
	{
		parse_type_struct(name, *xml_node);
	}
	else
	{
		BSTONE_GLAPIGEN_FAIL("Unknown type. ({})", name);
	}
}

void RegistryParser::parse_types(const XmlNode& first_xml_node)
{
	for (auto xml_node = &first_xml_node;
		xml_node != nullptr;
		xml_node = xml_node->sibling)
	{
		if (xml_node->is_text() || xml_node->name != "types")
		{
			continue;
		}

		for (auto xml_subnode = xml_node->child;
			xml_subnode != nullptr;
			xml_subnode = xml_subnode->sibling)
		{
			if (xml_subnode->name != "type")
			{
				continue;
			}

			parse_type(*xml_subnode);
		}
	}
}

void RegistryParser::parse_enum(const XmlNode& enum_xml_node, bool is_bitmask)
{
	auto registry_enum = RegistryEnum{};
	registry_enum.is_bitmask = is_bitmask;

	auto api_string = std::string{};
	auto& name = registry_enum.name;
	auto& value = registry_enum.value;
	auto& alias = registry_enum.alias;
	auto c_type_suffix_string = std::string{};

	for (const auto& xml_attribute : enum_xml_node.attributes)
	{
		const auto& attribute_name = xml_attribute.name;
		const auto& attribute_value = xml_attribute.value;

		if (false) {}
		else if (attribute_name == "api")
		{
			api_string = attribute_value;
		}
		else if (attribute_name == "name")
		{
			name = attribute_value;
		}
		else if (attribute_name == "value")
		{
			value = attribute_value;
		}
		else if (attribute_name == "alias")
		{
			alias = attribute_value;
		}
		else if (attribute_name == "type")
		{
			c_type_suffix_string = attribute_value;
		}
	}

	auto& api = registry_enum.api;
	api = RegistryApi::gl;

	if (!api_string.empty())
	{
		api = map_api(api_string);
	}

	auto& c_type_suffix = registry_enum.c_type_suffix;
	c_type_suffix = RegistryCTypeSuffix::none;

	if (!c_type_suffix_string.empty())
	{
		c_type_suffix = map_c_type_suffix(c_type_suffix_string);
	}

	register_enum(registry_enum);
}

void RegistryParser::parse_enums(const XmlNode& first_xml_node)
{
	static const auto enums_string = std::string{"enums"};
	static const auto enum_string = std::string{"enum"};

	auto is_bitmask = false;

	for (auto xml_node = &first_xml_node;
		xml_node != nullptr;
		xml_node = xml_node->sibling)
	{
		if (xml_node->name != enums_string)
		{
			continue;
		}

		for (const auto& xml_attribute : xml_node->attributes)
		{
			const auto& name = xml_attribute.name;
			const auto& value = xml_attribute.value;

			if (false) {}
			else if (name == "namespace")
			{
				if (value != "GL")
				{
					BSTONE_GLAPIGEN_FAIL("Unknown namespace. ({})", value);
				}
			}
			else if (name == "type")
			{
				if (false) {}
				else if (value.empty())
				{}
				else if (value == "bitmask")
				{
					is_bitmask = true;
				}
				else
				{
					BSTONE_GLAPIGEN_FAIL("Unknown type. ({})", value);
				}
			}
		}

		for (auto child_xml_node = xml_node->child;
			child_xml_node != nullptr;
			child_xml_node = child_xml_node->sibling)
		{
			if (child_xml_node->name != enum_string)
			{
				continue;
			}

			parse_enum(*child_xml_node, is_bitmask);
		}
	}
}

RegistryCommandParam RegistryParser::parse_command_proto_or_param(const XmlNode& first_xml_node) const
{
	auto type = std::string{};
	auto name = std::string{};

	for (auto xml_node = first_xml_node.child; xml_node != nullptr; xml_node = xml_node->sibling)
	{
		if (xml_node->is_text())
		{
			type += xml_node->value;
			continue;
		}

		if (false) {}
		else if (xml_node->name == "ptype")
		{
			if (xml_node->child == nullptr)
			{
				BSTONE_GLAPIGEN_FAIL("Expected XML child node.");
			}

			const auto& type_string = xml_node->child->value;

			if (!contains_key(registry_.type_map, type_string))
			{
				BSTONE_GLAPIGEN_FAIL("Undefined type. ({})", type_string);
			}

			type += type_string;
		}
		else if (xml_node->name == "name")
		{
			if (xml_node->child == nullptr)
			{
				BSTONE_GLAPIGEN_FAIL("Expected XML child node.");
			}

			name = xml_node->child->value;
		}
	}

	if (type.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Undefined type.");
	}

	if (name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Undefined name.");
	}

	auto result = RegistryCommandParam{};
	result.name = name;
	result.type = type;

	return result;
}

RegistryCommandParam RegistryParser::parse_command_proto(const XmlNode& first_xml_node)
{
	for (const auto& xml_attribute : first_xml_node.attributes)
	{
		const auto& name = xml_attribute.name;

		if (false) {}
		else if (name == "group" || name == "class" || name == "kind")
		{}
		else
		{
			BSTONE_GLAPIGEN_FAIL("Unknown attribute. ({})", name);
		}
	}

	return parse_command_proto_or_param(first_xml_node);
}

RegistryCommandParam RegistryParser::parse_command_param(const XmlNode& first_xml_node)
{
	for (const auto& xml_attribute : first_xml_node.attributes)
	{
		const auto& name = xml_attribute.name;

		if (false) {}
		else if (name == "group" || name == "len" || name == "class" || name == "kind")
		{}
		else
		{
			BSTONE_GLAPIGEN_FAIL("Unknown attribute. ({})", name);
		}
	}

	return parse_command_proto_or_param(first_xml_node);
}

void RegistryParser::set_command_proto(const RegistryCommandParam& proto, RegistryCommand& command)
{
	if (command.proto.name.empty())
	{
		command.proto.name = proto.name;
	}
	else
	{
		if (command.proto.name != proto.name)
		{
			BSTONE_GLAPIGEN_FAIL("Name redefinition.");
		}
	}

	if (command.proto.type.empty())
	{
		command.proto.type = proto.type;
	}
	else
	{
		if (command.proto.type != proto.type)
		{
			BSTONE_GLAPIGEN_FAIL("Type redefinition.");
		}
	}
}

void RegistryParser::add_command_param(const RegistryCommandParam& param, RegistryCommand& command)
{
	auto param_iter = std::find_if(
		command.params.begin(),
		command.params.end(),
		[&param](const RegistryCommandParam& old_param)
		{
			return old_param.name == param.name;
		});

	if (param_iter != command.params.end())
	{
		if (param_iter->type != param.type)
		{
			BSTONE_GLAPIGEN_FAIL("Type redefinition.");
		}

		return;
	}

	command.params.emplace_back(param);
}

void RegistryParser::parse_command(const XmlNode& first_xml_node)
{
	auto command = RegistryCommand{};
	auto was_proto = false;

	for (auto xml_node = first_xml_node.child; xml_node != nullptr; xml_node = xml_node->sibling)
	{
		if (xml_node->is_text())
		{
			continue;
		}

		const auto& name = xml_node->name;

		if (false) {}
		else if (name == "proto")
		{
			const auto proto = parse_command_proto(*xml_node);
			set_command_proto(proto, command);
			was_proto = true;
		}
		else if (name == "param")
		{
			const auto param = parse_command_param(*xml_node);
			add_command_param(param, command);
		}
		else if (name == "alias" || name == "vecequiv" || name == "glx")
		{}
		else
		{
			BSTONE_GLAPIGEN_FAIL("Unknown element. ({})", name);
		}
	}

	if (!was_proto)
	{
		BSTONE_GLAPIGEN_FAIL("No proto.");
	}

	register_command(command);
}

void RegistryParser::parse_commands(const XmlNode& first_xml_node)
{
	static const auto commands_string = std::string{"commands"};
	static const auto command_string = std::string{"command"};

	for (auto xml_node = &first_xml_node;
		xml_node != nullptr;
		xml_node = xml_node->sibling)
	{
		if (xml_node->name != commands_string)
		{
			continue;
		}

		for (auto child_xml_node = xml_node->child;
			child_xml_node != nullptr;
			child_xml_node = child_xml_node->sibling)
		{
			if (child_xml_node->name != command_string)
			{
				continue;
			}

			parse_command(*child_xml_node);
		}
	}
}

void RegistryParser::parse_feature_require_command(const XmlNode& first_xml_node, RegistryFeature& feature)
{
	auto command_name = std::string{};

	for (const auto& xml_attribute : first_xml_node.attributes)
	{
		if (xml_attribute.name != "name")
		{
			continue;
		}

		command_name = xml_attribute.value;
	}

	if (command_name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Undefined command name.");
	}

	auto& commands = feature.commands;

	if (contains(commands, command_name))
	{
		return;
	}

	commands.emplace_back(command_name);
}

void RegistryParser::parse_feature_requires(const XmlNode& first_xml_node, RegistryFeature& feature)
{
	for (auto xml_node = &first_xml_node; xml_node != nullptr; xml_node = xml_node->sibling)
	{
		if (xml_node->is_text() || xml_node->name != "command")
		{
			continue;
		}

		parse_feature_require_command(*xml_node, feature);
	}
}

void RegistryParser::parse_feature_remove_command(const XmlNode& first_xml_node)
{
	auto command_name = std::string{};

	for (const auto& xml_attribute : first_xml_node.attributes)
	{
		if (xml_attribute.name != "name")
		{
			continue;
		}

		command_name = xml_attribute.value;
	}

	if (command_name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Undefined command name.");
	}

	registry_.feature_removed_names.emplace(command_name);
}

void RegistryParser::parse_feature_removes(const XmlNode& first_xml_node)
{
	for (auto xml_node = &first_xml_node; xml_node != nullptr; xml_node = xml_node->sibling)
	{
		if (xml_node->is_text() || xml_node->name != "command")
		{
			continue;
		}

		parse_feature_remove_command(*xml_node);
	}
}

void RegistryParser::validate_feature_number(const std::string& number)
{
	if (number.size() != 3 ||
		!ascii_is_digit(number[0]) ||
		number[0] == '0' ||
		number[1] != '.' ||
		!ascii_is_digit(number[2]))
	{
		BSTONE_GLAPIGEN_FAIL("Unsupported number.");
	}
}

void RegistryParser::parse_feature(const XmlNode& first_xml_node)
{
	auto feature = RegistryFeature{};
	auto api_string = std::string{};

	for (const auto& xml_attribute : first_xml_node.attributes)
	{
		const auto& name = xml_attribute.name;
		const auto& value = xml_attribute.value;

		if (false) {}
		else if (name == "api")
		{
			api_string = value;
		}
		else if (name == "name")
		{
			feature.name = value;
		}
		else if (name == "number")
		{
			validate_feature_number(value);
			feature.number = value;
		}
		else
		{
			BSTONE_GLAPIGEN_FAIL("Unknown attribute. ({})", name);
		}
	}

	if (feature.name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Undefined name.");
	}

	feature.api = map_api(api_string);

	if (feature.api == RegistryApi::none)
	{
		BSTONE_GLAPIGEN_FAIL("Undefined API. ({})", feature.name);
	}

	if (feature.number.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Undefined number. ({})", feature.name);
	}

	const auto is_api_gl = feature.api == RegistryApi::gl;
	const auto is_number_3_2 = feature.number == "3.2";

	for (auto xml_node = first_xml_node.child; xml_node != nullptr; xml_node = xml_node->sibling)
	{
		if (xml_node->is_text())
		{
			continue;
		}

		const auto& name = xml_node->name;

		if (false) {}
		else if (name == "require")
		{
			if (xml_node->child != nullptr)
			{
				parse_feature_requires(*xml_node->child, feature);
			}
		}
		else if (name == "remove")
		{
			if (!is_api_gl || !is_number_3_2)
			{
				BSTONE_GLAPIGEN_FAIL("Expected GL v3.2.");
			}

			auto profile = RegistryProfile::none;

			for (const auto& xml_attribute : xml_node->attributes)
			{
				if (xml_attribute.name == "profile")
				{
					profile = map_profile(xml_attribute.value);
				}
			}

			if (profile != RegistryProfile::core)
			{
				BSTONE_GLAPIGEN_FAIL("Expected core profile.");
			}

			if (xml_node->child != nullptr)
			{
				parse_feature_removes(*xml_node->child);
			}
		}
		else
		{
			BSTONE_GLAPIGEN_FAIL("Unknown element. ({})", name);
		}
	}

	if (feature.commands.empty())
	{
		BSTONE_GLAPIGEN_FAIL("No commands. ({})", feature.name);
	}

	register_feature(feature);
}

void RegistryParser::parse_features(const XmlNode& first_xml_node)
{
	for (auto xml_node = &first_xml_node;
		xml_node != nullptr;
		xml_node = xml_node->sibling)
	{
		if (xml_node->name != "feature")
		{
			continue;
		}

		parse_feature(*xml_node);
	}
}

void RegistryParser::parse_extension_require_command(const XmlNode& first_xml_node, RegistryExtension& extension)
{
	auto name = std::string{};

	for (const auto& xml_attribute : first_xml_node.attributes)
	{
		if (false) {}
		else if (xml_attribute.name == "name")
		{
			name = xml_attribute.value;
		}
	}

	if (name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Undefined name.");
	}

	if (contains(extension.commands, name))
	{
		return;
	}

	extension.commands.emplace_back(name);
}

void RegistryParser::parse_extension_require(const XmlNode& first_xml_node, RegistryExtension& extension)
{
	for (auto xml_node = first_xml_node.child;
		xml_node != nullptr;
		xml_node = xml_node->sibling)
	{
		if (xml_node->name != "command")
		{
			continue;
		}

		parse_extension_require_command(*xml_node, extension);
	}
}

void RegistryParser::parse_extension(const XmlNode& first_xml_node)
{
	auto extension = RegistryExtension{};

	for (const auto& xml_attribute : first_xml_node.attributes)
	{
		if (false) {}
		else if (xml_attribute.name == "name")
		{
			extension.name = xml_attribute.value;
		}
		else if (xml_attribute.name == "supported")
		{
			if (xml_attribute.value == "disabled")
			{
				return;
			}

			extension.api_set = map_extension_api_set(xml_attribute.value);
		}
	}

	if (extension.name.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Undefined name.");
	}

	if (extension.api_set.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Undefined API set.");
	}

	for (auto xml_node = first_xml_node.child;
		xml_node != nullptr;
		xml_node = xml_node->sibling)
	{
		if (xml_node->name != "require")
		{
			continue;
		}

		parse_extension_require(*xml_node, extension);
	}

	register_extension(extension);
}

void RegistryParser::parse_extensions(const XmlNode& first_xml_node)
{
	for (auto xml_node = &first_xml_node;
		xml_node != nullptr;
		xml_node = xml_node->sibling)
	{
		if (xml_node->name != "extensions")
		{
			continue;
		}

		for (auto xml_subnode = xml_node->child;
			xml_subnode != nullptr;
			xml_subnode = xml_subnode->sibling)
		{
			if (xml_subnode->name != "extension")
			{
				continue;
			}

			parse_extension(*xml_subnode);
		}
	}
}

void RegistryParser::parse()
{
	const auto registry_name = std::string{"registry"};

	const auto& xml_first_node = xml_nodes_.get_first();
	const auto& registry_xml_node = xml_find_required_node(xml_first_node, registry_name);

	if (registry_xml_node.child == nullptr)
	{
		BSTONE_GLAPIGEN_FAIL("Empty root XML node. ({})", registry_name);
	}

	const auto& registry_first_xml_node = *registry_xml_node.child;

	parse_types(registry_first_xml_node);
	parse_enums(registry_first_xml_node);
	parse_commands(registry_first_xml_node);
	parse_features(registry_first_xml_node);
	parse_extensions(registry_first_xml_node);
}

// ===========================================================================

struct GeneratorParam
{
	const Registry* registry;
	bool is_indented_by_space;
	int indent_size;
	std::string header_file_path;
	std::string source_file_path;
	std::string include;
	bool is_include_angled;
	Strings namespaces;
	bool are_symbols_local;
	std::string symbols_name;
	std::string header_guard;

	bool has_namespaces() const noexcept;
};

// ---------------------------------------------------------------------------

bool GeneratorParam::has_namespaces() const noexcept
{
	return !namespaces.empty();
}

// ===========================================================================

class Generator
{
public:
	explicit Generator(const GeneratorParam& param);

private:
	static constexpr auto apientry_string = "GL_APIENTRY";
	static constexpr auto proto_prefix = "PFN";
	static constexpr auto proto_suffix = "PROC";

private:
	const GeneratorParam& param_;
	const Registry& registry_;
	std::ofstream stream_{};
	std::string buffer_{};
	std::string indentation_string_{};

private:
	void open_file(const std::string& file_path);
	void flush();

	void write(const char* string);
	void write(const std::string& string);
	void write_eol();
	void write_eol_if_need();
	void write_line(const char* string);
	void write_line(const std::string& string);
	void write_boilerplate();

	void write_namespaces_begin();
	void write_namespaces_end();

	void write_enum_value(const RegistryEnum& registry_enum);
	void write_commented_ambiguous_enum_values(const RegistryApiToEnumMap& enum_map);

	void write_header_guard_begin();
	void write_header_guard_end();
	void write_header_includes_platform();
	void write_header_includes_header();
	void write_header_macros();
	void write_header_types();
	void write_header_ambiguous_enum_declaration(const RegistryApiToEnumMap& enum_map);
	void write_header_unambiguous_enum(const RegistryEnum& registry_enum);
	void write_header_enum_global_declarations();
	void write_header_command_prototypes();
	void write_header_command_declarations();
	void write_header_struct_enum_declarations();
	void write_header_struct_command_declarations();
	void write_header_struct_declaration_begin();
	void write_header_struct_declaration_end();
	void write_header_struct_declaration();
	void write_header();

	void write_source_ambiguous_enum_definition(const RegistryApiToEnumMap& enum_map);
	void write_source_enum_definitions();
	void write_source_command_definitions();
	void write_source();
};

// ---------------------------------------------------------------------------

Generator::Generator(const GeneratorParam& param)
	:
	param_{param},
	registry_{*param.registry}
{
	if (param_.is_indented_by_space)
	{
		indentation_string_.assign(static_cast<std::size_t>(param_.indent_size), ' ');
	}
	else
	{
		indentation_string_ = "\t";
	}

	open_file(param_.header_file_path);
	write_header();

	open_file(param_.source_file_path);
	write_source();
}

void Generator::open_file(const std::string& file_path)
{
	stream_.close();
	stream_.open(file_path, std::ios_base::out | std::ios_base::trunc);

	if (!stream_.is_open())
	{
		BSTONE_GLAPIGEN_FAIL("Failed to open a file for writing. ({})", file_path);
	}
}

void Generator::flush()
{
	stream_ << buffer_;
	buffer_.clear();
}

void Generator::write(const char* string)
{
	buffer_ += string;
}

void Generator::write(const std::string& string)
{
	buffer_ += string;
}

void Generator::write_eol_if_need()
{
	if (buffer_.size() >= 2 && buffer_.back() == '\n' && buffer_[buffer_.size() - 2] == '\n')
	{
		return;
	}

	write_eol();
}

void Generator::write_eol()
{
	buffer_ += '\n';
}

void Generator::write_line(const char* string)
{
	write(string);
	write_eol();
}

void Generator::write_line(const std::string& string)
{
	write(string);
	write_eol();
}

void Generator::write_boilerplate()
{
	write_line("/* This file was generated by " CMAKE_PROJECT_NAME " v" CMAKE_PROJECT_VERSION ". */");
}

void Generator::write_namespaces_begin()
{
	if (!param_.has_namespaces())
	{
		return;
	}

	write_eol_if_need();
	write_line("#ifdef __cplusplus");

	for (const auto& ns : param_.namespaces)
	{
		write("namespace ");
		write(ns);
		write(" {");
		write_eol();
	}

	write_line("#endif /* __cplusplus */");
}

void Generator::write_namespaces_end()
{
	if (!param_.has_namespaces())
	{
		return;
	}

	write_eol_if_need();
	write_line("#ifdef __cplusplus");

	const auto& namespaces = param_.namespaces;

	for (auto iter = namespaces.rbegin(), iter_end = namespaces.rend(); iter != iter_end; ++iter)
	{
		write("} /* namespace ");
		write(*iter);
		write(" */");
		write_eol();
	}

	write_line("#endif /* __cplusplus */");
}

void Generator::write_enum_value(const RegistryEnum& registry_enum)
{
	write(registry_enum.value);

	switch (registry_enum.c_type_suffix)
	{
		case RegistryCTypeSuffix::none: break;
		case RegistryCTypeSuffix::u: write("U"); break;
		case RegistryCTypeSuffix::ull: write("ULL"); break;

		default: BSTONE_GLAPIGEN_FAIL("Unknown C-type suffix.");
	}
}

void Generator::write_commented_ambiguous_enum_values(const RegistryApiToEnumMap& enum_map)
{
	write("/* ");

	auto is_first = true;

	for (const auto& enum_item : enum_map)
	{
		const auto api_string = to_string(enum_item.first);
		const auto& enum_value = enum_item.second;

		if (!is_first)
		{
			write("; ");
		}

		write(api_string);
		write("=");
		write_enum_value(enum_value);

		is_first = false;
	}

	write(" */");
}

void Generator::write_header_guard_begin()
{
	write_eol_if_need();

	write("#ifndef ");
	write(param_.header_guard);
	write_eol();

	write("#define ");
	write(param_.header_guard);
	write_eol();
}

void Generator::write_header_guard_end()
{
	write_eol_if_need();

	write("#endif /* ");
	write(param_.header_guard);
	write(" */");
	write_eol();
}

void Generator::write_header_includes_platform()
{
	auto is_first = true;

	for (const auto& type_name : registry_.type_names)
	{
		const auto& type = registry_.type_map.at(type_name);

		if (!type.is_header)
		{
			continue;
		}

		if (is_first)
		{
			is_first = false;
			write_eol_if_need();
		}

		write_line(type.value);
	}
}

void Generator::write_header_includes_header()
{
	write_eol_if_need();

	write("#include ");
	write(param_.is_include_angled ? "<" : "\"");
	write(param_.include);
	write(param_.is_include_angled ? ">" : "\"");
	write_eol();
}

void Generator::write_header_macros()
{
	write_eol_if_need();

	write("#ifndef ");
	write(apientry_string);
	write_eol();

	write("#define ");
	write(apientry_string);
	write(" KHRONOS_APIENTRY");
	write_eol();

	write("#endif /* ");
	write(apientry_string);
	write(" */");
	write_eol();
}

void Generator::write_header_types()
{
	auto is_first = true;

	for (const auto& type_name : registry_.type_names)
	{
		const auto& type = registry_.type_map.at(type_name);

		if (type.is_header)
		{
			continue;
		}

		if (is_first)
		{
			is_first = false;
			write_eol_if_need();
		}

		if (type.has_apientry())
		{
			auto result = std::string{};
			result.reserve(type.value.size() + 32);
			result = type.value;
			result.insert(type.apientry_offset, apientry_string);

			write_line(result);
		}
		else
		{
			write_line(type.value);
		}
	}
}

void Generator::write_header_ambiguous_enum_declaration(const RegistryApiToEnumMap& enum_map)
{
	if (!param_.are_symbols_local)
	{
		write("extern ");
	}
	else
	{
		write(indentation_string_);
	}

	const auto& enum_value = enum_map.begin()->second;

	write("GLenum ");
	write(enum_value.name);
	write("; ");
	write_commented_ambiguous_enum_values(enum_map);
	write_eol();
}

void Generator::write_header_unambiguous_enum(const RegistryEnum& registry_enum)
{
	write("#define ");
	write(registry_enum.name);
	write(" ");
	write_enum_value(registry_enum);
	write_eol();
}

void Generator::write_header_enum_global_declarations()
{
	auto is_first = true;

	for (const auto& enum_name : registry_.enum_names)
	{
		const auto& enum_item = registry_.enum_map.at(enum_name);
		const auto is_ambiguous = enum_item.size() > 1;

		if (is_ambiguous && param_.are_symbols_local)
		{
			continue;
		}

		if (is_first)
		{
			is_first = false;
			write_eol_if_need();
		}

		if (is_ambiguous)
		{
			write_header_ambiguous_enum_declaration(enum_item);
		}
		else
		{
			const auto& enum_value = enum_item.cbegin()->second;
			write_header_unambiguous_enum(enum_value);
		}
	}
}

void Generator::write_header_command_prototypes()
{
	auto is_first_command = true;

	for (const auto& command_name : registry_.command_names)
	{
		const auto& command = registry_.command_map.at(command_name);

		if (is_first_command)
		{
			is_first_command = false;
			write_eol_if_need();
		}

		write("typedef ");
		write(command.proto.type);
		write("(");
		write(apientry_string);
		write(" * ");
		write(proto_prefix);
		write(ascii_to_upper(command.proto.name));
		write(proto_suffix);
		write(")(");

		if (command.params.empty())
		{
			write("void");
		}
		else
		{
			auto is_first_param = true;

			for (const auto& param : command.params)
			{
				if (!is_first_param)
				{
					write(",");
				}

				write(param.type);
				write(param.name);

				is_first_param = false;
			}
		}

		write(");");
		write_eol();
	}
}

void Generator::write_header_command_declarations()
{
	if (param_.are_symbols_local)
	{
		return;
	}

	auto is_first_command = true;

	for (const auto& command_name : registry_.command_names)
	{
		const auto& command = registry_.command_map.at(command_name);

		if (is_first_command)
		{
			is_first_command = false;
			write_eol_if_need();
		}

		write("extern ");
		write(proto_prefix);
		write(ascii_to_upper(command.proto.name));
		write(proto_suffix);
		write(" ");
		write(command.proto.name);
		write(";");
		write_eol();
	}
}

void Generator::write_header_struct_enum_declarations()
{
	auto is_first = true;

	for (const auto& enum_name : registry_.enum_names)
	{
		const auto& enum_item = registry_.enum_map.at(enum_name);

		if (enum_item.size() == 1)
		{
			continue;
		}

		if (is_first)
		{
			is_first = false;
			write_eol_if_need();
		}

		write_header_ambiguous_enum_declaration(enum_item);
	}
}

void Generator::write_header_struct_command_declarations()
{
	auto is_first_command = true;

	for (const auto& command_name : registry_.command_names)
	{
		const auto& command = registry_.command_map.at(command_name);

		if (is_first_command)
		{
			is_first_command = false;
			write_eol_if_need();
		}

		write(indentation_string_);
		write(proto_prefix);
		write(ascii_to_upper(command.proto.name));
		write(proto_suffix);
		write(" ");
		write(command.proto.name);
		write(";");
		write_eol();
	}
}

void Generator::write_header_struct_declaration_begin()
{
	write("struct ");
	write(param_.symbols_name);
	write(" {");
	write_eol();
}

void Generator::write_header_struct_declaration_end()
{
	write("}; /* ");
	write(param_.symbols_name);
	write(" */");
	write_eol();
}

void Generator::write_header_struct_declaration()
{
	if (!param_.are_symbols_local)
	{
		return;
	}

	write_eol_if_need();
	write_header_struct_declaration_begin();
	write_header_struct_enum_declarations();
	write_header_struct_command_declarations();
	write_header_struct_declaration_end();
}

void Generator::write_header()
{
	write_boilerplate();
	write_eol();
	write_header_guard_begin();
	write_header_includes_platform();
	write_header_macros();
	write_namespaces_begin();
	write_header_types();
	write_header_enum_global_declarations();
	write_header_command_prototypes();
	write_header_command_declarations();
	write_header_struct_declaration();
	write_namespaces_end();
	write_header_guard_end();
	flush();
}

void Generator::write_source_ambiguous_enum_definition(const RegistryApiToEnumMap& enum_map)
{
	if (param_.are_symbols_local)
	{
		return;
	}

	const auto& renum = enum_map.begin()->second;

	write("GLenum ");
	write(renum.name);
	write(" = 0; ");
	write_commented_ambiguous_enum_values(enum_map);
	write_eol();
}

void Generator::write_source_enum_definitions()
{
	auto is_first = true;

	for (const auto& enum_name : registry_.enum_names)
	{
		const auto& enum_item = registry_.enum_map.at(enum_name);

		if (enum_item.size() == 1)
		{
			continue;
		}

		if (is_first)
		{
			is_first = false;
			write_eol_if_need();
		}

		write_source_ambiguous_enum_definition(enum_item);
	}
}

void Generator::write_source_command_definitions()
{
	if (param_.are_symbols_local)
	{
		return;
	}

	auto is_first_command = true;

	for (const auto& command_name : registry_.command_names)
	{
		const auto& command = registry_.command_map.at(command_name);

		if (is_first_command)
		{
			is_first_command = false;
			write_eol_if_need();
		}

		write(proto_prefix);
		write(ascii_to_upper(command.proto.name));
		write(proto_suffix);
		write(" ");
		write(command.proto.name);
		write(" = 0;");
		write_eol();
	}
}

void Generator::write_source()
{
	write_boilerplate();
	write_eol();
	write_header_includes_header();
	write_namespaces_begin();
	write_source_enum_definitions();
	write_source_command_definitions();
	write_namespaces_end();
	flush();
}

// ===========================================================================

std::string fs_get_file_name(const std::string& file_path)
{
	constexpr auto separators =
#ifdef _WIN32
		":\\"
#else
		"/"
#endif
		;

	const auto separator_position = file_path.find_last_of(separators);

	if (separator_position == std::string::npos)
	{
		return file_path;
	}

	return file_path.substr(separator_position + 1);
}

// ===========================================================================

std::string make_header_guard_from_file_path(const std::string& header_file_path)
{
	auto result = fs_get_file_name(header_file_path);

	for (auto& ch : result)
	{
		if (ch == '_' ||
			(ch >= '0' && ch <= '9') ||
			(ch >= 'A' && ch <= 'Z'))
		{
			continue;
		}

		if (ch >= 'a' && ch <= 'z')
		{
			ch = static_cast<char>(ch + 'A' - 'a');
		}
		else
		{
			ch = '_';
		}
	}

	result += "_INCLUDED";
	return result;
}

// ===========================================================================

void cl_get_option_name_and_value(const std::string& option, std::string& name, std::string& value)
{
	name.clear();
	value.clear();

	if (option.size() < 2 || (option[0] != '-' && option[1] != '-'))
	{
		BSTONE_GLAPIGEN_FAIL("Expected an option. ({})", option);
	}

	const auto equals_sign_index = option.find_first_of('=');

	if (equals_sign_index == std::string::npos)
	{
		name = option.substr(2);
		return;
	}

	name = option.substr(2, equals_sign_index - 2);
	value = option.substr(equals_sign_index + 1);
}

bool cl_option_value_to_bool(const std::string& value_string)
{
	if (false) {}
	else if (value_string == "0")
	{
		return false;
	}
	else if (value_string == "1")
	{
		return true;
	}
	else
	{
		BSTONE_GLAPIGEN_FAIL("Invalid bool value. ({})", value_string);
	}
}

} // namespace

int main(int argc, char* argv[])
try
{
	std::cout << "BSTONE OpenGL API generator." << std::endl;

	constexpr auto default_indent_with_space = false;
	constexpr auto default_indent_size = 4;
	constexpr auto default_local_name = "GlSymbols";
	constexpr auto default_local = false;
	constexpr auto default_namespaces = "";
	constexpr auto default_include_angled = false;

	const auto registry_option_string = std::string{"registry"};
	const auto indent_with_space_option_string = std::string{"indent_with_space"};
	const auto indent_size_option_string = std::string{"indent_size"};
	const auto header_option_string = std::string{"header"};
	const auto source_option_string = std::string{"source"};
	const auto header_guard_option_string = std::string{"header_guard"};
	const auto include_option_string = std::string{"include"};
	const auto include_angled_option_string = std::string{"include_angled"};
	const auto namespaces_option_string = std::string{"namespaces"};
	const auto local_option_string = std::string{"local"};
	const auto local_name_option_string = std::string{"local_name"};

	if (argc == 1)
	{
		std::cerr << "Options:" << std::endl;

		// registry
		std::cerr << "  --" << registry_option_string << "=<path> - GL registry file path." << std::endl;

		// indent with space
		std::cerr << "  --" << indent_with_space_option_string <<
			"=<0|1> - use spaces or tabs for indentation (optional)." << std::endl;
		std::cerr << "    Default value: " << (default_indent_with_space ? '1' : '0') << std::endl;

		// indent with space
		std::cerr << "  --" << indent_size_option_string <<
			"=<space_count> - indentation size (optional)." << std::endl;
		std::cerr << "    Default value: " << default_indent_size << std::endl;

		// header
		std::cerr << "  --" << header_option_string << "=<path> - output file path for header." << std::endl;

		// source
		std::cerr << "  --" << source_option_string << "=<path> - output file path for source." << std::endl;

		// header guard
		std::cerr << "  --" << header_guard_option_string << "=<name> - custom header guard (optional)." << std::endl;
		std::cerr << "    Default value: header's file name, uppercased, and suffixed with \"_INCLUDED\"." << std::endl;
		std::cerr << "      Example for header name \"api.h\": API_H_INCLUDED" << std::endl;

		// include
		std::cerr << "  --" << include_option_string << "=<name> - custom name for include directive (optional)." << std::endl;

		// angled include
		std::cerr << "  --" << include_angled_option_string <<
			"=<0|1> - use angle brackets or quotation marks for #include (optional)." << std::endl;
		std::cerr << "    Default value: " << (default_include_angled ? '1' : '0') << std::endl;

		// namespaces
		std::cerr << "  --" << namespaces_option_string << "=<name> - namespace list (optional)." << std::endl;
		std::cerr << "    Use dot as seperator (i.e. \"one.two.three\")." << std::endl;
		std::cerr << "    Default value: \"" << default_namespaces << '"' << std::endl;

		// local
		std::cerr << "  --" << local_option_string << "=<0|1> - place symbols into the struct (optional)." << std::endl;
		std::cerr << "    Default value: " << (default_local ? '1' : '0') << std::endl;

		// local name
		std::cerr << "  --" << local_name_option_string << "=<name> - custom struct for local symbols (optional)." << std::endl;
		std::cerr << "    Default value: " << default_local_name << std::endl;

		return EXIT_FAILURE;
	}

	auto xml_file_path = std::string{};
	auto generator_param = GeneratorParam{};

	for (auto i = 1; i < argc; ++i)
	{
		const auto option = std::string{argv[i]};
		auto option_name = std::string{};
		auto option_value = std::string{};
		cl_get_option_name_and_value(option, option_name, option_value);

		if (false) {}
		else if (option_name == registry_option_string)
		{
			xml_file_path = option_value;
		}
		else if (option_name == indent_with_space_option_string)
		{
			generator_param.is_indented_by_space = cl_option_value_to_bool(option_value);
		}
		else if (option_name == indent_size_option_string)
		{
			generator_param.indent_size = std::stoi(option_value);

			if (generator_param.indent_size <= 0 || generator_param.indent_size > 8)
			{
				BSTONE_GLAPIGEN_FAIL("Indentation size out of range. ({})", option_value);
			}
		}
		else if (option_name == header_option_string)
		{
			generator_param.header_file_path = option_value;
		}
		else if (option_name == source_option_string)
		{
			generator_param.source_file_path = option_value;
		}
		else if (option_name == header_guard_option_string)
		{
			generator_param.header_guard = option_value;
		}
		else if (option_name == include_option_string)
		{
			generator_param.include = option_value;
		}
		else if (option_name == include_angled_option_string)
		{
			generator_param.is_include_angled = cl_option_value_to_bool(option_value);
		}
		else if (option_name == namespaces_option_string)
		{
			generator_param.namespaces = split(option_value, '.');
		}
		else if (option_name == local_option_string)
		{
			generator_param.are_symbols_local = cl_option_value_to_bool(option_value);
		}
		else if (option_name == local_name_option_string)
		{
			generator_param.symbols_name = option_value;
		}
		else
		{
			BSTONE_GLAPIGEN_FAIL("Unknown option. ({})", option);
		}
	}

	if (xml_file_path.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Registry file not specified.");
	}

	if (generator_param.indent_size <= 0)
	{
		generator_param.indent_size = default_indent_size;
	}

	if (generator_param.header_file_path.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Header file path not specified.");
	}

	if (generator_param.source_file_path.empty())
	{
		BSTONE_GLAPIGEN_FAIL("Source file path not specified.");
	}

	if (generator_param.symbols_name.empty())
	{
		generator_param.symbols_name = default_local_name;
	}

	if (generator_param.include.empty())
	{
		generator_param.include = fs_get_file_name(generator_param.header_file_path);
	}

	if (generator_param.header_guard.empty())
	{
		generator_param.header_guard = make_header_guard_from_file_path(generator_param.header_file_path);
	}

	auto namespaces_string = std::string{};

	for (const auto& namespace_string : generator_param.namespaces)
	{
		if (!namespaces_string.empty())
		{
			namespaces_string += "::";
		}

		namespaces_string += namespace_string;
	}

	std::cout << std::endl;
	std::cout << "Registry file: \"" << xml_file_path << '"' << std::endl;
	std::cout << "Header file: \"" << generator_param.header_file_path << '"' << std::endl;
	std::cout << "Source file: \"" << generator_param.source_file_path << '"' << std::endl;
	std::cout << "Indent with spaces: " << (generator_param.is_indented_by_space ? '1' : '0') << std::endl;
	std::cout << "Indentation size: " << generator_param.indent_size << std::endl;
	std::cout << "Header guard: " << generator_param.header_guard << std::endl;
	std::cout << "Include name: " << generator_param.include << std::endl;
	std::cout << "Include with angle brackets: " << generator_param.is_include_angled << std::endl;
	std::cout << "Namespaces: " << namespaces_string << std::endl;
	std::cout << "Are symbols local: " << (generator_param.are_symbols_local ? '1' : '0') << std::endl;
	std::cout << "Symbols struct name: " << generator_param.symbols_name << std::endl;
	std::cout << std::endl;

	auto registry = Registry{};

	{
		auto registry_parser = RegistryParser{xml_file_path};
		registry.swap(registry_parser.get_registry());
	}

	std::cout << "Registry:" << std::endl;
	std::cout << "  type count: " << registry.type_names.size() << std::endl;
	std::cout << "  enum count: " << registry.enum_names.size() << std::endl;
	std::cout << "  command count: " << registry.command_names.size() << std::endl;
	std::cout << "  feature count: " << registry.feature_names.size() << std::endl;
	std::cout << "  extension count: " << registry.extension_names.size() << std::endl;
	std::cout << "  removed command count (core): " << registry.feature_removed_names.size() << std::endl;

	generator_param.registry = &registry;
	const auto generator = Generator{generator_param};

	return EXIT_SUCCESS;
}
catch (const std::exception& exception)
{
	std::cerr << "[ERR] " << exception.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...)
{
	std::cerr << "[ERR] Generic failure." << std::endl;
	return EXIT_FAILURE;
}
