/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


//
// Content path.
//


#include "bstone_content_path.h"

#include "bstone_exception.h"


namespace bstone
{


namespace detail
{


AssetPath make_gog_content_path();

AssetPath make_steam_content_path();


class AssetPathException :
	public Exception
{
public:
	explicit AssetPathException(
		const char* const message)
		:
		Exception{std::string{"[CONTENT_PATH] "} + message}
	{
	}
}; // AssetPathException


} // detail


AssetPath make_content_path(
	const ContentPathProvider type)
{
	switch (type)
	{
		case ContentPathProvider::gog:
			return detail::make_gog_content_path();

		case ContentPathProvider::steam:
			return detail::make_steam_content_path();

		default:
			throw detail::AssetPathException{"Unsupported provider."};
	}
}


} // bstone
