/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Software accelerated video (SW).

#include <algorithm>
#include <utility>

#include "3d_def.h"
#include "id_in.h"
#include "id_vh.h"
#include "id_vl.h"

#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_logger.h"
#include "bstone_single_pool_resource.h"
#include "bstone_span.h"
#include "bstone_sw_video.h"
#include "bstone_video.h"

#include "bstone_r3r_utils.h"

#include "bstone_sys_video_mgr.h"
#include "bstone_sys_window_mgr.h"

namespace bstone {

namespace {

class SwVideo final : public Video
{
public:
	SwVideo(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr);

	~SwVideo() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

	bool is_hardware() const noexcept override;
	StringView get_renderer_name() override;
	void clear_vga_buffer() override;
	void take_screenshot(
		int width,
		int height,
		int stride_rgb_888,
		ScreenshotBuffer&& src_pixels_rgb_888) override;
	void vsync_present() override;
	void present() override;

	void get_palette(int offset, int count, std::uint8_t* vga_palette) const override;
	void fill_palette(int r, int g, int b) noexcept override;
	void set_palette(int offset, int count, const std::uint8_t* vga_palette) override;

	void apply_widescreen() override;
	void apply_window_mode() override;
	void apply_filler_color_index() override;

	void fade_out(int start, int end, int red, int green, int blue, int steps) override;
	void fade_in(int start, int end, const std::uint8_t* palette, int steps) override;

	// HW
	//

	const Rgba8Palette& get_default_palette() const noexcept override;

	void enable_fizzle_fx(bool is_enabled) override;
	void enable_fizzle_fx_fading(bool is_fading) override;
	void set_fizzle_fx_color_index(int color_index) override;
	void set_fizzle_fx_ratio(float ratio) override;

	void clear_wall_render_list() noexcept override;
	void add_wall_render_item(int tile_x, int tile_y) override;

	void clear_pushwall_render_list() noexcept override;
	void add_pushwall_render_item(int tile_x, int tile_y) override;

	void clear_door_render_list() noexcept override;
	void add_door_render_item(int tile_x, int tile_y) override;

	void clear_static_render_list() noexcept override;
	void add_static_render_item(int bs_static_index) override;

	void clear_actor_render_list() noexcept override;
	void add_actor_render_item(int bs_actor_index) override;

	void on_load_level() override;
	void on_update_wall_switch(int x, int y) override;
	void on_move_pushwall() override;
	void on_step_pushwall(int old_x, int old_y) override;
	void on_pushwall_to_wall(int old_x, int old_y, int new_x, int new_y) override;
	void on_move_door(int door_index) override;
	void on_update_door_lock(int bs_door_index) override;
	void on_remove_static(const statobj_t& bs_static) override;
	void on_remove_actor(const objtype& bs_actor) override;

	void apply_vsync() override;
	void apply_msaa() override;
	void apply_texture_upscale() override;
	void apply_external_textures() override;
	void update_samplers() override;

	const R3rDeviceFeatures& get_device_features() const noexcept override;

	//
	// HW

private:
	static constexpr auto log_prefix = "[VIDSW] ";
	static const sys::Color opaque_black;

private:
	void initialize_video();
	void copy_texture_to_rendering_target(
		sys::Texture& texture,
		const sys::Rectangle* src_rect,
		const sys::Rectangle* dst_rect);
	void enable_texture_blending(sys::Texture& texture, bool is_enable);
	void fill_rects(Span<const sys::Rectangle> rects);
	void initialize_vga_buffer();
	void create_window();
	void initialize_renderer();
	void create_screen_texture();
	void create_ui_texture();
	void initialize_textures();
	void initialize_palette();
	void calculate_dimensions();
	void uninitialize_vga_buffer() noexcept;
	void update_palette_from_vga(int offset, int count);

private:
	sys::VideoMgr& video_mgr_;
	sys::WindowMgr& window_mgr_;
	sys::WindowUPtr window_{};
	sys::RendererUPtr renderer_{};
	std::string renderer_name_buffer_{};
	StringView renderer_name_{};
	sys::TextureUPtr screen_texture_{};
	sys::TextureUPtr ui_texture_{};
	VgaBuffer sw_vga_buffer_{};
	VgaPalette vga_palette_{};
	SdlPalette palette_{};
	sys::Rectangle ui_src_rect_{};
	sys::Rectangle ui_4x3_dst_rect_{};
	sys::Rectangle ui_wide_dst_rect_{};
	sys::Rectangle ui_top_src_rect_{};
	sys::Rectangle ui_4x3_top_dst_rect_{};
	sys::Rectangle ui_wide_top_dst_rect_{};
	sys::Rectangle ui_middle_src_rect_{};
	sys::Rectangle ui_4x3_middle_dst_rect_{};
	sys::Rectangle ui_wide_middle_dst_rect_{};
	sys::Rectangle ui_bottom_src_rect_{};
	sys::Rectangle ui_4x3_bottom_dst_rect_{};
	sys::Rectangle ui_wide_bottom_dst_rect_{};
	std::array<sys::Rectangle, 2> filler_ui_rects_{};
	std::array<sys::Rectangle, 4> filler_hud_rects_{};
	sys::Rectangle screen_dst_rect_{};
	sys::Color filler_color_{};


