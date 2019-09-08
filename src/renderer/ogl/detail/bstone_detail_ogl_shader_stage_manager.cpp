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
// OpenGL shader stage manager (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_shader_stage_manager.h"

#include "bstone_exception.h"
#include "bstone_uptr_resource_list.h"

#include "bstone_detail_ogl_context.h"
#include "bstone_detail_ogl_shader_stage.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglShaderStageManager
//

OglShaderStageManager::OglShaderStageManager() = default;

OglShaderStageManager::~OglShaderStageManager() = default;

//
// OglShaderStageManager
// ==========================================================================


// ==========================================================================
// GenericOglShaderStageManager
//

class GenericOglShaderStageManager :
	public OglShaderStageManager
{
public:
	GenericOglShaderStageManager(
		const OglContextPtr ogl_context);

	~GenericOglShaderStageManager() override;


	OglContextPtr get_ogl_context() const noexcept override;


	RendererShaderStagePtr create(
		const RendererShaderStageCreateParam& param) override;

	void destroy(
		const RendererShaderStagePtr shader_stage) override;

	bool set_current(
		const RendererShaderStagePtr shader_stage) override;


private:
	const OglContextPtr ogl_context_;

	OglShaderStagePtr shader_stage_current_;

	using ShaderStages = UPtrResourceList<OglShaderStage, OglShaderStageFactory, Exception>;
	ShaderStages shader_stages_;


	void initialize();

	void shader_stage_set();
}; // GenericOglShaderStageManager

using GenericOglShaderStageManagerPtr = GenericOglShaderStageManager*;
using GenericOglShaderStageManagerUPtr = std::unique_ptr<GenericOglShaderStageManager>;

//
// GenericOglShaderStageManager
// ==========================================================================


// ==========================================================================
// GenericOglShaderStageManager
//

GenericOglShaderStageManager::GenericOglShaderStageManager(
	const OglContextPtr ogl_context)
	:
	ogl_context_{ogl_context},
	shader_stage_current_{},
	shader_stages_{}
{
	initialize();
}

GenericOglShaderStageManager::~GenericOglShaderStageManager() = default;

OglContextPtr GenericOglShaderStageManager::get_ogl_context() const noexcept
{
	return ogl_context_;
}

RendererShaderStagePtr GenericOglShaderStageManager::create(
	const RendererShaderStageCreateParam& param)
{
	return shader_stages_.add(this, param);
}

void GenericOglShaderStageManager::destroy(
	const RendererShaderStagePtr shader_stage)
{
	shader_stages_.remove(shader_stage);
}

bool GenericOglShaderStageManager::set_current(
	const RendererShaderStagePtr shader_stage)
{
	if (shader_stage_current_ == shader_stage)
	{
		return false;
	}

	shader_stage_current_ = static_cast<OglShaderStagePtr>(shader_stage);

	return true;
}

void GenericOglShaderStageManager::initialize()
{
}

void GenericOglShaderStageManager::shader_stage_set()
{
}

//
// GenericOglShaderStageManager
// ==========================================================================


// ==========================================================================
// OglShaderStageManagerFactory
//

OglShaderStageManagerUPtr OglShaderStageManagerFactory::create(
	const OglContextPtr ogl_context)
{
	return std::make_unique<GenericOglShaderStageManager>(ogl_context);
}

//
// OglShaderStageManagerFactory
// ==========================================================================


} // detail
} // bstone
