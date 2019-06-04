/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// Types of RGB color model.
//


#ifndef BSTONE_RGB_COLOR_MODEL_INCLUDED
#define BSTONE_RGB_COLOR_MODEL_INCLUDED


#include "glm/glm.hpp"


namespace bstone
{


using R8g8b8 = glm::u8vec3;
using R8g8b8Ptr = R8g8b8*;
using R8g8b8CPtr = const R8g8b8*;

using R8g8b8a8 = glm::u8vec4;
using R8g8b8a8Ptr = R8g8b8a8*;
using R8g8b8a8CPtr = const R8g8b8a8*;


} // bstone


#endif // !BSTONE_RGB_COLOR_MODEL_INCLUDED
