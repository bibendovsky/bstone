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
// OpenGL sampler object manager (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_sampler_manager.h"

#include "bstone_exception.h"
#include "bstone_uptr_resource_list.h"

#include "bstone_detail_ogl_sampler.h"
#include "bstone_detail_ogl_context.h"
#include "bstone_detail_ogl_texture_manager.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglSamplerManager
//

OglSamplerManager::OglSamplerManager() = default;

OglSamplerManager::~OglSamplerManager() = default;

//
// OglSamplerManager
// ==========================================================================


// ==========================================================================
// GenericOglSamplerManager
//

class GenericOglSamplerManager :
	public OglSamplerManager
{
public:
	GenericOglSamplerManager(
		const OglContextPtr ogl_context);

	~GenericOglSamplerManager() override;


	RendererSamplerPtr sampler_create(
		const RendererSamplerCreateParam& param) override;

	void sampler_destroy(
		const RendererSamplerPtr sampler) override;

	void sampler_set(
		const RendererSamplerPtr sampler) override;

	const RendererSamplerState& sampler_current_get_state() const noexcept override;


private:
	const OglContextPtr ogl_context_;

	using Samplers = UPtrResourceList<OglSampler, OglSamplerFactory, Exception>;
	OglSamplerPtr sampler_current_;
	OglSamplerUPtr sampler_default_;
	Samplers samplers_;


	void initialize_default_sampler();

	void initialize();

	void sampler_set();
}; // GenericOglSamplerManager

using GenericOglSamplerManagerPtr = GenericOglSamplerManager*;
using GenericOglSamplerManagerUPtr = std::unique_ptr<GenericOglSamplerManager>;

//
// GenericOglSamplerManager
// ==========================================================================


// ==========================================================================
// GenericOglSamplerManager
//

GenericOglSamplerManager::GenericOglSamplerManager(
	const OglContextPtr ogl_context)
	:
	ogl_context_{ogl_context},
	sampler_current_{},
	sampler_default_{},
	samplers_{}
{
	initialize();
}

GenericOglSamplerManager::~GenericOglSamplerManager()
{
}

RendererSamplerPtr GenericOglSamplerManager::sampler_create(
	const RendererSamplerCreateParam& param)
{
	return samplers_.add(ogl_context_, param);
}

void GenericOglSamplerManager::sampler_destroy(
	const RendererSamplerPtr sampler)
{
	if (sampler_current_ == sampler)
	{
		const auto& device_features = ogl_context_->get_device_features();

		if (device_features.sampler_is_available_)
		{
			sampler_current_ = nullptr;
		}
		else
		{
			sampler_current_ = sampler_default_.get();
		}
	}

	samplers_.remove(sampler);
}

void GenericOglSamplerManager::sampler_set(
	const RendererSamplerPtr sampler)
{
	if (!sampler)
	{
		throw Exception{"Null sampler."};
	}

	if (sampler_current_ == sampler)
	{
		return;
	}

	sampler_current_ = static_cast<OglSamplerPtr>(sampler);
	sampler_set();
}

const RendererSamplerState& GenericOglSamplerManager::sampler_current_get_state() const noexcept
{
	return sampler_current_->get_state();
}

void GenericOglSamplerManager::initialize_default_sampler()
{
	auto param = RendererSamplerCreateParam{};

	sampler_default_ = OglSamplerFactory::create(ogl_context_, param);
}

void GenericOglSamplerManager::initialize()
{
	if (!ogl_context_)
	{
		throw Exception{"Null OpenGL state."};
	}

	initialize_default_sampler();

	sampler_current_ = sampler_default_.get();
}

void GenericOglSamplerManager::sampler_set()
{
	const auto& device_features = ogl_context_->get_device_features();

	if (device_features.sampler_is_available_)
	{
		sampler_current_->bind();
	}
	else
	{
		const auto texture_manager = ogl_context_->texture_get_manager();

		texture_manager->texture_2d_current_update_sampler_state(sampler_current_get_state());
	}
}

//
// GenericOglSamplerManager
// ==========================================================================


// ==========================================================================
// OglSamplerManagerFactory
//

OglSamplerManagerUPtr OglSamplerManagerFactory::create(
	const OglContextPtr ogl_context)
{
	return std::make_unique<GenericOglSamplerManager>(ogl_context);
}

//
// OglSamplerManagerFactory
// ==========================================================================


} // detail
} // bstone
