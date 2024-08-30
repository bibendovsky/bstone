/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// 3D Renderer: Utils

#include <algorithm>
#include <unordered_set>

#include "bstone_exception.h"
#include "bstone_sprite.h"
#include "bstone_r3r_limits.h"
#include "bstone_r3r_utils.h"

namespace bstone {

int R3rUtils::find_nearest_pot_value(int value) noexcept
{
	for (auto i = 0; i < 32; ++i)
	{
		const auto new_value = 1 << i;

		if (new_value >= value)
		{
			return new_value;
		}
	}

	return 0;
}

bool R3rUtils::is_pot_value(int value) noexcept
{
	const auto nearest_value = find_nearest_pot_value(value);
	return nearest_value == value;
}

int R3rUtils::calculate_mipmap_count(int width, int height)
try {
	//
	// mipmap_count = [log2(max(width, height))] + 1
	//

	if (width <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Width out of range.");
	}

	if (height <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Height out of range.");
	}

	auto log_2 = 0;
	auto max_size = std::max(width, height);

	while (max_size > 1)
	{
		log_2 += 1;
		max_size /= 2;
	}

	return log_2 + 1;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

sys::WindowUPtr R3rUtils::create_window(
	const R3rUtilsCreateWindowParam& param,
	sys::WindowMgr& window_mgr)
try {
	const auto gl_attributes = create_window_make_gl_context_attributes(param);

	auto window_param = sys::WindowInitParam{};
	window_param.x = sys::WindowOffset::make_centered();
	window_param.y = sys::WindowOffset::make_centered();
	window_param.width = 320;
	window_param.height = 240;
	window_param.is_visible = false;
	window_param.fullscreen_type = sys::WindowFullscreenType::none;

	switch (param.renderer_type)
	{
		case R3rType::gl_2_0:
		case R3rType::gl_3_2_core:
		case R3rType::gles_2_0:
			window_param.renderer_type = sys::WindowRendererType::open_gl;
			window_param.gl_attributes = &gl_attributes;
			break;

		default:
			break;
	}

	return window_mgr.make_window(window_param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::set_window_mode(sys::Window& window, const R3rUtilsSetWindowModeParam& param)
try {
	if (param.size.height <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Height out of range.");
	}

	if (param.size.width <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Width out of range.");
	}

	//
	const auto is_current_native = window.is_fake_fullscreen();
	const auto is_native_changed = (is_current_native != param.is_native);

	//
	const auto current_size = window.get_size();

	if (current_size.width <= 0 || current_size.height <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get current window size.");
	}

	const auto is_size_changed =
		current_size.width != param.size.width ||
		current_size.height != param.size.height;

	//
	if (!is_native_changed && !is_size_changed)
	{
		return;
	}

	if (is_native_changed && !param.is_native)
	{
		window.set_fake_fullscreen(false);
	}

	if (is_size_changed)
	{
		window.set_size(sys::WindowSize{param.size.width, param.size.height});

		if (param.is_positioned)
		{
			const auto x = std::max(param.position.x.get(), 0);
			const auto y = std::max(param.position.y.get(), 0);
			window.set_position(sys::WindowPosition{sys::WindowOffset{x}, sys::WindowOffset{y}});
		}
		else
		{
			window.center();
		}
	}

	if (is_native_changed && param.is_native)
	{
		window.set_fake_fullscreen(true);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::validate_initialize_param(const R3rInitParam& param)
try {
	switch (param.renderer_type)
	{
	case R3rType::gl_2_0:
	case R3rType::gl_3_2_core:
	case R3rType::gles_2_0:
		break;

	default:
		BSTONE_THROW_STATIC_SOURCE("Unsupported renderer type.");
	}

	switch (param.aa_type)
	{
		case R3rAaType::none:
		case R3rAaType::ms:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Invalid anti-aliasing type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::validate_buffer_create_param(const R3rBufferInitParam& param)
try {
	switch (param.usage_type)
	{
		case R3rBufferUsageType::draw_streaming:
		case R3rBufferUsageType::draw_static:
		case R3rBufferUsageType::draw_dynamic:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Invalid usage type.");
	}

	if (param.size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid size.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::validate_buffer_update_param(const R3rUpdateBufferParam& param)
try {
	if (param.offset < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid offset.");
	}

	if (param.size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid count.");
	}

	if (param.data == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null indices.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::vertex_input_validate_format(const R3rVertexAttribFormat attribute_format)
try {
	switch (attribute_format)
	{
		case R3rVertexAttribFormat::rgba_8_unorm:
		case R3rVertexAttribFormat::rg_32_sfloat:
		case R3rVertexAttribFormat::rgb_32_sfloat:
			return;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported vertex input attribute format.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::vertex_input_validate_param(
	int max_locations,
	const R3rCreateVertexInputParam& param)
try {
	if (max_locations < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Maximum location count out of range.");
	}

	if (param.index_buffer)
	{
		if (param.index_buffer->get_type() != R3rBufferType::index)
		{
			BSTONE_THROW_STATIC_SOURCE("Non-index buffer.");
		}
	}

	const auto& attribute_descriptions = param.attrib_descrs;

	using UsedLocations = std::unordered_set<int>;
	auto used_locations = UsedLocations{};
	used_locations.reserve(max_locations);

	for (const auto& attribute_description : attribute_descriptions)
	{
		if (attribute_description.is_default)
		{
			continue;
		}

		// Location.
		//
		const auto location = attribute_description.location;

		if (location < 0 || location >= max_locations)
		{
			BSTONE_THROW_STATIC_SOURCE("Location out of range.");
		}

		const auto has_location = (used_locations.find(location) != used_locations.cend());

		if (has_location)
		{
			BSTONE_THROW_STATIC_SOURCE("Duplicate location.");
		}

		used_locations.insert(location);


		// Format.
		//
		vertex_input_validate_format(attribute_description.format);


		// Vertex buffer.
		//
		if (!attribute_description.vertex_buffer)
		{
			BSTONE_THROW_STATIC_SOURCE("Null vertex buffer.");
		}

		if (attribute_description.vertex_buffer->get_type() != R3rBufferType::vertex)
		{
			BSTONE_THROW_STATIC_SOURCE("Non-vertex buffer.");
		}


		// Offset.
		//
		if (attribute_description.offset < 0)
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid offset.");
		}


		// Stride.
		//
		if (attribute_description.stride < 0)
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid stride.");
		}
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::indexed_to_rgba_8(const IndexedToRgba8Param& param)
try {
	if (param.width <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Width out of range.");
	}

	if (param.height <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Height out of range.");
	}

	if (!param.indexed_pixels)
	{
		BSTONE_THROW_STATIC_SOURCE("Null indexed pixels.");
	}

	if (param.rgba_8_buffer->size() < static_cast<std::size_t>((param.width * param.height)))
	{
		BSTONE_THROW_STATIC_SOURCE("Bitmap buffer too small.");
	}

	const auto is_column_major = param.indexed_is_column_major;
	const auto has_alphas = (param.indexed_alphas != nullptr);

	if (!is_column_major && !has_alphas)
	{
		indexed_to_rgba_8_rm_na(param);
	}
	else if (!is_column_major && has_alphas)
	{
		indexed_to_rgba_8_rm_ha(param);
	}
	else if (is_column_major && !has_alphas)
	{
		indexed_to_rgba_8_cm_na(param);
	}
	else if (is_column_major && has_alphas)
	{
		indexed_to_rgba_8_cm_ha(param);
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE("Unsupported values combination.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::indexed_npot_to_rgba_8_pot(const IndexedToRgba8Param& param)
try {
	if (param.width <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Width out of range.");
	}

	if (param.height <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Height out of range.");
	}

	if (param.actual_width <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Actual width out of range.");
	}

	if (param.actual_height <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Actual height out of range.");
	}

	if (!param.indexed_pixels)
	{
		BSTONE_THROW_STATIC_SOURCE("Null indexed pixels.");
	}

	if (param.rgba_8_buffer->size() < static_cast<std::size_t>((param.actual_width * param.actual_height)))
	{
		BSTONE_THROW_STATIC_SOURCE("Indexed bitmap buffer too small.");
	}

	const auto is_column_major = param.indexed_is_column_major;
	const auto has_alphas = (param.indexed_alphas != nullptr);

	if (!is_column_major && !has_alphas)
	{
		indexed_npot_to_rgba_8_rm_na(param);
	}
	else if (!is_column_major && has_alphas)
	{
		indexed_npot_to_rgba_8_rm_ha(param);
	}
	else if (is_column_major && !has_alphas)
	{
		indexed_npot_to_rgba_8_cm_na(param);
	}
	else if (is_column_major && has_alphas)
	{
		indexed_npot_to_rgba_8_cm_ha(param);
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE("Unsupported values combination.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::indexed_to_rgba_8_pot(const IndexedToRgba8Param& param)
try {
	const auto is_npot =
		param.width != param.actual_width ||
		param.height != param.actual_height;

	if (!is_npot)
	{
		indexed_to_rgba_8(param);
	}
	else if (is_npot)
	{
		indexed_npot_to_rgba_8_pot(param);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::indexed_sprite_to_rgba_8_pot(
	const Sprite& indexed_sprite,
	const Rgba8Palette& indexed_palette,
	Rgba8Buffer& texture_buffer)
try {
	if (!indexed_sprite.is_initialized())
	{
		BSTONE_THROW_STATIC_SOURCE("Indexed sprite not initialized.");
	}

	const auto left = indexed_sprite.get_left();
	const auto right = indexed_sprite.get_right();
	const auto top = indexed_sprite.get_top();
	const auto bottom = indexed_sprite.get_bottom();

	for (auto w = 0; w < Sprite::dimension; ++w)
	{
		const std::int16_t* column = nullptr;

		if (w >= left && w <= right)
		{
			column = indexed_sprite.get_column(w - left);
		}

		auto dst_pixels = &texture_buffer[w];

		for (auto h = 0; h < Sprite::dimension; ++h)
		{
			auto& dst_pixel = *dst_pixels;

			if (column && h >= top && h <= bottom)
			{
				const auto src_index = h - top;
				const auto src_pixel = column[src_index];

				if (src_pixel < 0)
				{
					dst_pixel.reset();
				}
				else
				{
					dst_pixel = indexed_palette[src_pixel];
				}
			}
			else
			{
				dst_pixel.reset();
			}

			dst_pixels += Sprite::dimension;
		}
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::rgba_8_npot_to_rgba_8_pot(
	int width,
	int height,
	int actual_width,
	int actual_height,
	const Rgba8* rgba_8_pixels,
	Rgba8Buffer& texture_buffer)
try {
	if (width <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Width out of range.");
	}

	if (height <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Height out of range.");
	}

	if (actual_width <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Actual width out of range.");
	}

	if (actual_height <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Actual height out of range.");
	}

	if (!rgba_8_pixels)
	{
		BSTONE_THROW_STATIC_SOURCE("Null RGBA pixels.");
	}

	const auto src_du_f = static_cast<double>(width) / static_cast<double>(actual_width);
	const auto src_dv_f = static_cast<double>(height) / static_cast<double>(actual_height);

	auto src_v_d = 0.0;
	auto dst_index = 0;

	for (auto h = 0; h < actual_height; ++h)
	{
		const auto src_v = static_cast<int>(src_v_d);
		auto src_u_d = 0.0;

		for (int w = 0; w < actual_width; ++w)
		{
			const auto src_u = static_cast<int>(src_u_d);
			const auto src_index = (src_v * width) + src_u;
			texture_buffer[dst_index] = rgba_8_pixels[src_index];
			++dst_index;
			src_u_d += src_du_f;
		}

		src_v_d += src_dv_f;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rUtils::build_mipmap(
	int previous_width,
	int previous_height,
	Rgba8CPtr src_colors,
	Rgba8Ptr dst_colors)
try {
	if (previous_width <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Previous width out of range.");
	}

	if (previous_height <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Previous height out of range.");
	}

	if (previous_width == 1 && previous_height == 1)
	{
		BSTONE_THROW_STATIC_SOURCE("No more mipmap to build.");
	}

	if (!src_colors)
	{
		BSTONE_THROW_STATIC_SOURCE("Null source colors.");
	}

	if (dst_colors == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null target colors.");
	}

	if (previous_width == 1)
	{
		build_mipmap_1(previous_height, src_colors, dst_colors);
		return;
	}

	if (previous_height == 1)
	{
		build_mipmap_1(previous_width, src_colors, dst_colors);
		return;
	}

	const auto width = previous_width / 2;
	const auto height = previous_height / 2;
	auto dst_colors_0 = dst_colors;
	auto src_colors_0 = src_colors;
	auto src_colors_1 = src_colors_0 + previous_width;

	for (auto h = 0; h < height; ++h)
	{
		for (auto w = 0; w < width; ++w)
		{
			*dst_colors_0++ = average_pa(
				src_colors_0[0],
				src_colors_0[1],
				src_colors_1[0],
				src_colors_1[1]);

			src_colors_0 += 2;
			src_colors_1 += 2;
		}

		src_colors_0 += previous_width;
		src_colors_1 += previous_width;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

sys::GlContextAttributes R3rUtils::create_window_make_gl_context_attributes(
	const R3rUtilsCreateWindowParam& param)
try {
	auto gl_attributes = sys::GlContextAttributes{};
	gl_attributes.is_accelerated = true;

	switch (param.aa_type)
	{
		case R3rAaType::ms:
			if (param.aa_value >= R3rLimits::min_aa_on())
			{
				gl_attributes.multisample_buffer_count = 1;
				gl_attributes.multisample_sample_count = param.aa_value;
			}

			break;


		case R3rAaType::none:
		default:
			break;
	}

	gl_attributes.red_bit_count = 8;
	gl_attributes.green_bit_count = 8;
	gl_attributes.blue_bit_count = 8;

	if (!param.is_default_depth_buffer_disabled)
	{
		gl_attributes.depth_bit_count = 16;
	}

	switch (param.renderer_type)
	{
		case R3rType::gl_2_0:
			gl_attributes.profile = sys::GlContextProfile::compatibility;
			gl_attributes.major_version = 2;
			gl_attributes.minor_version = 0;
			break;

		case R3rType::gl_3_2_core:
			gl_attributes.profile = sys::GlContextProfile::core;
			gl_attributes.major_version = 3;
			gl_attributes.minor_version = 2;
			break;

		case R3rType::gles_2_0:
			gl_attributes.profile = sys::GlContextProfile::es;
			gl_attributes.major_version = 2;
			gl_attributes.minor_version = 0;
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported 3D renderer type.");
	}

	return gl_attributes;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Indexed (row major, has no alpha) -> RGBA
void R3rUtils::indexed_to_rgba_8_rm_na(const IndexedToRgba8Param& param) noexcept
{
	auto src_pixels = param.indexed_pixels;
	const auto& src_palette = *param.indexed_palette;
	auto dst_pixels = param.rgba_8_buffer->data();
	const auto area = param.width * param.height;

	for (auto i = 0; i < area; ++i)
	{
		*dst_pixels++ = src_palette[*src_pixels++];
	}
}

// Indexed (row major, has alpha) -> RGBA
void R3rUtils::indexed_to_rgba_8_rm_ha(const IndexedToRgba8Param& param) noexcept
{
	auto src_pixels = param.indexed_pixels;
	const auto& src_palette = *param.indexed_palette;
	auto dst_pixels = param.rgba_8_buffer->data();
	const auto area = param.width * param.height;

	for (auto i = 0; i < area; ++i)
	{
		const auto src_pixel = *src_pixels++;
		auto& dst_pixel = *dst_pixels++;

		if (param.indexed_alphas[i])
		{
			dst_pixel = src_palette[src_pixel];
		}
		else
		{
			dst_pixel.reset();
		}
	}
}

// Indexed (column major, has no alpha) -> RGBA
void R3rUtils::indexed_to_rgba_8_cm_na(const IndexedToRgba8Param& param) noexcept
{
	const auto& src_palette = *param.indexed_palette;
	auto dst_pixels = param.rgba_8_buffer->data();

	for (auto src_y = 0; src_y < param.height; ++src_y)
	{
		auto src_index = src_y;

		for (auto src_x = 0; src_x < param.width; ++src_x)
		{
			*dst_pixels++ = src_palette[param.indexed_pixels[src_index]];
			src_index += param.height;
		}
	}
}

// Indexed (column major, has alpha) -> RGBA
void R3rUtils::indexed_to_rgba_8_cm_ha(const IndexedToRgba8Param& param) noexcept
{
	const auto& src_palette = *param.indexed_palette;
	auto dst_pixels = param.rgba_8_buffer->data();

	for (auto src_y = 0; src_y < param.height; ++src_y)
	{
		auto src_index = src_y;

		for (auto src_x = 0; src_x < param.width; ++src_x)
		{
			auto& dst_pixel = *dst_pixels++;

			if (param.indexed_alphas[src_index])
			{
				dst_pixel = src_palette[param.indexed_pixels[src_index]];
			}
			else
			{
				dst_pixel.reset();
			}

			src_index += param.height;
		}
	}
}

// Indexed (row major, has no alpha) -> RGBA POT
void R3rUtils::indexed_npot_to_rgba_8_rm_na(const IndexedToRgba8Param& param) noexcept
{
	const auto src_du_f = static_cast<double>(param.width) / static_cast<double>(param.actual_width);
	const auto src_dv_f = static_cast<double>(param.height) / static_cast<double>(param.actual_height);
	const auto& src_palette = *param.indexed_palette;
	auto dst_colors = param.rgba_8_buffer->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = Rgba8{};

	for (auto h = 0; h < param.actual_height; ++h)
	{
		const auto src_v = static_cast<int>(src_v_f);
		const auto src_base_index = src_v * param.width;

		auto src_u_f = 0.0;

		for (auto w = 0; w < param.actual_width; ++w)
		{
			const auto src_u = static_cast<int>(src_u_f);
			const auto src_index = src_base_index + src_u;

			if (src_index != cached_src_index)
			{
				cached_src_index = src_index;
				cached_color = src_palette[param.indexed_pixels[src_index]];
			}

			*dst_colors++ = cached_color;

			src_u_f += src_du_f;
		}

		src_v_f += src_dv_f;
	}
}

// Indexed (row major, has alpha) -> RGBA POT
void R3rUtils::indexed_npot_to_rgba_8_rm_ha(const IndexedToRgba8Param& param) noexcept
{
	const auto src_du_f = static_cast<double>(param.width) / static_cast<double>(param.actual_width);
	const auto src_dv_f = static_cast<double>(param.height) / static_cast<double>(param.actual_height);
	const auto& src_palette = *param.indexed_palette;
	auto dst_colors = param.rgba_8_buffer->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = Rgba8{};

	for (auto h = 0; h < param.actual_height; ++h)
	{
		const auto src_v = static_cast<int>(src_v_f);
		const auto src_base_index = src_v * param.width;

		auto src_u_f = 0.0;

		for (auto w = 0; w < param.actual_width; ++w)
		{
			const auto src_u = static_cast<int>(src_u_f);
			const auto src_index = src_base_index + src_u;

			auto& dst_pixel = *dst_colors++;

			if (param.indexed_alphas[src_index])
			{
				if (src_index != cached_src_index)
				{
					cached_src_index = src_index;
					cached_color = src_palette[param.indexed_pixels[src_index]];
				}

				dst_pixel = cached_color;
			}
			else
			{
				dst_pixel.reset();
			}

			src_u_f += src_du_f;
		}

		src_v_f += src_dv_f;
	}
}

// Indexed (column major, has no alpha) -> RGBA POT
void R3rUtils::indexed_npot_to_rgba_8_cm_na(const IndexedToRgba8Param& param) noexcept
{
	const auto src_du_f = static_cast<double>(param.width) / static_cast<double>(param.actual_width);
	const auto src_dv_f = static_cast<double>(param.height) / static_cast<double>(param.actual_height);
	const auto& src_palette = *param.indexed_palette;
	auto dst_colors = param.rgba_8_buffer->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = Rgba8{};

	for (auto h = 0; h < param.actual_height; ++h)
	{
		const auto src_v = static_cast<int>(src_v_f);
		auto src_u_f = 0.0;

		for (auto w = 0; w < param.actual_width; ++w)
		{
			const auto src_u = static_cast<int>(src_u_f);
			const auto src_index = (src_u * param.height) + src_v;

			if (src_index != cached_src_index)
			{
				cached_src_index = src_index;
				cached_color = src_palette[param.indexed_pixels[src_index]];
			}

			*dst_colors++ = cached_color;
			src_u_f += src_du_f;
		}

		src_v_f += src_dv_f;
	}
}

// Indexed (column major, has alpha) -> RGBA POT
void R3rUtils::indexed_npot_to_rgba_8_cm_ha(const IndexedToRgba8Param& param) noexcept
{
	const auto src_du_f = static_cast<double>(param.width) / static_cast<double>(param.actual_width);
	const auto src_dv_f = static_cast<double>(param.height) / static_cast<double>(param.actual_height);
	const auto& src_palette = *param.indexed_palette;
	auto dst_colors = param.rgba_8_buffer->data();

	auto src_v_f = 0.0;
	auto cached_src_index = -1;
	auto cached_color = Rgba8{};

	for (auto h = 0; h < param.actual_height; ++h)
	{
		const auto src_v = static_cast<int>(src_v_f);
		auto src_u_f = 0.0;

		for (auto w = 0; w < param.actual_width; ++w)
		{
			const auto src_u = static_cast<int>(src_u_f);
			const auto src_index = (src_u * param.height) + src_v;
			auto& dst_pixel = *dst_colors++;

			if (param.indexed_alphas[src_index])
			{
				if (src_index != cached_src_index)
				{
					cached_src_index = src_index;
					cached_color = src_palette[param.indexed_pixels[src_index]];
				}

				dst_pixel = cached_color;
			}
			else
			{
				dst_pixel.reset();
			}

			src_u_f += src_du_f;
		}

		src_v_f += src_dv_f;
	}
}

void R3rUtils::build_mipmap_1(
	int previous_dimension,
	Rgba8CPtr src_colors,
	Rgba8Ptr dst_colors) noexcept
{
	const auto dimension = previous_dimension / 2;
	auto src_colors_0 = src_colors;
	auto dst_colors_0 = dst_colors;

	for (auto i = 0; i < dimension; ++i)
	{
		*dst_colors_0++ = average_pa(src_colors_0[0], src_colors_0[1]);
		src_colors_0 += 2;
	}
}

Rgba8 R3rUtils::average_pa(Rgba8 color_0, Rgba8 color_1) noexcept
{
	constexpr auto color_count = 2;
	constexpr auto denominator = 255 * color_count * color_count;

	const auto a_sum = color_0.a_ + color_1.a_;
	const auto r_sum = color_0.r_ + color_1.r_;
	const auto g_sum = color_0.g_ + color_1.g_;
	const auto b_sum = color_0.b_ + color_1.b_;

	return Rgba8
	{
		static_cast<std::uint8_t>((r_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((g_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((b_sum * a_sum) / denominator),
		static_cast<std::uint8_t>(a_sum / color_count),
	};
}

Rgba8 R3rUtils::average_pa(Rgba8 color_0, Rgba8 color_1, Rgba8 color_2, Rgba8 color_3) noexcept
{
	constexpr auto color_count = 4;
	constexpr auto denominator = 255 * color_count * color_count;

	const auto a_sum = color_0.a_ + color_1.a_ + color_2.a_ + color_3.a_;
	const auto r_sum = color_0.r_ + color_1.r_ + color_2.r_ + color_3.r_;
	const auto g_sum = color_0.g_ + color_1.g_ + color_2.g_ + color_3.g_;
	const auto b_sum = color_0.b_ + color_1.b_ + color_2.b_ + color_3.b_;

	return Rgba8
	{
		static_cast<std::uint8_t>((r_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((g_sum * a_sum) / denominator),
		static_cast<std::uint8_t>((b_sum * a_sum) / denominator),
		static_cast<std::uint8_t>(a_sum / color_count),
	};
}

} // namespace bstone
