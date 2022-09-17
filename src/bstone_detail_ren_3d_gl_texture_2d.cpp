/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// OpenGL 2D texture (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_detail_ren_3d_gl_texture_2d.h"

#include "bstone_exception.h"
#include "bstone_ren_3d_limits.h"
#include "bstone_ren_3d_tests.h"
#include "bstone_unique_resource.h"

#include "bstone_detail_ren_3d_gl_api.h"
#include "bstone_detail_ren_3d_gl_context.h"
#include "bstone_detail_ren_3d_gl_error.h"
#include "bstone_detail_ren_3d_gl_utils.h"
#include "bstone_detail_ren_3d_gl_sampler_mgr.h"
#include "bstone_detail_ren_3d_gl_texture_mgr.h"
#include "bstone_detail_ren_3d_utils.h"


namespace bstone
{
namespace detail
{


// =========================================================================
// Ren3dGlTexture2dException
//

class Ren3dGlTexture2dException :
	public Exception
{
public:
	explicit Ren3dGlTexture2dException(
		const char* message) noexcept
		:
		Exception{"REN_3D_GL_TEXTURE_2D", message}
	{
	}
}; // Ren3dGlTexture2dException

//
// Ren3dGlTexture2dException
// =========================================================================


// =========================================================================
// Ren3dGlTexture2dImpl
//

class Ren3dGlTexture2dImpl final :
	public Ren3dGlTexture2d
{
public:
	Ren3dGlTexture2dImpl(
		const Ren3dGlTextureMgrPtr texture_manager,
		const Ren3dCreateTexture2dParam& param);

	~Ren3dGlTexture2dImpl() override;


	void update(
		const Ren3dTexture2dUpdateParam& param) override;

	void generate_mipmaps() override;


	void set() override;

	void update_sampler_state(
		const Ren3dSamplerState& new_sampler_state) override;


private:
	const Ren3dGlTextureMgrPtr texture_manager_;
	const Ren3dDeviceFeatures& device_features_;
	const Ren3dGlDeviceFeatures& gl_device_features_;

	Ren3dPixelFormat pixel_format_;
	GLenum gl_internal_format_;
	GLenum gl_format_;
	GLenum gl_type_;

	int width_;
	int height_;

	int mipmap_count_;

	Ren3dSamplerState sampler_state_;


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	void validate(
		const Ren3dCreateTexture2dParam& param);

	void validate(
		const Ren3dTexture2dUpdateParam& param);


	void bind() override;

	void upload_mipmap(
		const int mipmap_level,
		const int width,
		const int height,
		const void* const src_data);

	void set_mag_filter();

	void set_min_filter();

	void set_address_mode(
		const Ren3dTextureAxis texture_axis,
		const Ren3dAddressMode address_mode);

	void set_address_mode_u();

	void set_address_mode_v();

	void set_anisotropy();

	void set_sampler_state_defaults();


	static void texture_deleter(
		GLuint gl_name) noexcept;

	using TextureResource = UniqueResource<GLuint, texture_deleter>;
	TextureResource texture_resource_;
}; // Ren3dGlTexture2d

//
// Ren3dGlTexture2dImpl
// =========================================================================


// ==========================================================================
// Ren3dGlTexture2dImpl
//

Ren3dGlTexture2dImpl::Ren3dGlTexture2dImpl(
	const Ren3dGlTextureMgrPtr texture_manager,
	const Ren3dCreateTexture2dParam& param)
try
	:
	texture_manager_{texture_manager},
	device_features_{texture_manager_->get_context()->get_device_features()},
	gl_device_features_{texture_manager_->get_context()->get_gl_device_features()},
	pixel_format_{},
	gl_internal_format_{},
	gl_format_{},
	gl_type_{},
	width_{},
	height_{},
	mipmap_count_{},
	sampler_state_{},
	texture_resource_{}
{
	validate(param);

	pixel_format_ = param.pixel_format_;

	const auto is_es = (gl_device_features_.context_kind_ == Ren3dGlContextKind::es);

	switch (pixel_format_)
	{
		case Ren3dPixelFormat::rgba_8_unorm:
			gl_internal_format_ = (is_es ? GL_RGBA : GL_RGBA8);
			gl_format_ = GL_RGBA;
			gl_type_ = GL_UNSIGNED_BYTE;
			break;

		default:
			fail("Unsupported image format.");
	}

	width_ = param.width_;
	height_ = param.height_;
	mipmap_count_ = param.mipmap_count_;

	const auto max_mipmap_count = Ren3dUtils::calculate_mipmap_count(width_, height_);

	if (mipmap_count_ > max_mipmap_count)
	{
		fail("Mipmap count out of range.");
	}

// TODO Disable when OpenGL ES 2.0 won't be supported.
#if 1
	if (mipmap_count_ > 1 && mipmap_count_ != max_mipmap_count)
	{
		fail("Mismatch mipmap count.");
	}
#endif

	auto gl_name = GLuint{};

	if (gl_device_features_.is_dsa_available_)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &gl_name);
		Ren3dGlError::ensure_debug();
	}
	else
	{
		glGenTextures(1, &gl_name);
		Ren3dGlError::ensure_debug();
	}

