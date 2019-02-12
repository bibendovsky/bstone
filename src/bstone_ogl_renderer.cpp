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
// OpenGL renderer.
//
// !!! Internal usage only !!!
//



#include "bstone_precompiled.h"
#include "bstone_ogl_renderer.h"


namespace bstone
{


const std::string& OglRenderer::get_error_message() const
{
	throw "Not implemented.";
}

RendererKind OglRenderer::get_kind() const
{
	throw "Not implemented.";
}

const std::string& OglRenderer::get_name() const
{
	throw "Not implemented.";
}

const std::string& OglRenderer::get_description() const
{
	throw "Not implemented.";
}

bool OglRenderer::probe(
	const RendererPath renderer_path,
	RendererPath& selected_renderer_path)
{
	throw "Not implemented.";
}

bool OglRenderer::is_initialized() const
{
	throw "Not implemented.";
}

bool OglRenderer::initialize(
	const RendererPath renderer_path)
{
	throw "Not implemented.";
}

void OglRenderer::uninitialize()
{
	throw "Not implemented.";
}

RendererPath OglRenderer::get_path() const
{
	throw "Not implemented.";
}


} // bstone
