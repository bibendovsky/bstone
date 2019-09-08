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
// OpenGL texture object manager (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_texture_manager.h"

#include "bstone_exception.h"
#include "bstone_renderer.h"
#include "bstone_uptr_resource_list.h"

#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_sampler_manager.h"
#include "bstone_detail_ogl_context.h"
#include "bstone_detail_ogl_texture_2d.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglTextureManager
//

OglTextureManager::OglTextureManager() = default;

OglTextureManager::~OglTextureManager() = default;

//
// OglTextureManager
// ==========================================================================


// ==========================================================================
// GenericOglTextureManager
//

class GenericOglTextureManager :
	public OglTextureManager
{
public:
	GenericOglTextureManager(
		const OglContextPtr ogl_context);

	~GenericOglTextureManager() override;


	OglContextPtr get_ogl_context() const noexcept override;


	RendererTexture2dPtr create(
		const RendererTexture2dCreateParam& param) override;

	void destroy(
		const RendererTexture2dPtr texture_2d) override;

	void set(
		const RendererTexture2dPtr texture_2d) override;

	bool set_current(
		const RendererTexture2dPtr texture_2d) override;

	void update_current_sampler_state(
		const RendererSamplerState& sampler_state) override;


private:
	const OglContextPtr ogl_context_;


	using Textures2d = UPtrResourceList<OglTexture2d, OglTexture2dFactory, Exception>;
	OglTexture2dPtr texture_2d_current_;
	Textures2d textures_2d_;


	void initialize();

	void set();
}; // GenericOglTextureManager

using GenericOglTextureManagerPtr = GenericOglTextureManager*;
using GenericOglTextureManagerUPtr = std::unique_ptr<GenericOglTextureManager>;

//
// GenericOglTextureManager
// ==========================================================================


// ==========================================================================
// GenericOglTextureManager
//

GenericOglTextureManager::GenericOglTextureManager(
	const OglContextPtr ogl_context)
	:
	ogl_context_{ogl_context},
	texture_2d_current_{},
	textures_2d_{}
{
	initialize();
}

GenericOglTextureManager::~GenericOglTextureManager() = default;

OglContextPtr GenericOglTextureManager::get_ogl_context() const noexcept
{
	return ogl_context_;
}

RendererTexture2dPtr GenericOglTextureManager::create(
	const RendererTexture2dCreateParam& param)
{
	return textures_2d_.add(this, param);
}

void GenericOglTextureManager::destroy(
	const RendererTexture2dPtr texture_2d)
{
	textures_2d_.remove(texture_2d);
}

void GenericOglTextureManager::set(
	const RendererTexture2dPtr texture_2d)
{
	if (!set_current(texture_2d))
	{
		return;
	}

	if (!texture_2d_current_)
	{
		return;
	}

	texture_2d_current_->bind();

	const auto& device_features = ogl_context_->get_device_features();

	if (!device_features.sampler_is_available_)
	{
		const auto sampler_manager = ogl_context_->sampler_get_manager();
		const auto& sampler_state = sampler_manager->get_current_state();

		texture_2d_current_->update_sampler_state(sampler_state);
	}
}

bool GenericOglTextureManager::set_current(
	const RendererTexture2dPtr texture_2d)
{
	if (texture_2d_current_ == texture_2d)
	{
		return false;
	}

	texture_2d_current_ = static_cast<OglTexture2dPtr>(texture_2d);

	return true;
}

void GenericOglTextureManager::update_current_sampler_state(
	const RendererSamplerState& sampler_state)
{
	if (!texture_2d_current_)
	{
		return;
	}

	texture_2d_current_->update_sampler_state(sampler_state);
}

void GenericOglTextureManager::initialize()
{
	if (!ogl_context_)
	{
		throw Exception{"Null OpenGL state."};
	}

	OglRendererUtils::texture_2d_unbind();
}

void GenericOglTextureManager::set()
{
	if (texture_2d_current_)
	{
		texture_2d_current_->bind();
	}
	else
	{
		OglRendererUtils::texture_2d_unbind();
	}
}

//
// GenericOglTextureManager
// ==========================================================================


// ==========================================================================
// OglTextureManagerFactory
//

OglTextureManagerUPtr OglTextureManagerFactory::create(
	const OglContextPtr ogl_context)
{
	return std::make_unique<GenericOglTextureManager>(ogl_context);
}

//
// OglTextureManagerFactory
// ==========================================================================


} // detail
} // bstone