	texture_resource_.reset(gl_name);

	if (!texture_resource_)
	{
		fail("Failed to create an object.");
	}

	if (!gl_device_features_.is_dsa_available_)
	{
		bind();
	}

// TODO Enable when OpenGL ES 2.0 won't be supported.
#if 0
	if (gl_device_features_.is_dsa_available_)
	{
		glTextureParameteri(texture_resource_.get(), GL_TEXTURE_BASE_LEVEL, 0);
		Ren3dGlError::ensure_debug();

		glTextureParameteri(texture_resource_.get(), GL_TEXTURE_MAX_LEVEL, mipmap_count_ - 1);
		Ren3dGlError::ensure_debug();
	}
	else
	{
		bind();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		Ren3dGlError::ensure_debug();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count_ - 1);
		Ren3dGlError::ensure_debug();
	}
#endif

	set_sampler_state_defaults();

	if (gl_device_features_.is_dsa_available_)
	{
		glTextureStorage2D(
			texture_resource_.get(),
			mipmap_count_,
			gl_internal_format_,
			width_,
			height_
		);

		Ren3dGlError::ensure_debug();
	}
	else
	{
		auto mipmap_width = width_;
		auto mipmap_height = height_;

		for (int i_mipmap = 0; i_mipmap < mipmap_count_; ++i_mipmap)
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

			Ren3dGlError::ensure_debug();

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
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dGlTexture2dImpl::~Ren3dGlTexture2dImpl() = default;

void Ren3dGlTexture2dImpl::update(
	const Ren3dTexture2dUpdateParam& param)
try
{
	validate(param);

	if (param.mipmap_level_ >= mipmap_count_)
	{
		fail("Mipmap level out of range.");
	}

	if (!gl_device_features_.is_dsa_available_)
	{
		texture_manager_->set(this);
	}

	auto mipmap_width = width_;
	auto mipmap_height = height_;

	for (auto i = 0; i < param.mipmap_level_; ++i)
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

	upload_mipmap(param.mipmap_level_, mipmap_width, mipmap_height, param.image_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::generate_mipmaps()
try
{
	if (mipmap_count_ <= 1)
	{
		fail("Base mipmap.");
	}

	if (!device_features_.is_mipmap_available_)
	{
		fail("Mipmap generation not available.");
	}

	if (gl_device_features_.is_dsa_available_)
	{
		glGenerateTextureMipmap(texture_resource_.get());
		Ren3dGlError::ensure_debug();
	}
	else
	{
		texture_manager_->set(this);

		Ren3dGlUtils::generate_mipmap(
			GL_TEXTURE_2D,
			device_features_,
			gl_device_features_);
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::texture_deleter(
	GLuint gl_name) noexcept
{
	glDeleteTextures(1, &gl_name);
	Ren3dGlError::ensure_assert();
}

[[noreturn]]
void Ren3dGlTexture2dImpl::fail(
	const char* message)
{
	throw Ren3dGlTexture2dException{message};
}

[[noreturn]]
void Ren3dGlTexture2dImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(Ren3dGlTexture2dException{message});
}

void Ren3dGlTexture2dImpl::validate(
	const Ren3dCreateTexture2dParam& param)
try
{
	switch (param.pixel_format_)
	{
		case Ren3dPixelFormat::rgba_8_unorm:
			break;

		default:
			fail("Invalid pixel format.");
	}

	if (param.width_ <= 0)
	{
		fail("Invalid width.");
	}

	if (param.height_ <= 0)
	{
		fail("Invalid height.");
	}

	if (param.mipmap_count_ <= 0)
	{
		fail("Invalid mipmap count.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::validate(
	const Ren3dTexture2dUpdateParam& param)
try
{
	if (param.mipmap_level_ < 0 ||
		param.mipmap_level_ >= Ren3dLimits::max_mipmap_count)
	{
		fail("Mipmap level out of range.");
	}

	if (!param.image_)
	{
		fail("Null image data.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::bind()
try
{
	glBindTexture(GL_TEXTURE_2D, texture_resource_.get());
	Ren3dGlError::ensure_debug();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::upload_mipmap(
	const int mipmap_level,
	const int width,
	const int height,
	const void* const src_data)
try
{
	if (gl_device_features_.is_dsa_available_)
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

		Ren3dGlError::ensure_debug();
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

		Ren3dGlError::ensure_debug();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::set()
try
{
	bind();

	if (!gl_device_features_.is_dsa_available_)
	{
		const auto sampler_manger = texture_manager_->get_context()->get_sampler_manager();
		const auto& sampler_state = sampler_manger->get_current_state();
		update_sampler_state(sampler_state);
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::set_mag_filter()
try
{
	const auto gl_mag_filter = Ren3dGlUtils::get_mag_filter(sampler_state_.mag_filter_);

	if (gl_device_features_.is_dsa_available_)
	{
		glTextureParameteri(texture_resource_.get(), GL_TEXTURE_MAG_FILTER, gl_mag_filter);
		Ren3dGlError::ensure_debug();
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_mag_filter);
		Ren3dGlError::ensure_debug();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::set_min_filter()
try
{
	const auto gl_min_filter = Ren3dGlUtils::get_min_filter(
		sampler_state_.min_filter_,
		sampler_state_.mipmap_mode_
	);

	if (gl_device_features_.is_dsa_available_)
	{
		glTextureParameteri(texture_resource_.get(), GL_TEXTURE_MIN_FILTER, gl_min_filter);
		Ren3dGlError::ensure_debug();
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_min_filter);
		Ren3dGlError::ensure_debug();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::set_address_mode(
	const Ren3dTextureAxis texture_axis,
	const Ren3dAddressMode address_mode)
try
{
	const auto gl_wrap_axis = Ren3dGlUtils::get_texture_wrap_axis(texture_axis);
	const auto gl_address_mode = Ren3dGlUtils::get_address_mode(address_mode);

	if (gl_device_features_.is_dsa_available_)
	{
		glTextureParameteri(texture_resource_.get(), gl_wrap_axis, gl_address_mode);
		Ren3dGlError::ensure_debug();
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, gl_wrap_axis, gl_address_mode);
		Ren3dGlError::ensure_debug();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::set_address_mode_u()
try
{
	set_address_mode(Ren3dTextureAxis::u, sampler_state_.address_mode_u_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::set_address_mode_v()
try
{
	set_address_mode(Ren3dTextureAxis::v, sampler_state_.address_mode_v_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::set_anisotropy()
try
{
	if (!device_features_.is_anisotropy_available_)
	{
		return;
	}

	auto anisotropy = sampler_state_.anisotropy_;

	if (anisotropy < Ren3dLimits::min_anisotropy_off)
	{
		anisotropy = Ren3dLimits::min_anisotropy_off;
	}
	else if (anisotropy > device_features_.max_anisotropy_degree_)
	{
		anisotropy = device_features_.max_anisotropy_degree_;
	}

	const auto gl_anisotropy = static_cast<GLfloat>(anisotropy);

	if (gl_device_features_.is_dsa_available_)
	{
		glTextureParameterf(texture_resource_.get(), GL_TEXTURE_MAX_ANISOTROPY, gl_anisotropy);
		Ren3dGlError::ensure_debug();
	}
	else
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, gl_anisotropy);
		Ren3dGlError::ensure_debug();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlTexture2dImpl::update_sampler_state(
	const Ren3dSamplerState& new_sampler_state)
try
{
	auto is_modified = false;

	// Magnification filter.
	//
	auto is_mag_filter_modified = false;

	if (sampler_state_.mag_filter_ != new_sampler_state.mag_filter_)
	{
		is_modified = true;
		is_mag_filter_modified = true;

		sampler_state_.mag_filter_ = new_sampler_state.mag_filter_;
	}

	// Minification filter.
	//
	auto is_min_filter_modified = false;

	if (sampler_state_.min_filter_ != new_sampler_state.min_filter_ ||
		sampler_state_.mipmap_mode_ != new_sampler_state.mipmap_mode_)
	{
		is_modified = true;
		is_min_filter_modified = true;

		sampler_state_.min_filter_ = new_sampler_state.min_filter_;
		sampler_state_.mipmap_mode_ = new_sampler_state.mipmap_mode_;
	}

	// U-axis address mode.
	//
	auto is_address_mode_u = false;

	if (sampler_state_.address_mode_u_ != new_sampler_state.address_mode_u_)
	{
		is_modified = true;
		is_address_mode_u = true;

		sampler_state_.address_mode_u_ = new_sampler_state.address_mode_u_;
	}

	// V-axis address mode.
	//
	auto is_address_mode_v = false;

	if (sampler_state_.address_mode_v_ != new_sampler_state.address_mode_v_)
	{
		is_modified = true;
		is_address_mode_v = true;

		sampler_state_.address_mode_v_ = new_sampler_state.address_mode_v_;
	}

	// Anisotropy.
	//
	auto is_anisotropy = false;

	if (sampler_state_.anisotropy_ != new_sampler_state.anisotropy_)
	{
		is_modified = true;
		is_anisotropy = true;

		sampler_state_.anisotropy_ = new_sampler_state.anisotropy_;
	}


	// Modify.
	//
	if (is_modified)
	{
		texture_manager_->set(this);

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

void Ren3dGlTexture2dImpl::set_sampler_state_defaults()
try
{
	sampler_state_.mag_filter_ = Ren3dFilterKind::nearest;
	set_mag_filter();

	sampler_state_.min_filter_ = Ren3dFilterKind::nearest;
	sampler_state_.mipmap_mode_ = Ren3dMipmapMode::none;
	set_min_filter();

	sampler_state_.address_mode_u_ = Ren3dAddressMode::clamp;
	set_address_mode_u();

	sampler_state_.address_mode_v_ = Ren3dAddressMode::clamp;
	set_address_mode_v();

	sampler_state_.anisotropy_ = Ren3dLimits::min_anisotropy_off;
	set_anisotropy();
}
catch (...)
{
	fail_nested(__func__);
}

//
// Ren3dGlTexture2dImpl
// ==========================================================================


// =========================================================================
// Ren3dGlTexture2dFactory
//

Ren3dGlTexture2dUPtr Ren3dGlTexture2dFactory::create(
	const Ren3dGlTextureMgrPtr texture_manager,
	const Ren3dCreateTexture2dParam& param)
{
	return std::make_unique<Ren3dGlTexture2dImpl>(texture_manager, param);
}

//
// Ren3dGlTexture2dFactory
// =========================================================================


} // detail
} // bstone
