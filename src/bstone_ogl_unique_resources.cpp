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
// OpenGL handle wrappers.
//


#include "bstone_precompiled.h"
#include "bstone_ogl_unique_resources.h"
#include "bstone_detail_ogl_renderer_utils.h"


namespace bstone
{


void ogl_shader_unique_resource_deleter(
	const GLuint& ogl_name) noexcept
{
	::glDeleteShader(ogl_name);
	assert(!detail::OglRendererUtils::was_errors());
}

void ogl_program_unique_resource_deleter(
	const GLuint& ogl_name) noexcept
{
	::glDeleteProgram(ogl_name);
	assert(!detail::OglRendererUtils::was_errors());
}

void ogl_texture_unique_resource_deleter(
	const GLuint& ogl_name) noexcept
{
	::glDeleteTextures(1, &ogl_name);
	assert(!detail::OglRendererUtils::was_errors());
}

void ogl_vertex_array_resource_deleter(
	const GLuint& resource) noexcept
{
	::glDeleteVertexArrays(1, &resource);
	assert(!detail::OglRendererUtils::was_errors());
}


} // bstone
