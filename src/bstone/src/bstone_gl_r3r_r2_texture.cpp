/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: 2D Texture

#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_unique_resource.h"

#include "bstone_r3r_limits.h"
#include "bstone_r3r_tests.h"
#include "bstone_r3r_utils.h"

#include "bstone_gl_r3r_api.h"
#include "bstone_gl_r3r_context.h"
#include "bstone_gl_r3r_error.h"
#include "bstone_gl_r3r_r2_texture.h"
#include "bstone_gl_r3r_sampler_mgr.h"
#include "bstone_gl_r3r_utils.h"

namespace bstone {

GlR3rR2Texture::GlR3rR2Texture() = default;

GlR3rR2Texture::~GlR3rR2Texture() = default;

// =========================================================================

class GlR3rR2TextureImpl final : public GlR3rR2Texture
{
public:
	GlR3rR2TextureImpl(GlR3rContext& context, const R3rR2TextureInitParam& param);
	~GlR3rR2TextureImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	void do_update(const R3rR2TextureUpdateParam& param) override;
	void do_generate_mipmaps() override;

public:
	void set() override;
	void update_sampler_state(const R3rSamplerState& new_sampler_state) override;

private:
	struct TextureDeleter
	{
		void operator()(GLuint gl_name) noexcept;
	};

	using TextureResource = UniqueResource<GLuint, TextureDeleter>;

private:
	GlR3rContext& context_;
	const R3rDeviceFeatures& device_features_;
	const GlR3rDeviceFeatures& gl_device_features_;

	R3rPixelFormat pixel_format_{};
	GLenum gl_internal_format_{};
	GLenum gl_format_{};
	GLenum gl_type_{};

	int width_{};
	int height_{};

	int mipmap_count_{};

	R3rSamplerState sampler_state_{};

	TextureResource texture_resource_;

private:
	void validate(const R3rR2TextureInitParam& param);
	void validate(const R3rR2TextureUpdateParam& param);

	void bind() override;

	void upload_mipmap(int mipmap_level, int width, int height, const void* src_data);

	void set_mag_filter();
	void set_min_filter();

	void set_address_mode(R3rTextureAxis texture_axis, R3rAddressMode address_mode);
	void set_address_mode_u();
	void set_address_mode_v();

	void set_anisotropy();

	void set_sampler_state_defaults();
};

// =========================================================================

using GlR3rR2TextureImplPool = FixedPoolResource<GlR3rR2TextureImpl, R3rLimits::max_textures()>;
GlR3rR2TextureImplPool gl_r3r_r2_texture_impl_pool{};

// =========================================================================

GlR3rR2TextureImpl::GlR3rR2TextureImpl(GlR3rContext& context, const R3rR2TextureInitParam& param)
try
	:
	context_{context},
	device_features_{context_.get_device_features()},
	gl_device_features_{context_.get_gl_device_features()}
{
	validate(param);

	pixel_format_ = param.pixel_format;

	const auto is_es = (gl_device_features_.context_profile == sys::GlContextProfile::es);

	switch (pixel_format_)
	{
		case R3rPixelFormat::rgba_8_unorm:
			gl_internal_format_ = (is_es ? GL_RGBA : GL_RGBA8);
			gl_format_ = GL_RGBA;
			gl_type_ = GL_UNSIGNED_BYTE;
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported image format.");
	}

	width_ = param.width;
	height_ = param.height;
	mipmap_count_ = param.mipmap_count;

	const auto max_mipmap_count = R3rUtils::calculate_mipmap_count(width_, height_);

	if (mipmap_count_ > max_mipmap_count)
	{
		BSTONE_THROW_STATIC_SOURCE("Mipmap count out of range.");
	}

// TODO Disable when OpenGL ES 2.0 won't be supported.
#if 1
	if (mipmap_count_ > 1 && mipmap_count_ != max_mipmap_count)
	{
		BSTONE_THROW_STATIC_SOURCE("Mismatch mipmap count.");
	}
#endif

	auto gl_name = GLuint{};

	if (gl_device_features_.is_dsa_available)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &gl_name);
		GlR3rError::check_optionally();
	}
	else
	{
		glGenTextures(1, &gl_name);
		GlR3rError::check_optionally();
	}

	texture_resource_.reset(gl_name);

	if (texture_resource_.is_empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create an object.");
	}

	if (!gl_device_features_.is_dsa_available)
	{
		bind();
	}

// TODO Enable when OpenGL ES 2.0 won't be supported.
#if 0
	if (gl_device_features_.is_dsa_available)
	{
		glTextureParameteri(texture_resource_.get(), GL_TEXTURE_BASE_LEVEL, 0);
		GlR3rError::check_optionally();

		glTextureParameteri(texture_resource_.get(), GL_TEXTURE_MAX_LEVEL, mipmap_count_ - 1);
		GlR3rError::check_optionally();
	}
	else
	{
		bind();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		GlR3rError::check_optionally();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count_ - 1);
		GlR3rError::check_optionally();
	}
