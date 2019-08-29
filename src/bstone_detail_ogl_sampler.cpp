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
// OpenGL sampler (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_sampler.h"

#include "bstone_exception.h"
#include "bstone_unique_resource.h"

#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_context.h"
#include "bstone_detail_ogl_texture_manager.h"


namespace bstone
{
namespace detail
{


// =========================================================================
// OglSampler
//

OglSampler::OglSampler() = default;

OglSampler::~OglSampler() = default;

//
// OglSampler
// =========================================================================


// =========================================================================
// GenericOglSampler
//

class GenericOglSampler final :
	public OglSampler
{
public:
	GenericOglSampler(
		OglContextPtr ogl_context,
		const RendererSamplerCreateParam& param);

	GenericOglSampler(
		const OglSampler& rhs) = delete;

	~GenericOglSampler() override;


	void bind() override;

	void update(
		const RendererSamplerUpdateParam& param) override;

	const RendererSamplerState& get_state() const noexcept override;


private:
	static void sampler_deleter(
		const GLuint& resource) noexcept;

	using SamplerResource = bstone::UniqueResource<GLuint, sampler_deleter>;


	OglContextPtr ogl_context_;

	RendererSamplerState state_;
	SamplerResource ogl_resource_;


	void initialize(
		const RendererSamplerCreateParam& param);

	void set_mag_filter();

	void set_min_filter();

	void set_address_mode(
		const RendererTextureAxis texture_axis,
		const RendererAddressMode address_mode);

	void set_address_mode_u();

	void set_address_mode_v();

	void set_anisotropy();

	void set_initial_state();
}; // GenericOglSampler

using GenericOglSamplerPtr = GenericOglSampler*;
using GenericOglSamplerUPtr = std::unique_ptr<GenericOglSampler>;

//
// GenericOglSampler
// =========================================================================


// =========================================================================
// GenericOglSampler
//

GenericOglSampler::GenericOglSampler(
	OglContextPtr ogl_context,
	const RendererSamplerCreateParam& param)
	:
	ogl_context_{ogl_context},
	state_{}
{
	if (!ogl_context_)
	{
		throw Exception{"Null OpenGL state."};
	}

	initialize(param);
}

GenericOglSampler::~GenericOglSampler() = default;

void GenericOglSampler::bind()
{
	if (!ogl_resource_)
	{
		return;
	}

	::glBindSampler(0, ogl_resource_);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglSampler::update(
	const RendererSamplerUpdateParam& param)
{
	auto is_modified = false;

	// Magnification filter.
	//
	auto is_mag_filter_modified = false;

	if (state_.mag_filter_ != param.state_.mag_filter_)
	{
		is_modified = true;
		is_mag_filter_modified = true;

		state_.mag_filter_ = param.state_.mag_filter_;
	}

	// Minification filter.
	//
	auto is_min_filter_modified = false;

	if (state_.min_filter_ != param.state_.min_filter_ ||
		state_.mipmap_mode_ != param.state_.mipmap_mode_)
	{
		is_modified = true;
		is_min_filter_modified = true;

		state_.min_filter_ = param.state_.min_filter_;
		state_.mipmap_mode_ = param.state_.mipmap_mode_;
	}

	// U-axis address mode.
	//
	auto is_address_mode_u = false;

	if (state_.address_mode_u_ != param.state_.address_mode_u_)
	{
		is_modified = true;
		is_address_mode_u = true;

		state_.address_mode_u_ = param.state_.address_mode_u_;
	}

	// V-axis address mode.
	//
	auto is_address_mode_v = false;

	if (state_.address_mode_v_ != param.state_.address_mode_v_)
	{
		is_modified = true;
		is_address_mode_v = true;

		state_.address_mode_v_ = param.state_.address_mode_v_;
	}

	// Anisotropy.
	//
	auto is_anisotropy = false;

	if (state_.anisotropy_ != param.state_.anisotropy_)
	{
		is_modified = true;
		is_anisotropy = true;

		state_.anisotropy_ = param.state_.anisotropy_;
	}


	// Modify.
	//
	if (is_modified)
	{
		if (ogl_resource_)
		{
			if (is_mag_filter_modified)
			{
				set_mag_filter();
			}

			if (is_min_filter_modified)
			{
				set_min_filter();
			}

			if (is_address_mode_u)
			{
				set_address_mode_u();
			}

			if (is_address_mode_v)
			{
				set_address_mode_v();
			}

			if (is_anisotropy)
			{
				set_anisotropy();
			}
		}
		else
		{
			const auto texture_manager = ogl_context_->texture_get_manager();

			texture_manager->texture_2d_current_update_sampler_state(state_);
		}
	}
}

const RendererSamplerState& GenericOglSampler::get_state() const noexcept
{
	return state_;
}

void GenericOglSampler::sampler_deleter(
	const GLuint& resource) noexcept
{
	::glDeleteSamplers(1, &resource);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglSampler::initialize(
	const RendererSamplerCreateParam& param)
{
	const auto& device_features = ogl_context_->get_device_features();
	const auto& ogl_device_features = ogl_context_->get_ogl_device_features();

	state_ = param.state_;

	if (device_features.sampler_is_available_)
	{
		auto ogl_name = GLuint{};

		if (ogl_device_features.dsa_is_available_)
		{
			::glCreateSamplers(1, &ogl_name);
			assert(!OglRendererUtils::was_errors());
		}
		else
		{
			::glGenSamplers(1, &ogl_name);
			assert(!OglRendererUtils::was_errors());
		}

		if (ogl_name == 0)
		{
			throw Exception{"Failed to create OpenGL sampler object."};
		}

		ogl_resource_.reset(ogl_name);
	}

	set_initial_state();
}

void GenericOglSampler::set_mag_filter()
{
	const auto ogl_mag_filter = OglRendererUtils::filter_get_mag(state_.mag_filter_);

	::glSamplerParameteri(ogl_resource_, GL_TEXTURE_MAG_FILTER, ogl_mag_filter);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglSampler::set_min_filter()
{
	const auto ogl_min_filter = OglRendererUtils::filter_get_min(state_.min_filter_, state_.mipmap_mode_);

	::glSamplerParameteri(ogl_resource_, GL_TEXTURE_MIN_FILTER, ogl_min_filter);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglSampler::set_address_mode(
	const RendererTextureAxis texture_axis,
	const RendererAddressMode address_mode)
{
	const auto ogl_wrap_axis = OglRendererUtils::texture_wrap_get_axis(texture_axis);
	const auto ogl_address_mode = OglRendererUtils::address_mode_get(address_mode);

	::glSamplerParameteri(ogl_resource_, ogl_wrap_axis, ogl_address_mode);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglSampler::set_address_mode_u()
{
	set_address_mode(RendererTextureAxis::u, state_.address_mode_u_);
}

void GenericOglSampler::set_address_mode_v()
{
	set_address_mode(RendererTextureAxis::v, state_.address_mode_v_);
}

void GenericOglSampler::set_anisotropy()
{
	OglRendererUtils::sampler_set_anisotropy(
		ogl_resource_,
		ogl_context_->get_device_features(),
		state_.anisotropy_
	);
}

void GenericOglSampler::set_initial_state()
{
	if (!ogl_resource_)
	{
		return;
	}

	set_mag_filter();
	set_min_filter();
	set_address_mode_u();
	set_address_mode_v();
	set_anisotropy();
}

//
// GenericOglSampler
// =========================================================================


// =========================================================================
// OglSamplerFactory
//

OglSamplerUPtr OglSamplerFactory::create(
	OglContextPtr ogl_context,
	const RendererSamplerCreateParam& param)
{
	return std::make_unique<GenericOglSampler>(ogl_context, param);
}

//
// OglSamplerFactory
// =========================================================================


} // detail
} // bstone
