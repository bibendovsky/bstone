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
// OpenGL RAII wrapper.
//


#ifndef BSTONE_OGL_RAII_INCLUDED
#define BSTONE_OGL_RAII_INCLUDED


#include "bstone_detail_ogl_renderer_utils.h"


namespace bstone
{


enum class OglRaiiDeleterId
{
	buffer,
	shader,
	program,
}; // OglRaiiDeleterId


template<OglRaiiDeleterId TDeleterId>
struct OglRaiiDeleter
{
};

template<>
struct OglRaiiDeleter<OglRaiiDeleterId::buffer>
{
	void operator()(
		const GLuint ogl_name)
	{
		::glDeleteBuffers(1, &ogl_name);
	}
};

template<>
struct OglRaiiDeleter<OglRaiiDeleterId::shader>
{
	void operator()(
		const GLuint ogl_name)
	{
		::glDeleteShader(ogl_name);
	}
};

template<>
struct OglRaiiDeleter<OglRaiiDeleterId::program>
{
	void operator()(
		const GLuint ogl_name)
	{
		::glDeleteProgram(ogl_name);
	}
};


template<OglRaiiDeleterId TDeleterId>
class OglRaii
{
public:
	OglRaii() noexcept
		:
		ogl_name_{}
	{
	}

	OglRaii(
		const GLuint ogl_name) noexcept
		:
		ogl_name_{ogl_name}
	{
	}

	OglRaii(
		const OglRaii& rhs) = delete;

	OglRaii(
		OglRaii&& rhs) noexcept
		:
		ogl_name_{rhs.ogl_name_}
	{
		rhs.ogl_name_ = 0;
	}

	OglRaii& operator=(
		const OglRaii& rhs) = delete;

	OglRaii& operator=(
		OglRaii&& rhs) noexcept
	{
		{
			OglRaii{std::move(*this)};
		}

		ogl_name_ = rhs.ogl_name_;
		rhs.ogl_name_ = 0;

		return *this;
	}

	~OglRaii()
	{
		if (ogl_name_ == 0)
		{
			return;
		}

		OglRaiiDeleter<TDeleterId>{}(ogl_name_);
		assert(!detail::OglRendererUtils::was_errors());

		ogl_name_ = GL_NONE;
	}

	operator GLuint() const noexcept
	{
		return ogl_name_;
	}

	GLuint get() const noexcept
	{
		return ogl_name_;
	}


private:
	GLuint ogl_name_;
}; // OglRaii


using OglBufferRaii = OglRaii<OglRaiiDeleterId::buffer>;
using OglShaderRaii = OglRaii<OglRaiiDeleterId::shader>;
using OglProgramRaii = OglRaii<OglRaiiDeleterId::program>;


template<OglRaiiDeleterId TDeleterId>
bool operator==(
	const OglRaii<TDeleterId>& lhs,
	const OglRaii<TDeleterId>& rhs)
{
	return static_cast<GLuint>(lhs) == static_cast<GLuint>(rhs);
}

template<OglRaiiDeleterId TDeleterId>
bool operator!=(
	const OglRaii<TDeleterId>& lhs,
	const OglRaii<TDeleterId>& rhs)
{
	return !(lhs == rhs);
}


} // bstone


#endif // !BSTONE_OGL_RAII_INCLUDED