#endif

	set_sampler_state_defaults();

	if (gl_device_features_.is_dsa_available)
	{
		glTextureStorage2D(
			texture_resource_.get(),
			mipmap_count_,
			gl_internal_format_,
			width_,
			height_);

		GlR3rError::check_optionally();
	}
	else
	{
		auto mipmap_width = width_;
		auto mipmap_height = height_;

		for (auto i_mipmap = 0; i_mipmap < mipmap_count_; ++i_mipmap)
		{
			glTexImage2D(
				GL_TEXTURE_2D, // target
				i_mipmap, // level
				gl_internal_format_, // internal format
				mipmap_width, // width
				mipmap_height, // height
				0, // border
				gl_format_, // format
				gl_type_, // type
				nullptr // pixels
			);

			GlR3rError::check_optionally();

			if (mipmap_width > 1)
			{
				mipmap_width /= 2;
			}

			if (mipmap_height > 1)
			{
				mipmap_height /= 2;
			}
		}
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rR2TextureImpl::~GlR3rR2TextureImpl() = default;

void* GlR3rR2TextureImpl::operator new(std::size_t size)
try {
	return gl_r3r_r2_texture_impl_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::operator delete(void* ptr)
{
	gl_r3r_r2_texture_impl_pool.deallocate(ptr);
}

void GlR3rR2TextureImpl::do_update(const R3rR2TextureUpdateParam& param)
try {
	validate(param);

	if (param.mipmap_level >= mipmap_count_)
	{
		BSTONE_THROW_STATIC_SOURCE("Mipmap level out of range.");
	}

	if (!gl_device_features_.is_dsa_available)
	{
		context_.bind_r2_texture(this);
	}

	auto mipmap_width = width_;
	auto mipmap_height = height_;

	for (auto i = 0; i < param.mipmap_level; ++i)
	{
		if (mipmap_width > 1)
		{
			mipmap_width /= 2;
		}

		if (mipmap_height > 1)
		{
			mipmap_height /= 2;
		}
	}

	upload_mipmap(param.mipmap_level, mipmap_width, mipmap_height, param.image);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::do_generate_mipmaps()
try {
	if (mipmap_count_ <= 1)
	{
		BSTONE_THROW_STATIC_SOURCE("Base mipmap.");
	}

	if (!device_features_.is_mipmap_available)
	{
		BSTONE_THROW_STATIC_SOURCE("Mipmap generation not available.");
	}

	if (gl_device_features_.is_dsa_available)
	{
		glGenerateTextureMipmap(texture_resource_.get());
		GlR3rError::check_optionally();
	}
	else
	{
		context_.bind_r2_texture(this);
		GlR3rUtils::generate_mipmap(GL_TEXTURE_2D, device_features_, gl_device_features_);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::TextureDeleter::operator()(GLuint gl_name) noexcept
{
	glDeleteTextures(1, &gl_name);
	GlR3rError::ensure_no_errors_assert();
}

void GlR3rR2TextureImpl::validate(const R3rR2TextureInitParam& param)
try {
	switch (param.pixel_format)
	{
		case R3rPixelFormat::rgba_8_unorm:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Invalid pixel format.");
	}

	if (param.width <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid width.");
	}

	if (param.height <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid height.");
	}

	if (param.mipmap_count <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid mipmap count.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::validate(const R3rR2TextureUpdateParam& param)
try {
	if (param.mipmap_level < 0 ||
		param.mipmap_level >= R3rLimits::max_mipmap_count())
	{
		BSTONE_THROW_STATIC_SOURCE("Mipmap level out of range.");
	}

	if (param.image == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null image data.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::bind()
try {
	glBindTexture(GL_TEXTURE_2D, texture_resource_.get());
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::upload_mipmap(int mipmap_level, int width, int height, const void* src_data)
try {
	if (gl_device_features_.is_dsa_available)
	{
		glTextureSubImage2D(
			texture_resource_.get(), // target
			mipmap_level, // level
			0, // xoffset
			0, // yoffset
			width, // width
			height, // height
			gl_format_, // format
			gl_type_, // type
			src_data // pixels
		);

		GlR3rError::check_optionally();
	}
	else
	{
		glTexSubImage2D(
			GL_TEXTURE_2D, // target
			mipmap_level, // level
			0, // xoffset
			0, // yoffset
			width, // width
			height, // height
			gl_format_, // format
			gl_type_, // type
			src_data // pixels
		);

		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::set()
try {
	bind();

	if (!gl_device_features_.is_dsa_available)
	{
		const auto& sampler_manger = context_.get_sampler_manager();
		const auto& sampler_state = sampler_manger.get_current_state();
		update_sampler_state(sampler_state);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::set_mag_filter()
try {
	const auto gl_mag_filter = GlR3rUtils::get_mag_filter(sampler_state_.mag_filter);

	if (gl_device_features_.is_dsa_available)
	{
		glTextureParameteri(texture_resource_.get(), GL_TEXTURE_MAG_FILTER, gl_mag_filter);
		GlR3rError::check_optionally();
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_mag_filter);
		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::set_min_filter()
try {
	const auto gl_min_filter = GlR3rUtils::get_min_filter(
		sampler_state_.min_filter,
		sampler_state_.mipmap_mode);

	if (gl_device_features_.is_dsa_available)
	{
		glTextureParameteri(texture_resource_.get(), GL_TEXTURE_MIN_FILTER, gl_min_filter);
		GlR3rError::check_optionally();
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_min_filter);
		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::set_address_mode(R3rTextureAxis texture_axis, R3rAddressMode address_mode)
try {
	const auto gl_wrap_axis = GlR3rUtils::get_texture_wrap_axis(texture_axis);
	const auto gl_address_mode = GlR3rUtils::get_address_mode(address_mode);

	if (gl_device_features_.is_dsa_available)
	{
		glTextureParameteri(texture_resource_.get(), gl_wrap_axis, gl_address_mode);
		GlR3rError::check_optionally();
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, gl_wrap_axis, gl_address_mode);
		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::set_address_mode_u()
try {
	set_address_mode(R3rTextureAxis::u, sampler_state_.address_mode_u);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::set_address_mode_v()
try {
	set_address_mode(R3rTextureAxis::v, sampler_state_.address_mode_v);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::set_anisotropy()
try {
	if (!device_features_.is_anisotropy_available)
	{
		return;
	}

	auto anisotropy = sampler_state_.anisotropy;

	if (anisotropy < R3rLimits::min_anisotropy_off())
	{
		anisotropy = R3rLimits::min_anisotropy_off();
	}
	else if (anisotropy > device_features_.max_anisotropy_degree)
	{
		anisotropy = device_features_.max_anisotropy_degree;
	}

	const auto gl_anisotropy = static_cast<GLfloat>(anisotropy);

	if (gl_device_features_.is_dsa_available)
	{
		glTextureParameterf(texture_resource_.get(), GL_TEXTURE_MAX_ANISOTROPY, gl_anisotropy);
		GlR3rError::check_optionally();
	}
	else
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, gl_anisotropy);
		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rR2TextureImpl::update_sampler_state(const R3rSamplerState& new_sampler_state)
try {
	auto is_modified = false;

	// Magnification filter.
	//
	auto is_mag_filter_modified = false;

	if (sampler_state_.mag_filter != new_sampler_state.mag_filter)
	{
		is_modified = true;
		is_mag_filter_modified = true;

		sampler_state_.mag_filter = new_sampler_state.mag_filter;
	}

	// Minification filter.
	//
	auto is_min_filter_modified = false;

	if (sampler_state_.min_filter != new_sampler_state.min_filter ||
		sampler_state_.mipmap_mode != new_sampler_state.mipmap_mode)
	{
		is_modified = true;
		is_min_filter_modified = true;

		sampler_state_.min_filter = new_sampler_state.min_filter;
		sampler_state_.mipmap_mode = new_sampler_state.mipmap_mode;
	}

	// U-axis address mode.
	//
	auto is_address_mode_u = false;

	if (sampler_state_.address_mode_u != new_sampler_state.address_mode_u)
	{
		is_modified = true;
		is_address_mode_u = true;

		sampler_state_.address_mode_u = new_sampler_state.address_mode_u;
	}

	// V-axis address mode.
	//
	auto is_address_mode_v = false;

	if (sampler_state_.address_mode_v != new_sampler_state.address_mode_v)
	{
		is_modified = true;
		is_address_mode_v = true;

		sampler_state_.address_mode_v = new_sampler_state.address_mode_v;
	}

	// Anisotropy.
	//
	auto is_anisotropy = false;

	if (sampler_state_.anisotropy != new_sampler_state.anisotropy)
	{
		is_modified = true;
		is_anisotropy = true;

		sampler_state_.anisotropy = new_sampler_state.anisotropy;
	}


	// Modify.
	//
	if (is_modified)
	{
		context_.bind_r2_texture(this);

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

void GlR3rR2TextureImpl::set_sampler_state_defaults()
try {
	sampler_state_.mag_filter = R3rFilterType::nearest;
	set_mag_filter();

	sampler_state_.min_filter = R3rFilterType::nearest;
	sampler_state_.mipmap_mode = R3rMipmapMode::none;
	set_min_filter();

	sampler_state_.address_mode_u = R3rAddressMode::clamp;
	set_address_mode_u();

	sampler_state_.address_mode_v = R3rAddressMode::clamp;
	set_address_mode_v();

	sampler_state_.anisotropy = R3rLimits::min_anisotropy_off();
	set_anisotropy();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

GlR3rR2TextureUPtr make_gl_r3r_r2_texture(GlR3rContext& context, const R3rR2TextureInitParam& param)
{
	return std::make_unique<GlR3rR2TextureImpl>(context, param);
}

} // bstone
