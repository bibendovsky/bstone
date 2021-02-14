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
// OPL3 emulator interface.
//


#include "bstone_opl3.h"


namespace bstone
{


namespace detail
{


Opl3UPtr make_dbopl_opl3();


} // detail


Opl3UPtr make_opl3(
	const Opl3Type opl3_type)
{
	switch (opl3_type)
	{
		case Opl3Type::dbopl:
			return detail::make_dbopl_opl3();

		default:
			return nullptr;
	}
}


} // bstone
