/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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
// Ren3dGlSamplerException
//

class Ren3dGlSamplerException :
	public Exception
{
public:
	explicit Ren3dGlSamplerException(
		const char* message) noexcept
		:
		Exception{"REN_3D_GL_SAMPLER", message}
	{
	}
}; // Ren3dGlSamplerException

//
// Ren3dGlSamplerException
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
	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	static void sampler_deleter(
		GLuint gl_name) noexcept;

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
try
	:
	context_{context},
	state_{}
{
	if (!context_)
	{
		fail("Null context.");
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
			fail("Failed to create an object.");
		}
	}

	set_initial_state();
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dGlSamplerImpl::~Ren3dGlSamplerImpl() = default;

void Ren3dGlSamplerImpl::set()
try
{
	if (!sampler_resource_)
	{
		return;
	}

	glBindSampler(0, sampler_resource_.get());
	Ren3dGlError::ensure_debug();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlSamplerImpl::update(
	const Ren3dSamplerUpdateParam& param)
try
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
catch (...)
{
	fail_nested(__func__);
}

const Ren3dSamplerState& Ren3dGlSamplerImpl::get_state() const noexcept
{
	return state_;
}

[[noreturn]]
void Ren3dGlSamplerImpl::fail(
	const char* message)
{
	throw Ren3dGlSamplerException{message};
}

[[noreturn]]
void Ren3dGlSamplerImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(Ren3dGlSamplerException{message});
}

void Ren3dGlSamplerImpl::sampler_deleter(
	GLuint gl_name) noexcept
{
	glDeleteSamplers(1, &gl_name);
	Ren3dGlError::ensure_assert();
}

void Ren3dGlSamplerImpl::set_mag_filter()
try
{
	const auto gl_mag_filter = Ren3dGlUtils::get_mag_filter(state_.mag_filter_);

	glSamplerParameteri(sampler_resource_.get(), GL_TEXTURE_MAG_FILTER, gl_mag_filter);
	Ren3dGlError::ensure_debug();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlSamplerImpl::set_min_filter()
try
{
	const auto gl_min_filter = Ren3dGlUtils::get_min_filter(state_.min_filter_, state_.mipmap_mode_);

	glSamplerParameteri(sampler_resource_.get(), GL_TEXTURE_MIN_FILTER, gl_min_filter);
	Ren3dGlError::ensure_debug();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlSamplerImpl::set_address_mode(
	const Ren3dTextureAxis texture_axis,
	const Ren3dAddressMode address_mode)
try
{
	const auto gl_wrap_axis = Ren3dGlUtils::get_texture_wrap_axis(texture_axis);
	const auto gl_address_mode = Ren3dGlUtils::get_address_mode(address_mode);

	glSamplerParameteri(sampler_resource_.get(), gl_wrap_axis, gl_address_mode);
	Ren3dGlError::ensure_debug();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlSamplerImpl::set_address_mode_u()
try
{
	set_address_mode(Ren3dTextureAxis::u, state_.address_mode_u_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlSamplerImpl::set_address_mode_v()
try
{
	set_address_mode(Ren3dTextureAxis::v, state_.address_mode_v_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlSamplerImpl::set_anisotropy()
try
{
	Ren3dGlUtils::set_sampler_anisotropy(
		sampler_resource_.get(),
		context_->get_device_features(),
		state_.anisotropy_
	);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlSamplerImpl::set_initial_state()
try
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
catch (...)
{
	fail_nested(__func__);
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
