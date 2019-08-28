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
// OpenGL 2D texture (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_texture_2d.h"

#include "bstone_exception.h"
#include "bstone_ogl_unique_resources.h"

#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_state.h"
#include "bstone_detail_ogl_texture_manager.h"
#include "bstone_detail_renderer_utils.h"


namespace bstone
{
namespace detail
{


// =========================================================================
// OglTexture2d
//

OglTexture2d::OglTexture2d() = default;

OglTexture2d::~OglTexture2d() = default;

//
// OglTexture2d
// =========================================================================


// =========================================================================
// GenericOglTexture2d
//

class GenericOglTexture2d final :
	public OglTexture2d
{
public:
	GenericOglTexture2d(
		const OglTextureManagerPtr ogl_texture_manager,
		const RendererTexture2dCreateParam& param);

	GenericOglTexture2d(
		const OglTexture2d& rhs) = delete;

	~GenericOglTexture2d() override;


	void update(
		const RendererTexture2dUpdateParam& param) override;

	void generate_mipmaps() override;


	void bind() override;

	void update_sampler_state(
		const RendererSamplerState& new_sampler_state) override;


private:
	void uninitialize_internal();

	void upload_mipmap(
		const int mipmap_level,
		const int width,
		const int height,
		const R8g8b8a8CPtr src_pixels);

	void set_mag_filter();

	void set_min_filter();

	void set_address_mode(
		const RendererTextureAxis texture_axis,
		const RendererAddressMode address_mode);

	void set_address_mode_u();

	void set_address_mode_v();

	void set_anisotropy();

	void set_sampler_state_defaults();


	const OglTextureManagerPtr ogl_texture_manager_;

	RendererPixelFormat storage_pixel_format_;

	int width_;
	int height_;

	int mipmap_count_;

	RendererSamplerState sampler_state_;

	OglTextureUniqueResource ogl_resource_;


	void initialize(
		const RendererTexture2dCreateParam& param);
}; // OglTexture2d

using GenericOglTexture2dPtr = GenericOglTexture2d*;
using GenericOglTexture2dUPtr = std::unique_ptr<GenericOglTexture2d>;

//
// GenericOglTexture2d
// =========================================================================


// ==========================================================================
// GenericOglTexture2d
//

GenericOglTexture2d::GenericOglTexture2d(
	const OglTextureManagerPtr ogl_texture_manager,
	const RendererTexture2dCreateParam& param)
	:
	ogl_texture_manager_{ogl_texture_manager},
	storage_pixel_format_{},
	width_{},
	height_{},
	mipmap_count_{},
	sampler_state_{},
	ogl_resource_{}
{
	initialize(param);
}

GenericOglTexture2d::~GenericOglTexture2d()
{
	uninitialize_internal();
}

void GenericOglTexture2d::update(
	const RendererTexture2dUpdateParam& param)
{
	RendererUtils::validate_texture_2d_update_param(param);

	if (param.mipmap_level_ >= mipmap_count_)
	{
		throw Exception{"Mipmap level out of range."};
	}

	const auto ogl_state = ogl_texture_manager_->ogl_state_get();

	ogl_state->texture_2d_set(this);

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

	upload_mipmap(param.mipmap_level_, mipmap_width, mipmap_height, param.rgba_pixels_);
}

void GenericOglTexture2d::generate_mipmaps()
{
	if (mipmap_count_ <= 1)
	{
		throw Exception{"Base mipmap."};
	}

	const auto ogl_state = ogl_texture_manager_->ogl_state_get();
	const auto& device_features = ogl_state->get_device_features();

	if (!device_features.mipmap_is_available_)
	{
		throw Exception{"Mipmap generation not available."};
	}

	const auto& ogl_device_features = ogl_state->get_ogl_device_features();

	if (ogl_device_features.mipmap_function_ == nullptr)
	{
		throw Exception{"Null mipmap generation function."};
	}

	ogl_device_features.mipmap_function_(GL_TEXTURE_2D);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglTexture2d::initialize(
	const RendererTexture2dCreateParam& param)
{
	RendererUtils::validate_texture_2d_create_param(param);

	storage_pixel_format_ = param.storage_pixel_format_;

	width_ = param.width_;
	height_ = param.height_;
	mipmap_count_ = param.mipmap_count_;

	const auto max_mipmap_count = RendererUtils::calculate_mipmap_count(width_, height_);

	if (mipmap_count_ > max_mipmap_count)
	{
		throw Exception{"Mipmap count out of range."};
	}

	const auto internal_format = (storage_pixel_format_ == RendererPixelFormat::r8g8b8a8_unorm ? GL_RGBA8 : GL_RGB8);

	auto ogl_name = GLuint{};

	::glGenTextures(1, &ogl_name);
	assert(!OglRendererUtils::was_errors());

	if (ogl_name == 0)
	{
		throw Exception{"Failed to create OpenGL 2D-texture object."};
	}

	ogl_resource_.reset(ogl_name);

	ogl_texture_manager_->texture_2d_set(this);

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	assert(!OglRendererUtils::was_errors());

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_count_ - 1);
	assert(!OglRendererUtils::was_errors());

	set_sampler_state_defaults();

	auto mipmap_width = width_;
	auto mipmap_height = height_;

	for (int i_mipmap = 0; i_mipmap < mipmap_count_; ++i_mipmap)
	{
		::glTexImage2D(
			GL_TEXTURE_2D, // target
			i_mipmap, // level
			internal_format, // internal format
			mipmap_width, // width
			mipmap_height, // height
			0, // border
			GL_RGBA, // format
			GL_UNSIGNED_BYTE, // type
			nullptr // pixels
		);

		assert(!OglRendererUtils::was_errors());

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

void GenericOglTexture2d::uninitialize_internal()
{
	ogl_resource_.reset();
}

void GenericOglTexture2d::upload_mipmap(
	const int mipmap_level,
	const int width,
	const int height,
	const R8g8b8a8CPtr src_pixels)
{
	::glTexSubImage2D(
		GL_TEXTURE_2D, // target
		mipmap_level, // level
		0, // xoffset
		0, // yoffset
		width, // width
		height, // height
		GL_RGBA, // format
		GL_UNSIGNED_BYTE, // type
		src_pixels // pixels
	);

	assert(!OglRendererUtils::was_errors());
}

void GenericOglTexture2d::bind()
{
	OglRendererUtils::texture_2d_bind(ogl_resource_);
}

void GenericOglTexture2d::set_mag_filter()
{
	const auto ogl_mag_filter = OglRendererUtils::filter_get_mag(sampler_state_.mag_filter_);

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ogl_mag_filter);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglTexture2d::set_min_filter()
{
	const auto ogl_min_filter = OglRendererUtils::filter_get_min(
		sampler_state_.min_filter_,
		sampler_state_.mipmap_mode_
	);

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ogl_min_filter);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglTexture2d::set_address_mode(
	const RendererTextureAxis texture_axis,
	const RendererAddressMode address_mode)
{
	const auto ogl_wrap_axis = OglRendererUtils::texture_wrap_get_axis(texture_axis);
	const auto ogl_address_mode = OglRendererUtils::address_mode_get(address_mode);

	::glTexParameteri(GL_TEXTURE_2D, ogl_wrap_axis, ogl_address_mode);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglTexture2d::set_address_mode_u()
{
	set_address_mode(RendererTextureAxis::u, sampler_state_.address_mode_u_);
}

void GenericOglTexture2d::set_address_mode_v()
{
	set_address_mode(RendererTextureAxis::v, sampler_state_.address_mode_v_);
}

void GenericOglTexture2d::set_anisotropy()
{
	const auto ogl_state = ogl_texture_manager_->ogl_state_get();
	const auto& device_features = ogl_state->get_device_features();

	OglRendererUtils::anisotropy_set_value(
		GL_TEXTURE_2D,
		device_features,
		sampler_state_.anisotropy_
	);
}

void GenericOglTexture2d::update_sampler_state(
	const RendererSamplerState& new_sampler_state)
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
		const auto ogl_state = ogl_texture_manager_->ogl_state_get();

		ogl_state->texture_2d_set(this);

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

void GenericOglTexture2d::set_sampler_state_defaults()
{
	sampler_state_.mag_filter_ = RendererFilterKind::nearest;
	set_mag_filter();

	sampler_state_.min_filter_ = RendererFilterKind::nearest;
	sampler_state_.mipmap_mode_ = RendererMipmapMode::none;
	set_min_filter();

	sampler_state_.address_mode_u_ = RendererAddressMode::clamp;
	set_address_mode_u();

	sampler_state_.address_mode_v_ = RendererAddressMode::clamp;
	set_address_mode_v();

	sampler_state_.anisotropy_ = RendererSampler::anisotropy_min;
	set_anisotropy();
}

//
// GenericOglTexture2d
// ==========================================================================


// =========================================================================
// OglTexture2dFactory
//

OglTexture2dUPtr OglTexture2dFactory::create(
	const OglTextureManagerPtr ogl_texture_manager,
	const RendererTexture2dCreateParam& param)
{
	return std::make_unique<GenericOglTexture2d>(ogl_texture_manager, param);
}

//
// OglTexture2dFactory
// =========================================================================


} // detail
} // bstone
