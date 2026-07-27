/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Minimal reader for Valve Data Format (VDF, also known as KeyValues) text.

#include "bstone_vdf.h"
#include "bstone_ascii.h"
#include <cstddef>

namespace bstone {

namespace {

// Guards against stack exhaustion from a corrupt or hostile file. Steam's own
// files nest three or four levels deep.
constexpr int max_depth = 32;

constexpr char utf8_bom[] = "\xEF\xBB\xBF";
constexpr std::size_t utf8_bom_size = 3;

bool is_space(char ch) noexcept
{
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

// A bare token ends at whitespace, a brace or a quote.
bool is_token_end(char ch) noexcept
{
	return is_space(ch) || ch == '{' || ch == '}' || ch == '"';
}

class VdfParser
{
public:
	VdfParser(std::string_view text) noexcept
		:
		text_{text}
	{
		// Steam writes UTF-8; some editors add a byte-order mark.
		if (text_.size() >= utf8_bom_size && text_.compare(0, utf8_bom_size, utf8_bom) == 0)
		{
			pos_ = utf8_bom_size;
		}
	}

	// Parses the top-level sequence of entries into `root`.
	bool parse(VdfNode& root)
	{
		while (true)
		{
			skip_space_and_comments();
			if (is_eof())
				return true;
			// A stray closing brace at the top level is malformed.
			if (peek() == '}')
				return false;
			VdfNode child;
			if (!parse_entry(child, 0))
				return false;
			root.children.push_back(std::move(child));
		}
	}

private:
	std::string_view text_;
	std::size_t pos_{};

	bool is_eof() const noexcept { return pos_ >= text_.size(); }
	char peek() const noexcept { return text_[pos_]; }

	void skip_space_and_comments() noexcept
	{
		while (!is_eof())
		{
			if (is_space(peek()))
			{
				++pos_;
			}
			else if (peek() == '/' && pos_ + 1 < text_.size() && text_[pos_ + 1] == '/')
			{
				while (!is_eof() && peek() != '\n')
					++pos_;
			}
			else
			{
				break;
			}
		}
	}

	// Reads a quoted or bare token. Returns false on an unterminated quote.
	bool parse_token(std::string& token)
	{
		token.clear();
		if (peek() != '"')
		{
			while (!is_eof() && !is_token_end(peek()))
			{
				token += text_[pos_];
				++pos_;
			}
			return !token.empty();
		}
		++pos_; // Opening quote.
		while (true)
		{
			if (is_eof())
				return false; // Unterminated.
			const char ch = text_[pos_];
			if (ch == '"')
			{
				++pos_; // Closing quote.
				return true;
			}
			if (ch == '\\')
			{
				++pos_;
				if (is_eof())
					return false; // Trailing backslash.
				// Steam writes Windows paths as "D:\\Games", so `\\` must collapse
				// to a single separator. An unknown escape keeps its literal
				// character, which is what KeyValues does.
				const char escaped = text_[pos_];
				switch (escaped)
				{
					case 'n': token += '\n'; break;
					case 't': token += '\t'; break;
					default: token += escaped; break;
				}
				++pos_;
				continue;
			}
			token += ch;
			++pos_;
		}
	}

	// Parses `name` followed by either a value token or a `{ ... }` block.
	bool parse_entry(VdfNode& node, int depth)
	{
		if (depth >= max_depth)
			return false;
		if (!parse_token(node.name))
			return false;
		skip_space_and_comments();
		if (is_eof())
			return false; // A name with neither value nor block.
		if (peek() != '{')
			return parse_token(node.value);
		++pos_; // Opening brace.
		while (true)
		{
			skip_space_and_comments();
			if (is_eof())
				return false; // Unterminated block.
			if (peek() == '}')
			{
				++pos_; // Closing brace.
				return true;
			}
			VdfNode child;
			if (!parse_entry(child, depth + 1))
				return false;
			node.children.push_back(std::move(child));
		}
	}
};

bool are_names_equal(std::string_view lhs, std::string_view rhs) noexcept
{
	if (lhs.size() != rhs.size())
		return false;
	for (std::size_t i = 0; i < lhs.size(); ++i)
	{
		if (ascii::to_lower(lhs[i]) != ascii::to_lower(rhs[i]))
			return false;
	}
	return true;
}

} // namespace

bool VdfNode::is_object() const noexcept
{
	return !children.empty();
}

const VdfNode* VdfNode::find_child(std::string_view child_name) const noexcept
{
	for (const VdfNode& child : children)
	{
		if (are_names_equal(child.name, child_name))
			return &child;
	}
	return nullptr;
}

std::string_view VdfNode::find_value(std::string_view child_name) const noexcept
{
	const VdfNode* const child = find_child(child_name);
	return child != nullptr ? std::string_view{child->value} : std::string_view{};
}

bool parse_vdf(std::string_view text, VdfNode& root) noexcept
try {
	root = VdfNode{};
	VdfParser parser{text};
	if (parser.parse(root))
		return true;
	root = VdfNode{};
	return false;
} catch (...) {
	// The only plausible failure is allocation; report it like malformed input.
	root = VdfNode{};
	return false;
}

} // namespace bstone
