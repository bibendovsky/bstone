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
// OpenGL sampler object manager (implementation interface).
//


#ifndef BSTONE_DETAIL_OGL_SAMPLER_MANAGER_INCLUDED
#define BSTONE_DETAIL_OGL_SAMPLER_MANAGER_INCLUDED


#include <memory>

#include "bstone_renderer.h"


namespace bstone
{
namespace detail
{


class OglContext;
using OglContextPtr = OglContext*;


// ==========================================================================
// OglSamplerManager
//

class OglSamplerManager
{
protected:
	OglSamplerManager();


public:
	virtual ~OglSamplerManager();


	virtual RendererSamplerPtr create(
		const RendererSamplerCreateParam& param) = 0;

	virtual void destroy(
		const RendererSamplerPtr sampler) = 0;

	virtual void set(
		const RendererSamplerPtr sampler) = 0;

	virtual const RendererSamplerState& get_current_state() const noexcept = 0;
}; // OglSamplerManager

using OglSamplerManagerPtr = OglSamplerManager*;
using OglSamplerManagerUPtr = std::unique_ptr<OglSamplerManager>;

//
// OglSamplerManager
// ==========================================================================


// ==========================================================================
// OglSamplerManagerFactory
//

struct OglSamplerManagerFactory final
{
	static OglSamplerManagerUPtr create(
		const OglContextPtr ogl_context);
}; // OglSamplerManagerFactory

//
// OglSamplerManagerFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_SAMPLER_MANAGER_INCLUDED
