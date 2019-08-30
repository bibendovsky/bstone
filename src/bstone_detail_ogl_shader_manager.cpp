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
// OpenGL shader object manager (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_shader_manager.h"

#include "bstone_exception.h"
#include "bstone_uptr_resource_list.h"

#include "bstone_detail_ogl_shader.h"
#include "bstone_detail_ogl_context.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglShaderManager
//

OglShaderManager::OglShaderManager() = default;

OglShaderManager::~OglShaderManager() = default;

//
// OglShaderManager
// ==========================================================================


// ==========================================================================
// GenericOglShaderManager
//

class GenericOglShaderManager :
	public OglShaderManager
{
public:
	GenericOglShaderManager(
		const OglContextPtr ogl_context);

	~GenericOglShaderManager() override;


	RendererShaderPtr shader_create(
		const RendererShader::CreateParam& param) override;

	void shader_destroy(
		const RendererShaderPtr shader) override;


private:
	const OglContextPtr ogl_context_;

	using Shaders = UPtrResourceList<OglShader, OglShaderFactory, Exception>;
	Shaders shaders_;


	void initialize();
}; // GenericOglShaderManager

using GenericOglShaderManagerPtr = GenericOglShaderManager*;
using GenericOglShaderManagerUPtr = std::unique_ptr<GenericOglShaderManager>;

//
// GenericOglShaderManager
// ==========================================================================


// ==========================================================================
// GenericOglShaderManager
//

GenericOglShaderManager::GenericOglShaderManager(
	const OglContextPtr ogl_context)
	:
	ogl_context_{ogl_context},
	shaders_{}
{
	initialize();
}

GenericOglShaderManager::~GenericOglShaderManager() = default;

RendererShaderPtr GenericOglShaderManager::shader_create(
	const RendererShader::CreateParam& param)
{
	return shaders_.add(this, param);
}

void GenericOglShaderManager::shader_destroy(
	const RendererShaderPtr shader)
{
	shaders_.remove(shader);
}

void GenericOglShaderManager::initialize()
{
	if (!ogl_context_)
	{
		throw Exception{"Null OpenGL context."};
	}
}

//
// GenericOglShaderManager
// ==========================================================================


// ==========================================================================
// OglShaderManagerFactory
//

OglShaderManagerUPtr OglShaderManagerFactory::create(
	const OglContextPtr ogl_context)
{
	return std::make_unique<GenericOglShaderManager>(ogl_context);
}

//
// OglShaderManagerFactory
// ==========================================================================


} // detail
} // bstone
