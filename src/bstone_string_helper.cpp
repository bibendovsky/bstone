/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "bstone_string_helper.h"
#include <algorithm>
#include <functional>
#include <locale>


namespace
{


struct Internals final
{
	using CType = std::ctype<char>;


	const CType& get_ctype_facet() const
	{
		static std::locale locale;
		static const auto& result = std::use_facet<CType>(locale);
		return result;
	}

	char to_lower(
		const char value) const
	{
		return get_ctype_facet().tolower(value);
	}
}; // Internals


} // namespace


namespace bstone
{


char StringHelper::to_lower(
	const char value) const
{
	const auto& internals = Internals{};

	return internals.to_lower(value);
}

std::string StringHelper::to_lower(
	const std::string& value) const
{
	auto result = value;

	const auto& internals = Internals{};

	std::transform(result.begin(), result.end(), result.begin(), std::bind(&Internals::to_lower, internals, std::placeholders::_1));

	return result;
}

bool StringHelper::is_iequal(
	const std::string& a,
	const std::string& b) const
{
	auto result = std::mismatch(
		a.cbegin(),
		a.cend(),
		b.cbegin(),
		[](const auto lhs, const auto rhs)
		{
			const auto& internals = Internals{};

			return internals.get_ctype_facet().tolower(lhs) == internals.get_ctype_facet().tolower(rhs);
		}
	);

	return result.first == a.cend();
}

bool StringHelper::is(
	std::ctype_base::mask mask,
	const char value) const
{
	const auto& internals = Internals{};

	return internals.get_ctype_facet().is(mask, value);
}

const std::string& StringHelper::get_empty() const
{
	static const auto result = std::string{};

	return result;
}


} // bstone
