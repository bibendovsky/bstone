/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include "bstone_detail_ren_3d_gl_sampler.h"

#include "bstone_exception.h"
#include "bstone_ren_3d_tests.h"
#include "bstone_unique_resource.h"

#include "bstone_detail_ren_3d_gl_api.h"
#include "bstone_detail_ren_3d_gl_context.h"
#include "bstone_detail_ren_3d_gl_error.h"
#include "bstone_detail_ren_3d_gl_utils.h"
#include "bstone_detail_ren_3d_gl_texture_mgr.h"


namespace bstone
{
namespace detail
{


// =========================================================================
// Ren3dGlSamplerImplCreateException
//

class Ren3dGlSamplerImplCreateException :
	public Exception
{
public:
	explicit Ren3dGlSamplerImplCreateException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_SMPLR_INIT] "} +  message}
	{
	}
}; // Ren3dGlSamplerImplCreateException

//
// Ren3dGlSamplerImplCreateException
// =========================================================================


// =========================================================================
// Ren3dGlSamplerImpl
//

class Ren3dGlSamplerImpl final :
	public Ren3dGlSampler
{
public:
	Ren3dGlSamplerImpl(
		Ren3dGlContextPtr context,
		const Ren3dCreateSamplerParam& param);

	Ren3dGlSamplerImpl(
		const Ren3dGlSampler& rhs) = delete;

	~Ren3dGlSamplerImpl() override;


	void set() override;

	void update(
		const Ren3dSamplerUpdateParam& param) override;

	const Ren3dSamplerState& get_state() const noexcept override;


private:
	static void sampler_deleter(
		const GLuint& gl_name) noexcept;

	using SamplerResource = bstone::UniqueResource<GLuint, sampler_deleter>;


	Ren3dGlContextPtr context_;

	Ren3dSamplerState state_;
	SamplerResource sampler_resource_;


	void set_mag_filter();

	void set_min_filter();

	void set_address_mode(
		const Ren3dTextureAxis texture_axis,
		const Ren3dAddressMode address_mode);

	void set_address_mode_u();

	void set_address_mode_v();

	void set_anisotropy();

	void set_initial_state();
}; // Ren3dGlSamplerImpl

//
// Ren3dGlSamplerImpl
// =========================================================================


// =========================================================================
// Ren3dGlSamplerImpl
//

Ren3dGlSamplerImpl::Ren3dGlSamplerImpl(
	Ren3dGlContextPtr context,
	const Ren3dCreateSamplerParam& param)
	:
	context_{context},
	state_{}
{
	if (!context_)
	{
		throw Ren3dGlSamplerImplCreateException{"Null context."};
	}

	const auto& device_features = context_->get_device_features();
	const auto& gl_device_features = context_->get_gl_device_features();

	state_ = param.state_;

	if (device_features.is_sampler_available_)
	{
		auto gl_name = GLuint{};

		if (gl_device_features.is_dsa_available_)
		{
			glCreateSamplers(1, &gl_name);
			Ren3dGlError::ensure_debug();
		}
		else
		{
			glGenSamplers(1, &gl_name);
			Ren3dGlError::ensure_debug();
		}

		sampler_resource_.reset(gl_name);

		if (!sampler_resource_)
		{
			throw Ren3dGlSamplerImplCreateException{"Failed to create an object."};
		}
	}

	set_initial_state();
}

Ren3dGlSamplerImpl::~Ren3dGlSamplerImpl() = default;

void Ren3dGlSamplerImpl::set()
{
	if (!sampler_resource_)
	{
		return;
	}

	glBindSampler(0, sampler_resource_.get());
	Ren3dGlError::ensure_debug();
}

void Ren3dGlSamplerImpl::update(
	const Ren3dSamplerUpdateParam& param)
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
	if (is_modified && sampler_resource_)
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
}

const Ren3dSamplerState& Ren3dGlSamplerImpl::get_state() const noexcept
{
	return state_;
}

void Ren3dGlSamplerImpl::sampler_deleter(
	const GLuint& gl_name) noexcept
{
	glDeleteSamplers(1, &gl_name);
	Ren3dGlError::ensure_debug();
}

void Ren3dGlSamplerImpl::set_mag_filter()
{
	const auto gl_mag_filter = Ren3dGlUtils::get_mag_filter(state_.mag_filter_);

	glSamplerParameteri(sampler_resource_.get(), GL_TEXTURE_MAG_FILTER, gl_mag_filter);
	Ren3dGlError::ensure_debug();
}

void Ren3dGlSamplerImpl::set_min_filter()
{
	const auto gl_min_filter = Ren3dGlUtils::get_min_filter(state_.min_filter_, state_.mipmap_mode_);

	glSamplerParameteri(sampler_resource_.get(), GL_TEXTURE_MIN_FILTER, gl_min_filter);
	Ren3dGlError::ensure_debug();
}

void Ren3dGlSamplerImpl::set_address_mode(
	const Ren3dTextureAxis texture_axis,
	const Ren3dAddressMode address_mode)
{
	const auto gl_wrap_axis = Ren3dGlUtils::get_texture_wrap_axis(texture_axis);
	const auto gl_address_mode = Ren3dGlUtils::get_address_mode(address_mode);

	glSamplerParameteri(sampler_resource_.get(), gl_wrap_axis, gl_address_mode);
	Ren3dGlError::ensure_debug();
}

void Ren3dGlSamplerImpl::set_address_mode_u()
{
	set_address_mode(Ren3dTextureAxis::u, state_.address_mode_u_);
}

void Ren3dGlSamplerImpl::set_address_mode_v()
{
	set_address_mode(Ren3dTextureAxis::v, state_.address_mode_v_);
}

void Ren3dGlSamplerImpl::set_anisotropy()
{
	Ren3dGlUtils::set_sampler_anisotropy(
		sampler_resource_.get(),
		context_->get_device_features(),
		state_.anisotropy_
	);
}

void Ren3dGlSamplerImpl::set_initial_state()
{
	if (!sampler_resource_)
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
// Ren3dGlSamplerImpl
// =========================================================================


// =========================================================================
// Ren3dGlSamplerFactory
//

Ren3dGlSamplerUPtr Ren3dGlSamplerFactory::create(
	Ren3dGlContextPtr context,
	const Ren3dCreateSamplerParam& param)
{
	return std::make_unique<Ren3dGlSamplerImpl>(context, param);
}

//
// Ren3dGlSamplerFactory
// =========================================================================


} // detail
} // bstone
