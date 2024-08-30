/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Sampler

#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_unique_resource.h"

#include "bstone_r3r_limits.h"
#include "bstone_r3r_tests.h"

#include "bstone_gl_r3r_api.h"
#include "bstone_gl_r3r_context.h"
#include "bstone_gl_r3r_error.h"
#include "bstone_gl_r3r_sampler.h"
#include "bstone_gl_r3r_utils.h"

namespace bstone {

GlR3rSampler::GlR3rSampler() = default;

GlR3rSampler::~GlR3rSampler() = default;

// =========================================================================

class GlR3rSamplerImpl final : public GlR3rSampler
{
public:
	GlR3rSamplerImpl(GlR3rContext& context, const R3rSamplerInitParam& param);
	GlR3rSamplerImpl(const GlR3rSampler& rhs) = delete;
	~GlR3rSamplerImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

	void set() override;

private:
	void do_update(const R3rSamplerUpdateParam& param) override;

	const R3rSamplerState& do_get_state() const noexcept override;

private:
	struct SamplerDeleter
	{
		void operator()(GLuint gl_name) noexcept;
	};

	using SamplerResource = bstone::UniqueResource<GLuint, SamplerDeleter>;

private:
	GlR3rContext& context_;

	R3rSamplerState state_{};
	SamplerResource sampler_resource_{};

private:
	void set_mag_filter();
	void set_min_filter();

	void set_address_mode(R3rTextureAxis texture_axis, R3rAddressMode address_mode);
	void set_address_mode_u();
	void set_address_mode_v();

	void set_anisotropy();

	void set_initial_state();
};

// =========================================================================

using GlR3rSamplerImplPool = FixedPoolResource<GlR3rSamplerImpl, R3rLimits::max_samplers()>;
GlR3rSamplerImplPool gl_r3r_sampler_impl_pool{};

// =========================================================================

GlR3rSamplerImpl::GlR3rSamplerImpl(GlR3rContext& context, const R3rSamplerInitParam& param)
try
	:
	context_{context},
	state_{}
{
	const auto& device_features = context_.get_device_features();
	const auto& gl_device_features = context_.get_gl_device_features();

	state_ = param.state;

	if (device_features.is_sampler_available)
	{
		auto gl_name = GLuint{};

		if (gl_device_features.is_dsa_available)
		{
			glCreateSamplers(1, &gl_name);
			GlR3rError::check_optionally();
		}
		else
		{
			glGenSamplers(1, &gl_name);
			GlR3rError::check_optionally();
		}

		sampler_resource_.reset(gl_name);

		if (sampler_resource_.is_empty())
		{
			BSTONE_THROW_STATIC_SOURCE("Failed to create an object.");
		}
	}

	set_initial_state();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rSamplerImpl::~GlR3rSamplerImpl() = default;

void* GlR3rSamplerImpl::operator new(std::size_t size)
try {
	return gl_r3r_sampler_impl_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rSamplerImpl::operator delete(void* ptr)
{
	gl_r3r_sampler_impl_pool.deallocate(ptr);
}

void GlR3rSamplerImpl::set()
try {
	if (sampler_resource_.get() == 0U)
	{
		return;
	}

	glBindSampler(0, sampler_resource_.get());
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rSamplerImpl::do_update(const R3rSamplerUpdateParam& param)
try {
	auto is_modified = false;

	// Magnification filter.
	//
	auto is_mag_filter_modified = false;

	if (state_.mag_filter != param.state.mag_filter)
	{
		is_modified = true;
		is_mag_filter_modified = true;

		state_.mag_filter = param.state.mag_filter;
	}

	// Minification filter.
	//
	auto is_min_filter_modified = false;

	if (state_.min_filter != param.state.min_filter ||
		state_.mipmap_mode != param.state.mipmap_mode)
	{
		is_modified = true;
		is_min_filter_modified = true;

		state_.min_filter = param.state.min_filter;
		state_.mipmap_mode = param.state.mipmap_mode;
	}

	// U-axis address mode.
	//
	auto is_address_mode_u = false;

	if (state_.address_mode_u != param.state.address_mode_u)
	{
		is_modified = true;
		is_address_mode_u = true;

		state_.address_mode_u = param.state.address_mode_u;
	}

	// V-axis address mode.
	//
	auto is_address_mode_v = false;

	if (state_.address_mode_v != param.state.address_mode_v)
	{
		is_modified = true;
		is_address_mode_v = true;

		state_.address_mode_v = param.state.address_mode_v;
	}

	// Anisotropy.
	//
	auto is_anisotropy = false;

	if (state_.anisotropy != param.state.anisotropy)
	{
		is_modified = true;
		is_anisotropy = true;

		state_.anisotropy = param.state.anisotropy;
	}

	// Commit.
	//
	if (is_modified && !sampler_resource_.is_empty())
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSamplerState& GlR3rSamplerImpl::do_get_state() const noexcept
{
	return state_;
}

void GlR3rSamplerImpl::SamplerDeleter::operator()(GLuint gl_name) noexcept
{
	glDeleteSamplers(1, &gl_name);
	GlR3rError::ensure_no_errors_assert();
}

void GlR3rSamplerImpl::set_mag_filter()
try {
	const auto gl_mag_filter = GlR3rUtils::get_mag_filter(state_.mag_filter);
	glSamplerParameteri(sampler_resource_.get(), GL_TEXTURE_MAG_FILTER, gl_mag_filter);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rSamplerImpl::set_min_filter()
try {
	const auto gl_min_filter = GlR3rUtils::get_min_filter(state_.min_filter, state_.mipmap_mode);
	glSamplerParameteri(sampler_resource_.get(), GL_TEXTURE_MIN_FILTER, gl_min_filter);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rSamplerImpl::set_address_mode(R3rTextureAxis texture_axis, R3rAddressMode address_mode)
try {
	const auto gl_wrap_axis = GlR3rUtils::get_texture_wrap_axis(texture_axis);
	const auto gl_address_mode = GlR3rUtils::get_address_mode(address_mode);

	glSamplerParameteri(sampler_resource_.get(), gl_wrap_axis, gl_address_mode);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rSamplerImpl::set_address_mode_u()
try {
	set_address_mode(R3rTextureAxis::u, state_.address_mode_u);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rSamplerImpl::set_address_mode_v()
try {
	set_address_mode(R3rTextureAxis::v, state_.address_mode_v);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rSamplerImpl::set_anisotropy()
try {
	GlR3rUtils::set_sampler_anisotropy(
		sampler_resource_.get(),
		context_.get_device_features(),
		state_.anisotropy);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rSamplerImpl::set_initial_state()
try {
	if (sampler_resource_.get() == 0U)
	{
		return;
	}

	set_mag_filter();
	set_min_filter();
	set_address_mode_u();
	set_address_mode_v();
	set_anisotropy();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// =========================================================================

GlR3rSamplerUPtr make_gl_r3r_sampler(GlR3rContext& context, const R3rSamplerInitParam& param)
{
	return std::make_unique<GlR3rSamplerImpl>(context, param);
}

} // namespace bstone
