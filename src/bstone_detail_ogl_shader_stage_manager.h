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
// OpenGL shader stage manager (implementation interface).
//


#ifndef BSTONE_DETAIL_OGL_SHADER_STAGE_MANAGER_INCLUDED
#define BSTONE_DETAIL_OGL_SHADER_STAGE_MANAGER_INCLUDED


#include <memory>

#include "bstone_renderer.h"


namespace bstone
{
namespace detail
{


class OglContext;
using OglContextPtr = OglContext*;


// ==========================================================================
// OglShaderStageManager
//

class OglShaderStageManager
{
protected:
	OglShaderStageManager();


public:
	virtual ~OglShaderStageManager();


	virtual OglContextPtr ogl_context_get() const noexcept = 0;


	virtual RendererShaderStagePtr shader_stage_create(
		const RendererShaderStage::CreateParam& param) = 0;

	virtual void shader_stage_destroy(
		const RendererShaderStagePtr shader_stage) = 0;

	virtual bool shader_stage_set_current(
		const RendererShaderStagePtr shader_stage) = 0;
}; // OglShaderStageManager

using OglShaderStageManagerPtr = OglShaderStageManager*;
using OglShaderStageManagerUPtr = std::unique_ptr<OglShaderStageManager>;

//
// OglShaderStageManager
// ==========================================================================


// ==========================================================================
// OglShaderStageManagerFactory
//

struct OglShaderStageManagerFactory final
{
	static OglShaderStageManagerUPtr create(
		const OglContextPtr ogl_context);
}; // OglShaderStageManagerFactory

//
// OglShaderStageManager
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_SHADER_STAGE_MANAGER_INCLUDED
