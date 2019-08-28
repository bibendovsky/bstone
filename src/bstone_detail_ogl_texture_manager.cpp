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
#include "bstone_detail_ogl_state.h"
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
		const OglStatePtr ogl_state);

	~GenericOglTextureManager() override;


	OglStatePtr ogl_state_get() const noexcept override;


	RendererTexture2dPtr texture_2d_create(
		const RendererTexture2dCreateParam& param) override;

	void texture_2d_destroy(
		const RendererTexture2dPtr texture_2d) override;

	void texture_2d_set(
		const RendererTexture2dPtr texture_2d) override;

	bool texture_2d_set_current(
		const RendererTexture2dPtr texture_2d) override;

	RendererTexture2dPtr texture_2d_get_current() const noexcept override;

	void texture_2d_current_update_sampler_state(
		const RendererSamplerState& sampler_state) override;


private:
	const OglStatePtr ogl_state_;


	using Textures2d = UPtrResourceList<OglTexture2d, OglTexture2dFactory, Exception>;
	bool texture_2d_is_enabled_;
	OglTexture2dPtr texture_2d_current_;
	Textures2d textures_2d_;


	void initialize();

	void texture_2d_set();
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
	const OglStatePtr ogl_state)
	:
	ogl_state_{ogl_state},
	texture_2d_is_enabled_{},
	texture_2d_current_{},
	textures_2d_{}
{
	initialize();
}

GenericOglTextureManager::~GenericOglTextureManager() = default;

OglStatePtr GenericOglTextureManager::ogl_state_get() const noexcept
{
	return ogl_state_;
}

RendererTexture2dPtr GenericOglTextureManager::texture_2d_create(
	const RendererTexture2dCreateParam& param)
{
	return textures_2d_.add(this, param);
}

void GenericOglTextureManager::texture_2d_destroy(
	const RendererTexture2dPtr texture_2d)
{
	textures_2d_.remove(texture_2d);
}

void GenericOglTextureManager::texture_2d_set(
	const RendererTexture2dPtr texture_2d)
{
	if (!texture_2d_set_current(texture_2d))
	{
		return;
	}

	if (!texture_2d_current_)
	{
		return;
	}

	texture_2d_current_->bind();

	const auto& device_features = ogl_state_->get_device_features();

	if (!device_features.sampler_is_available_)
	{
		const auto sampler_manager = ogl_state_->sampler_get_manager();
		const auto& sampler_state = sampler_manager->sampler_current_get_state();

		texture_2d_current_->update_sampler_state(sampler_state);
	}
}

bool GenericOglTextureManager::texture_2d_set_current(
	const RendererTexture2dPtr texture_2d)
{
	if (texture_2d_current_ == texture_2d)
	{
		return false;
	}

	texture_2d_current_ = static_cast<OglTexture2dPtr>(texture_2d);

	return true;
}

RendererTexture2dPtr GenericOglTextureManager::texture_2d_get_current() const noexcept
{
	return texture_2d_current_;
}

void GenericOglTextureManager::texture_2d_current_update_sampler_state(
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
	if (!ogl_state_)
	{
		throw Exception{"Null OpenGL state."};
	}

	texture_2d_is_enabled_ = true;
	OglRendererUtils::texture_2d_enable(true);

	OglRendererUtils::texture_2d_unbind();
}

void GenericOglTextureManager::texture_2d_set()
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
	const OglStatePtr ogl_state)
{
	return std::make_unique<GenericOglTextureManager>(ogl_state);
}

//
// OglTextureManagerFactory
// ==========================================================================


} // detail
} // bstone