	// HW
	//

	Rgba8Palette default_palette_{};
	R3rDeviceFeatures device_features_{};

	//
	// HW
};

// ==========================================================================

using SwVideoPool = bstone::SinglePoolResource<SwVideo>;
SwVideoPool sw_video_pool{};

// ==========================================================================

constexpr sys::Color SwVideo::opaque_black = sys::Color{0, 0, 0, 0xFF};

SwVideo::SwVideo(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr)
try
	:
	video_mgr_{video_mgr},
	window_mgr_{window_mgr}
{
	initialize_video();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SwVideo::~SwVideo()
{
	uninitialize_vga_buffer();
}

void* SwVideo::operator new(std::size_t size)
try {
	return sw_video_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::operator delete(void* ptr)
{
	sw_video_pool.deallocate(ptr);
}

bool SwVideo::is_hardware() const noexcept
{
	return false;
}

StringView SwVideo::get_renderer_name()
{
	return renderer_name_;
}

void SwVideo::clear_vga_buffer()
{
	std::fill(sw_vga_buffer_.begin(), sw_vga_buffer_.end(), VgaBuffer::value_type{});
}

void SwVideo::apply_widescreen()
try {
	screen_texture_ = nullptr;

	uninitialize_vga_buffer();
	calculate_dimensions();
	initialize_vga_buffer();
	create_screen_texture();
	renderer_->set_viewport();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::take_screenshot(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888)
try {
	renderer_->read_pixels(sys::PixelFormat::r8g8b8, src_pixels_rgb_888.get(), stride_rgb_888);

	vid_schedule_save_screenshot_task(
		width,
		height,
		stride_rgb_888,
		std::move(src_pixels_rgb_888),
		false);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::vsync_present()
try {
	// Clear all
	//
	renderer_->set_draw_color(opaque_black);
	renderer_->clear();

	// Present
	//
	renderer_->present();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::present()
try {
	// HUD+3D stuff
	//
	if (vid_is_hud)
	{
		const auto texture_lock = screen_texture_->make_lock();

		const auto& src_pixels = sw_vga_buffer_;
		const auto src_pitch = vga_width;

		const auto dst_pixels = texture_lock->get_pixels<std::uint32_t*>();
		const auto dst_bytes_pitch = texture_lock->get_pitch();
		const auto dst_pitch = dst_bytes_pitch / 4;

		auto src_line_offset = 0;
		auto dst_line_offset = 0;

		for (auto y = decltype(vga_height){}; y < vga_height; ++y)
		{
			const auto src_line = &src_pixels[src_line_offset];
			auto dst_line = &dst_pixels[dst_line_offset];

			for (auto x = decltype(vga_width){}; x < vga_width; ++x)
			{
				dst_line[x] = palette_[src_line[x]];
			}

			src_line_offset += src_pitch;
			dst_line_offset += dst_pitch;
		}
	}

	// 2D stuff
	//
	{
		const auto texture_lock = ui_texture_->make_lock();

		const auto dst_pixels = texture_lock->get_pixels<std::uint32_t*>();
		const auto dst_bytes_pitch = texture_lock->get_pitch();
		const auto dst_pitch = dst_bytes_pitch / 4;

		auto src_line_offset = 0;
		auto dst_line_offset = 0;

		const auto alpha_0_mask = 0x00FFFFFFU;

		for (auto y = decltype(vga_ref_height){}; y < vga_ref_height; ++y)
		{
			auto dst_line = &dst_pixels[dst_line_offset];

			for (auto x = decltype(vga_ref_width){}; x < vga_ref_width; ++x)
			{
				const auto src_offset = src_line_offset + x;
				auto dst_color = palette_[vid_ui_buffer_[src_offset]];

				if (vid_is_hud)
				{
					if (!vid_mask_buffer_[src_offset])
					{
						dst_color &= alpha_0_mask;
					}
				}

				dst_line[x] = dst_color;
			}

			src_line_offset += vga_ref_width;
			dst_line_offset += dst_pitch;
		}
	}

	// Clear all
	//
	renderer_->set_draw_color(opaque_black);
	renderer_->clear();

	// Copy HUD+3D stuff
	//
	if (vid_is_hud)
	{
		copy_texture_to_rendering_target(*screen_texture_, nullptr, &screen_dst_rect_);
	}

	// Copy 2D stuff
	//
	if (vid_is_hud)
	{
		enable_texture_blending(*ui_texture_, true);
	}

	const auto is_stretched = vid_cfg_is_ui_stretched();
	const auto is_widescreen = vid_cfg_is_widescreen();

	const auto is_top_wide = is_stretched;
	const auto is_middle_wide = (vid_is_hud && is_widescreen) || (!vid_is_hud && is_stretched);
	const auto is_bottom_wide = is_stretched;

	if (false)
	{}
	else if (is_top_wide && is_middle_wide && is_bottom_wide)
	{
		copy_texture_to_rendering_target(*ui_texture_, nullptr, &ui_wide_dst_rect_);
	}
	else if (!is_top_wide && !is_middle_wide && !is_bottom_wide)
	{
		copy_texture_to_rendering_target(*ui_texture_, nullptr, &ui_4x3_dst_rect_);
	}
	else
	{
		const auto& dst_top_rect = is_top_wide ? ui_wide_top_dst_rect_ : ui_4x3_top_dst_rect_;
		const auto& dst_middle_rect = is_middle_wide ? ui_wide_middle_dst_rect_ : ui_4x3_middle_dst_rect_;
		const auto& dst_bottom_rect = is_bottom_wide ? ui_wide_bottom_dst_rect_ : ui_4x3_bottom_dst_rect_;

		copy_texture_to_rendering_target(*ui_texture_, &ui_top_src_rect_, &dst_top_rect);
		copy_texture_to_rendering_target(*ui_texture_, &ui_middle_src_rect_, &dst_middle_rect);
		copy_texture_to_rendering_target(*ui_texture_, &ui_bottom_src_rect_, &dst_bottom_rect);
	}

	if (vid_is_hud)
	{
		enable_texture_blending(*ui_texture_, false);
	}

	// Use filler if necessary
	//
	if (!vid_cfg_is_ui_stretched())
	{
		const auto is_hud = vid_is_hud;

		auto fill_color = opaque_black;

		if (!vid_is_movie)
		{
			fill_color = filler_color_;
		}

		renderer_->set_draw_color(fill_color);

		if (is_hud)
		{
			fill_rects(make_const_span(filler_hud_rects_.data(), static_cast<std::intptr_t>(filler_hud_rects_.size())));
		}
		else
		{
			fill_rects(make_const_span(filler_ui_rects_.data(), static_cast<std::intptr_t>(filler_ui_rects_.size())));
		}
	}

	if (vid_is_take_screenshot_scheduled)
	{
		vid_take_screenshot();
	}

	// Present
	//
	renderer_->present();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::get_palette(int offset, int count, std::uint8_t* vga_palette) const
try {
	BSTONE_ASSERT(offset >= 0);
	BSTONE_ASSERT(count >= 0);
	BSTONE_ASSERT((offset + count) <= 256);
	BSTONE_ASSERT(vga_palette);

	auto& dst_vga_palette = *reinterpret_cast<VgaPalette*>(vga_palette);

	std::copy_n(
		vga_palette_.cbegin() + offset,
		count,
		dst_vga_palette.begin()
	);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::fill_palette(int r, int g, int b) noexcept
{
	for (auto& vga_color : vga_palette_)
	{
		vga_color[0] = static_cast<std::uint8_t>(r);
		vga_color[1] = static_cast<std::uint8_t>(g);
		vga_color[2] = static_cast<std::uint8_t>(b);
	}

	update_palette_from_vga(0, 256);
}

void SwVideo::set_palette(int offset, int count, const std::uint8_t* vga_palette)
try {
	BSTONE_ASSERT(offset >= 0);
	BSTONE_ASSERT(count >= 0);
	BSTONE_ASSERT((offset + count) <= 256);
	BSTONE_ASSERT(vga_palette);

	const auto& src_vga_palette = *reinterpret_cast<const VgaPalette*>(vga_palette);

	std::copy_n(
		src_vga_palette.cbegin(),
		count,
		vga_palette_.begin() + offset);

	update_palette_from_vga(offset, count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::apply_window_mode()
try {
	calculate_dimensions();
	vid_initialize_vanilla_raycaster();

	auto param = R3rUtilsSetWindowModeParam{};
	param.is_native = vid_is_native_mode();
	param.size.width = vid_cfg_get_width();
	param.size.height = vid_cfg_get_height();
	R3rUtils::set_window_mode(*window_, param);

	vid_initialize_common();

	initialize_textures();
	initialize_vga_buffer();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::apply_filler_color_index()
try {
	filler_color_.r = static_cast<std::uint8_t>((255 * vgapal[(vid_cfg_get_filler_color_index() * 3) + 0]) / 63);
	filler_color_.g = static_cast<std::uint8_t>((255 * vgapal[(vid_cfg_get_filler_color_index() * 3) + 1]) / 63);
	filler_color_.b = static_cast<std::uint8_t>((255 * vgapal[(vid_cfg_get_filler_color_index() * 3) + 2]) / 63);
	filler_color_.a = 0xFF;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::fade_out(int start, int end, int red, int green, int blue, int steps)
try {
	if (!gp_no_fade_in_or_out())
	{
		auto palette1 = VgaPalette{};
		auto palette2 = VgaPalette{};

		get_palette(0, 256, &palette1[0][0]);

		std::copy_n(&palette1[0][0], 768, &palette2[0][0]);

		//
		// fade through intermediate frames
		//
		for (int i = 0; i < steps; ++i)
		{
			auto origptr = &palette1[start][0];
			auto newptr = &palette2[start][0];

			for (int j = start; j <= end; ++j)
			{
				auto orig = *origptr++;
				auto delta = red - orig;
				*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));

				orig = *origptr++;
				delta = green - orig;
				*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));

				orig = *origptr++;
				delta = blue - orig;
				*newptr++ = static_cast<std::uint8_t>(orig + ((delta * i) / steps));
			}

			set_palette(0, 256, &palette2[0][0]);

			present();

			if (!vid_has_vsync)
			{
				VL_WaitVBL(1);
			}
		}
	}

	//
	// final color
	//
	fill_palette(red, green, blue);

	present();

	if (!vid_has_vsync)
	{
		VL_WaitVBL(1);
	}

	screenfaded = true;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::fade_in(int start, int end, const std::uint8_t* palette, int steps)
try {
	if (!gp_no_fade_in_or_out())
	{
		auto palette1 = VgaPalette{};
		auto palette2 = VgaPalette{};

		VL_GetPalette(0, 256, &palette1[0][0]);

		palette2 = palette1;

		const auto& src_palette = *reinterpret_cast<const VgaPalette*>(palette);

		//
		// fade through intermediate frames
		//
		for (auto i = 0; i < steps; ++i)
		{
			for (auto j = start; j <= end; ++j)
			{
				for (auto k = 0; k < 3; ++k)
				{
					const auto delta = src_palette[j][k] - palette1[j][k];

					palette2[j][k] = static_cast<std::uint8_t>(palette1[j][k] + ((delta * i) / steps));
				}
			}

			VL_SetPalette(0, 256, &palette2[0][0]);

			present();

			if (!vid_has_vsync)
			{
				VL_WaitVBL(1);
			}
		}
	}

	//
	// final color
	//
	VL_SetPalette(0, 256, palette);

	present();

	if (!vid_has_vsync)
	{
		VL_WaitVBL(1);
	}

	screenfaded = false;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// HW
//

const Rgba8Palette& SwVideo::get_default_palette() const noexcept
{
	return default_palette_;
}

void SwVideo::enable_fizzle_fx(bool) {}

void SwVideo::enable_fizzle_fx_fading(bool) {}

void SwVideo::set_fizzle_fx_color_index(int) {}

void SwVideo::set_fizzle_fx_ratio(float) {}

void SwVideo::clear_wall_render_list() noexcept {}

void SwVideo::add_wall_render_item(int, int) {}

void SwVideo::clear_pushwall_render_list() noexcept {}

void SwVideo::add_pushwall_render_item(int, int) {}

void SwVideo::clear_door_render_list() noexcept {}

void SwVideo::add_door_render_item(int, int) {}

void SwVideo::clear_static_render_list() noexcept {}

void SwVideo::add_static_render_item(int) {}

void SwVideo::clear_actor_render_list() noexcept {}

void SwVideo::add_actor_render_item(int) {}

void SwVideo::on_load_level() {}

void SwVideo::on_update_wall_switch(int, int) {}

void SwVideo::on_move_pushwall() {}

void SwVideo::on_step_pushwall(int, int) {}

void SwVideo::on_pushwall_to_wall(int, int, int, int) {}

void SwVideo::on_move_door(int) {}

void SwVideo::on_update_door_lock(int) {}

void SwVideo::on_remove_static(const statobj_t&) {}

void SwVideo::on_remove_actor(const objtype&) {}

void SwVideo::apply_vsync() {}

void SwVideo::apply_msaa() {}

void SwVideo::apply_texture_upscale() {}

void SwVideo::apply_external_textures() {}

void SwVideo::update_samplers() {}

const R3rDeviceFeatures& SwVideo::get_device_features() const noexcept
{
	return device_features_;
}

//
// HW

void SwVideo::initialize_video()
try {
	vid_initialize_common();
	calculate_dimensions();

	vid_initialize_vanilla_raycaster();

	create_window();
	initialize_renderer();
	initialize_textures();
	initialize_palette();
	initialize_vga_buffer();
	vid_initialize_ui_buffer();

	renderer_name_buffer_.reserve(128);
	renderer_name_buffer_ += "sw (";
	const auto sys_renderer_name = get_renderer_name();

	renderer_name_buffer_.append(
		sys_renderer_name.get_data(),
		static_cast<std::size_t>(sys_renderer_name.get_size()));

	renderer_name_buffer_ += ')';

	renderer_name_ = StringView{
		renderer_name_buffer_.data(),
		static_cast<std::intptr_t>(renderer_name_buffer_.size())};

	const auto window_title = vid_get_window_title_for_renderer(renderer_name_);
	window_->set_title(window_title.c_str());
	window_->show(true);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::copy_texture_to_rendering_target(
	sys::Texture& texture,
	const sys::Rectangle* src_rect,
	const sys::Rectangle* dst_rect)
try {
	texture.copy(src_rect, dst_rect);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::enable_texture_blending(sys::Texture& texture, bool is_enable)
try {
	texture.set_blend_mode(is_enable ? sys::TextureBlendMode::blend : sys::TextureBlendMode::none);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::fill_rects(Span<const sys::Rectangle> rects)
try {
	renderer_->fill(rects);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::initialize_vga_buffer()
try {
	const auto area = 2 * vga_width * vga_height;

	sw_vga_buffer_ = VgaBuffer{};
	sw_vga_buffer_.resize(area);

	vga_memory = sw_vga_buffer_.data();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::create_window()
try {
	const auto is_native_mode = vid_is_native_mode();
	const auto title = vid_get_game_name_and_game_version_string();

	auto param = sys::WindowInitParam{};
	param.title = title.c_str();
	param.x = sys::WindowOffset::make_centered();
	param.y = sys::WindowOffset::make_centered();

	if (!is_native_mode && vid_cfg_is_positioned())
	{
		param.x = sys::WindowOffset{vid_cfg_get_x()};
		param.y = sys::WindowOffset{vid_cfg_get_y()};
	}

	param.width = vid_cfg_get_width();
	param.height = vid_cfg_get_height();

	param.is_visible = true;
	param.fullscreen_type = is_native_mode ? sys::WindowFullscreenType::fake : sys::WindowFullscreenType::none;
	window_ = window_mgr_.make_window(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::initialize_renderer()
try {
	auto renderer_param = sys::RendererInitParam{};
	renderer_param.is_vsync = vid_cfg_is_vsync();

	renderer_ = window_->make_renderer(renderer_param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::create_screen_texture()
try {
	auto param = sys::TextureInitParam{};
	param.pixel_format = sys::PixelFormat::b8g8r8a8;
	param.access = sys::TextureAccess::streaming;
	param.width = vga_width;
	param.height = vga_height;

	screen_texture_ = renderer_->make_texture(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::create_ui_texture()
try {
	auto param = sys::TextureInitParam{};
	param.pixel_format = sys::PixelFormat::b8g8r8a8;
	param.access = sys::TextureAccess::streaming;
	param.width = vga_ref_width;
	param.height = vga_ref_height;

	ui_texture_ = renderer_->make_texture(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::initialize_textures()
try {
	create_screen_texture();
	create_ui_texture();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SwVideo::initialize_palette()
{
	vga_palette_ = {};
	palette_ = {};
}

void SwVideo::calculate_dimensions()
{
	auto src_param = vid_create_screen_size_param();
	vid_calculate_window_elements_dimensions(src_param, vid_layout_);
	vid_calculate_vga_dimensions();


	// UI whole rect
	//
	ui_src_rect_ = sys::Rectangle
	{
		0,
		0,
		vga_ref_width,
		vga_ref_height,
	};

	ui_4x3_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_width_4x3,
		vid_layout_.screen_height,
	};

	// UI stretched rect
	//
	ui_wide_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_width,
		vid_layout_.screen_height,
	};

	// UI top rect
	//
	ui_top_src_rect_ = sys::Rectangle
	{
		0,
		0,
		vga_ref_width,
		ref_top_bar_height,
	};

	ui_4x3_top_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_width_4x3,
		vid_layout_.screen_top_filler_height,
	};

	ui_wide_top_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_width,
		vid_layout_.screen_top_filler_height,
	};

	// UI middle rect (stretched to full width)
	//
	ui_middle_src_rect_ = sys::Rectangle
	{
		0,
		ref_view_top_y,
		vga_ref_width,
		ref_view_height,
	};

	ui_4x3_middle_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_top_filler_height,
		vid_layout_.screen_width_4x3,
		vid_layout_.screen_height - vid_layout_.screen_top_filler_height - vid_layout_.screen_bottom_filler_height,
	};

	ui_wide_middle_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_top_filler_height,
		vid_layout_.screen_width,
		vid_layout_.screen_height - vid_layout_.screen_top_filler_height - vid_layout_.screen_bottom_filler_height,
	};

	// UI bottom rect
	//
	ui_bottom_src_rect_ = sys::Rectangle
	{
		0,
		ref_view_bottom_y + 1,
		vga_ref_width,
		ref_bottom_bar_height,
	};

	ui_4x3_bottom_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_height - vid_layout_.screen_bottom_filler_height,
		vid_layout_.screen_width_4x3,
		vid_layout_.screen_bottom_filler_height,
	};

	ui_wide_bottom_dst_rect_ = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_height - vid_layout_.screen_bottom_filler_height,
		vid_layout_.screen_width,
		vid_layout_.screen_bottom_filler_height,
	};

	// UI left bar
	filler_ui_rects_[0] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_height,
	};

	// UI right bar
	filler_ui_rects_[1] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_width - vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_height,
	};

	// HUD upper left rect
	filler_hud_rects_[0] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_top_filler_height,
	};

	// HUD upper right rect
	filler_hud_rects_[1] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_width - vid_layout_.screen_right_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_right_filler_width,
		vid_layout_.screen_top_filler_height,
	};

	// HUD lower left rect
	filler_hud_rects_[2] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_height - vid_layout_.screen_bottom_filler_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_bottom_filler_height,
	};

	// HUD lower right rect
	filler_hud_rects_[3] = sys::Rectangle
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_width - vid_layout_.screen_right_filler_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_height - vid_layout_.screen_bottom_filler_height,
		vid_layout_.screen_right_filler_width,
		vid_layout_.screen_bottom_filler_height,
	};

	apply_filler_color_index();

	// Screen destination rect.

	const auto screen_left = (
		vid_cfg_is_widescreen() ?
		0 :
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width);

	const auto screen_top = vid_layout_.window_viewport_top_height;
	const auto screen_width = (vid_cfg_is_widescreen() ? vid_layout_.screen_width : vid_layout_.screen_width_4x3);
	const auto screen_height = vid_layout_.screen_height;

	screen_dst_rect_ = sys::Rectangle
	{
		screen_left,
		screen_top,
		screen_width,
		screen_height,
	};
}

void SwVideo::uninitialize_vga_buffer() noexcept
{
	sw_vga_buffer_.clear();
	vga_memory = nullptr;
}

void SwVideo::update_palette_from_vga(int offset, int count)
{
	for (auto i = 0; i < count; ++i)
	{
		const auto& vga_color = vga_palette_[offset + i];
		auto& color = palette_[offset + i];

		color =
			0xFF000000U |
			((((255U * vga_color[0]) / 63U) & 0xFFU) << 16) |
			((((255U * vga_color[1]) / 63U) & 0xFFU) << 8) |
			(((255U * vga_color[2]) / 63U) & 0xFFU);
	}
}

} // namespace

// ==========================================================================

VideoUPtr make_sw_video(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr)
{
	return std::make_unique<SwVideo>(video_mgr, window_mgr);
}

} // bstone
