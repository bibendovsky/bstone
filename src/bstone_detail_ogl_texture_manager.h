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
// OpenGL texture object manager (implementation interface).
//


#ifndef BSTONE_DETAIL_OGL_TEXTURE_MANAGER_INCLUDED
#define BSTONE_DETAIL_OGL_TEXTURE_MANAGER_INCLUDED


#include <memory>


namespace bstone
{


struct RendererSamplerState;

class OglContext;
using OglContextPtr = OglContext*;

class RendererTexture2d;
using RendererTexture2dPtr = RendererTexture2d*;

struct RendererTexture2dCreateParam;


namespace detail
{


class OglContext;
using OglContextPtr = OglContext*;


// ==========================================================================
// OglTextureManager
//

class OglTextureManager
{
protected:
	OglTextureManager();


public:
	virtual ~OglTextureManager();


	virtual OglContextPtr get_ogl_context() const noexcept = 0;


	virtual RendererTexture2dPtr create(
		const RendererTexture2dCreateParam& param) = 0;

	virtual void destroy(
		const RendererTexture2dPtr texture_2d) = 0;

	virtual void set(
		const RendererTexture2dPtr texture_2d) = 0;

	virtual bool set_current(
		const RendererTexture2dPtr texture_2d) = 0;

	virtual void update_current_sampler_state(
		const RendererSamplerState& sampler_state) = 0;
}; // OglTextureManager

using OglTextureManagerPtr = OglTextureManager*;
using OglTextureManagerUPtr = std::unique_ptr<OglTextureManager>;

//
// OglTextureManager
// ==========================================================================


// ==========================================================================
// OglTextureManagerFactory
//

struct OglTextureManagerFactory final
{
	static OglTextureManagerUPtr create(
		const OglContextPtr ogl_context);
}; // OglTextureManagerFactory

//
// OglTextureManagerFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_TEXTURE_MANAGER_INCLUDED
