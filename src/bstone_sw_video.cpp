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
// Software accelerated video (SW).
//


#include "bstone_sw_video.h"

#include <cassert>

#include "3d_def.h"
#include "id_in.h"
#include "id_vh.h"
#include "id_vl.h"

#include "bstone_exception.h"
#include "bstone_logger.h"
#include "bstone_sdl_exception.h"
#include "bstone_sdl_texture_lock.h"
#include "bstone_sdl_utils.h"
#include "bstone_video.h"

#include "bstone_detail_ren_3d_utils.h"


namespace bstone
{


class SwVideoException :
	public bstone::Exception
{
public:
	explicit SwVideoException(
		const char* message) noexcept
		:
		Exception{"SW_VIDEO", message}
	{
	}
}; // SwVideoException


class SwVideo :
	public Video
{
public:
	SwVideo();

	~SwVideo() override;


	bool is_hardware() const noexcept override;

	std::string get_renderer_name() override;

	void clear_vga_buffer() override;

	void take_screenshot(
		int width,
		int height,
		int stride_rgb_888,
		::ScreenshotBuffer&& src_pixels_rgb_888) override;

	void present() override;


	void get_palette(
		int offset,
		int count,
		std::uint8_t* vga_palette) const override;

	void fill_palette(
		int r,
		int g,
		int b) noexcept override;

	void set_palette(
		int offset,
		int count,
		const std::uint8_t* vga_palette) override;

	void apply_widescreen() override;

	void apply_window_mode() override;

	void apply_filler_color_index() override;

	void fade_out(
		int start,
		int end,
		int red,
		int green,
		int blue,
		int steps) override;

	void fade_in(
		int start,
		int end,
		const std::uint8_t* palette,
		int steps) override;


	// HW
	//

	const bstone::Rgba8Palette& get_default_palette() const noexcept override;

	void enable_fizzle_fx(
		bool is_enabled) override;

	void enable_fizzle_fx_fading(
		bool is_fading) override;

	void set_fizzle_fx_color_index(
		int color_index) override;

	void set_fizzle_fx_ratio(
		float ratio) override;

	void clear_wall_render_list() noexcept override;

	void add_wall_render_item(
		int tile_x,
		int tile_y) override;

	void clear_pushwall_render_list() noexcept override;

	void add_pushwall_render_item(
		int tile_x,
		int tile_y) override;

	void clear_door_render_list() noexcept override;

	void add_door_render_item(
		int tile_x,
		int tile_y) override;

	void clear_static_render_list() noexcept override;

	void add_static_render_item(
		int bs_static_index) override;

	void clear_actor_render_list() noexcept override;

	void add_actor_render_item(
		int bs_actor_index) override;

	void on_load_level() override;

	void on_update_wall_switch(
		int x,
		int y) override;

	void on_move_pushwall() override;

	void on_step_pushwall(
		int old_x,
		int old_y) override;

	void on_pushwall_to_wall(
		int old_x,
		int old_y,
		int new_x,
		int new_y) override;

	void on_move_door(
		int door_index) override;

	void on_update_door_lock(
		int bs_door_index) override;

	void on_remove_static(
		const statobj_t& bs_static) override;

	void on_remove_actor(
		const objtype& bs_actor) override;

	void apply_vsync() override;

	void apply_msaa() override;

	void apply_texture_upscale() override;

	void apply_external_textures() override;

	void update_samplers() override;

	const bstone::Ren3dDeviceFeatures& get_device_features() const noexcept override;

	//
	// HW


private:
	static constexpr auto log_prefix = "[VIDSW] ";


	static void log(
		bstone::LoggerMessageKind message_type,
		const std::string& message);

	static void log(
		const std::string& message);

	static void log_warning(
		const std::string& message);

	static void log_error(
		const std::string& message);


	void initialize_video();

	void set_draw_color(
		std::uint8_t r,
		std::uint8_t g,
		std::uint8_t b);

	void clear_rendering_target();

	void copy_texture_to_rendering_target(
		::SDL_Texture* sdl_texture,
		const ::SDL_Rect* src_rect,
		const ::SDL_Rect* dst_rect);

	void enable_texture_blending(
		::SDL_Texture* sdl_texture,
		bool is_enable);

	void fill_rects(
		::SDL_Rect* rects,
		int rect_count);

	void initialize_vga_buffer();

	void create_window();

	void initialize_renderer();

	void create_screen_texture();

	void create_ui_texture();

	void initialize_textures();

	void update_viewport();

	void initialize_palette();

	void calculate_dimensions();

	void uninitialize_vga_buffer();

	void update_palette_from_vga(
		int offset,
		int count);


private:
	bstone::SdlWindowUPtr window_{};
	bstone::SdlRendererUPtr renderer_{};
	bstone::SdlPixelFormatUPtr texture_pixel_format_{};
	bstone::SdlTextureUPtr screen_texture_{};
	bstone::SdlTextureUPtr ui_texture_{};
	::VgaBuffer sw_vga_buffer_{};
	::VgaPalette vga_palette_{};
	::SdlPalette palette_{};
	::SDL_Rect ui_whole_src_rect_{};
	::SDL_Rect ui_whole_dst_rect_{};
	::SDL_Rect ui_stretched_dst_rect_{};
	::SDL_Rect ui_top_src_rect_{};
	::SDL_Rect ui_top_dst_rect_{};
	::SDL_Rect ui_wide_middle_src_rect_{};
	::SDL_Rect ui_wide_middle_dst_rect_{};
	::SDL_Rect ui_bottom_src_rect_{};
	::SDL_Rect ui_bottom_dst_rect_{};
	std::array<::SDL_Rect, 2> filler_ui_rects_{};
	std::array<::SDL_Rect, 4> filler_hud_rects_{};
	::SDL_Rect screen_dst_rect_{};
	::SDL_Color filler_color_{};


	// HW
	//

	bstone::Rgba8Palette default_palette_{};
	bstone::Ren3dDeviceFeatures device_features_{};

	//
	// HW
}; // SwVideo

// ==========================================================================

SwVideo::SwVideo()
{
	initialize_video();
}

SwVideo::~SwVideo()
{
	uninitialize_vga_buffer();
}

bool SwVideo::is_hardware() const noexcept
{
	return false;
}

std::string SwVideo::get_renderer_name()
{
	auto sdl_renderer_info = ::SDL_RendererInfo{};

	bstone::ensure_sdl_result(::SDL_GetRendererInfo(
		renderer_.get(),
		&sdl_renderer_info
	));

	auto result = std::string{};
	result += "software (";
	result += sdl_renderer_info.name;
	result += ")";

	return result;
}

void SwVideo::clear_vga_buffer()
{
	std::uninitialized_fill(
		sw_vga_buffer_.begin(),
		sw_vga_buffer_.end(),
		::VgaBuffer::value_type{}
	);
}

void SwVideo::apply_widescreen()
{
	screen_texture_ = nullptr;
	uninitialize_vga_buffer();
	calculate_dimensions();
	initialize_vga_buffer();
	create_screen_texture();
	update_viewport();
}

void SwVideo::take_screenshot(
	int width,
	int height,
	int stride_rgb_888,
	::ScreenshotBuffer&& src_pixels_rgb_888)
{
	bstone::ensure_sdl_result(::SDL_RenderReadPixels(
		renderer_.get(),
		nullptr,
		::SDL_PIXELFORMAT_RGB24,
		src_pixels_rgb_888.get(),
		stride_rgb_888
	));

	vid_schedule_save_screenshot_task(
		width,
		height,
		stride_rgb_888,
		std::move(src_pixels_rgb_888),
		false
	);
}

void SwVideo::present()
{
	// HUD+3D stuff
	//
	if (vid_is_hud)
	{
		const auto texture_lock = bstone::SdlTextureLock{screen_texture_.get()};

		const auto& src_pixels = sw_vga_buffer_;
		const auto src_pitch = vga_width;

		const auto dst_pixels = texture_lock.get_pixels<std::uint32_t*>();
		const auto dst_bytes_pitch = texture_lock.get_pitch();
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
		const auto texture_lock = bstone::SdlTextureLock{ui_texture_.get()};

		const auto dst_pixels = texture_lock.get_pixels<std::uint32_t*>();
		const auto dst_bytes_pitch = texture_lock.get_pitch();
		const auto dst_pitch = dst_bytes_pitch / 4;

		auto src_line_offset = 0;
		auto dst_line_offset = 0;

		const auto alpha_0_mask = ~texture_pixel_format_->Amask;

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
	set_draw_color(0x00, 0x00, 0x00);
	clear_rendering_target();


	// Copy HUD+3D stuff
	//
	if (vid_is_hud)
	{
		copy_texture_to_rendering_target(screen_texture_.get(), nullptr, &screen_dst_rect_);
	}

	// Copy 2D stuff
	//
	if (vid_is_hud)
	{
		enable_texture_blending(ui_texture_.get(), true);
	}

	if (!vid_cfg_get().is_ui_stretched_)
	{
		if (vid_is_fizzle_fade)
		{
			copy_texture_to_rendering_target(ui_texture_.get(), &ui_top_src_rect_, &ui_top_dst_rect_);
			copy_texture_to_rendering_target(ui_texture_.get(), &ui_wide_middle_src_rect_, &ui_wide_middle_dst_rect_);
			copy_texture_to_rendering_target(ui_texture_.get(), &ui_bottom_src_rect_, &ui_bottom_dst_rect_);
		}
		else
		{
			copy_texture_to_rendering_target(ui_texture_.get(), nullptr, &ui_whole_dst_rect_);
		}
	}
	else
	{
		copy_texture_to_rendering_target(ui_texture_.get(), nullptr, &ui_stretched_dst_rect_);
	}

	if (vid_is_hud)
	{
		enable_texture_blending(ui_texture_.get(), false);
	}


	// Use filler if necessary
	//
	if (!vid_cfg_get().is_ui_stretched_)
	{
		const auto is_hud = vid_is_hud;

		auto fill_color = ::SDL_Color{};

		if (!vid_is_movie)
		{
			fill_color = filler_color_;
		}

		set_draw_color(fill_color.r, fill_color.g, fill_color.b);

		if (is_hud)
		{
			fill_rects(filler_hud_rects_.data(), static_cast<int>(filler_hud_rects_.size()));
		}
		else
		{
			fill_rects(filler_ui_rects_.data(), static_cast<int>(filler_ui_rects_.size()));
		}
	}

	if (vid_is_take_screenshot_scheduled)
	{
		vid_take_screenshot();
	}

	// Present
	//
	::SDL_RenderPresent(renderer_.get());
}

void SwVideo::get_palette(
	int offset,
	int count,
	std::uint8_t* vga_palette) const
{
	assert(offset >= 0);
	assert(count >= 0);
	assert((offset + count) <= 256);
	assert(vga_palette);

	auto& dst_vga_palette = *reinterpret_cast<::VgaPalette*>(vga_palette);

	std::uninitialized_copy_n(
		vga_palette_.cbegin() + offset,
		count,
		dst_vga_palette.begin()
	);
}

void SwVideo::fill_palette(
	int r,
	int g,
	int b) noexcept
{
	for (auto& vga_color : vga_palette_)
	{
		vga_color[0] = static_cast<std::uint8_t>(r);
		vga_color[1] = static_cast<std::uint8_t>(g);
		vga_color[2] = static_cast<std::uint8_t>(b);
	}

	update_palette_from_vga(0, 256);
}

void SwVideo::set_palette(
	int offset,
	int count,
	const std::uint8_t* vga_palette)
{
	assert(offset >= 0);
	assert(count >= 0);
	assert((offset + count) <= 256);
	assert(vga_palette);

	const auto& src_vga_palette = *reinterpret_cast<const ::VgaPalette*>(vga_palette);

	std::uninitialized_copy_n(
		src_vga_palette.cbegin(),
		count,
		vga_palette_.begin() + offset
	);

	update_palette_from_vga(offset, count);
}

void SwVideo::apply_window_mode()
{
	calculate_dimensions();
	vid_initialize_vanilla_raycaster();

	auto param = bstone::Ren3dSetWindowModeParam{};
	param.is_native = vid_is_native_mode();
	param.rect_2d_.extent_.width_ = vid_cfg_get().width;
	param.rect_2d_.extent_.height_ = vid_cfg_get().height;
	bstone::detail::Ren3dUtils::set_window_mode(window_.get(), param);

	vid_initialize_common();

	initialize_textures();
	initialize_vga_buffer();
}

void SwVideo::apply_filler_color_index()
{
	filler_color_.r = static_cast<::Uint8>((255 * vgapal[(vid_cfg_get().filler_color_index * 3) + 0]) / 63);
	filler_color_.g = static_cast<::Uint8>((255 * vgapal[(vid_cfg_get().filler_color_index * 3) + 1]) / 63);
	filler_color_.b = static_cast<::Uint8>((255 * vgapal[(vid_cfg_get().filler_color_index * 3) + 2]) / 63);
	filler_color_.a = 0xFF;
}

void SwVideo::fade_out(
	int start,
	int end,
	int red,
	int green,
	int blue,
	int steps)
{
	if (!g_no_fade_in_or_out)
	{
		auto palette1 = ::VgaPalette{};
		auto palette2 = ::VgaPalette{};

		get_palette(0, 256, &palette1[0][0]);

		std::uninitialized_copy_n(&palette1[0][0], 768, &palette2[0][0]);

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
}

void SwVideo::fade_in(
	int start,
	int end,
	const std::uint8_t* palette,
	int steps)
{
	if (!g_no_fade_in_or_out)
	{
		auto palette1 = ::VgaPalette{};
		auto palette2 = ::VgaPalette{};

		VL_GetPalette(0, 256, &palette1[0][0]);

		palette2 = palette1;

		const auto& src_palette = *reinterpret_cast<const ::VgaPalette*>(palette);

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
}

// HW
//

const bstone::Rgba8Palette& SwVideo::get_default_palette() const noexcept
{
	return default_palette_;
}

void SwVideo::enable_fizzle_fx(
	bool is_enabled)
{
	static_cast<void>(is_enabled);
}

void SwVideo::enable_fizzle_fx_fading(
	bool is_fading)
{
	static_cast<void>(is_fading);
}

void SwVideo::set_fizzle_fx_color_index(
	int color_index)
{
	static_cast<void>(color_index);
}

void SwVideo::set_fizzle_fx_ratio(
	float ratio)
{
	static_cast<void>(ratio);
}

void SwVideo::clear_wall_render_list() noexcept
{
}

void SwVideo::add_wall_render_item(
	int tile_x,
	int tile_y)
{
	static_cast<void>(tile_x);
	static_cast<void>(tile_y);
}

void SwVideo::clear_pushwall_render_list() noexcept
{
}

void SwVideo::add_pushwall_render_item(
	int tile_x,
	int tile_y)
{
	static_cast<void>(tile_x);
	static_cast<void>(tile_y);
}

void SwVideo::clear_door_render_list() noexcept
{
}

void SwVideo::add_door_render_item(
	int tile_x,
	int tile_y)
{
	static_cast<void>(tile_x);
	static_cast<void>(tile_y);
}

void SwVideo::clear_static_render_list() noexcept
{
}

void SwVideo::add_static_render_item(
	int bs_static_index)
{
	static_cast<void>(bs_static_index);
}

void SwVideo::clear_actor_render_list() noexcept
{
}

void SwVideo::add_actor_render_item(
	int bs_actor_index)
{
	static_cast<void>(bs_actor_index);
}

void SwVideo::on_load_level()
{
}

void SwVideo::on_update_wall_switch(
	int x,
	int y)
{
	static_cast<void>(x);
	static_cast<void>(y);
}

void SwVideo::on_move_pushwall()
{
}

void SwVideo::on_step_pushwall(
	int old_x,
	int old_y)
{
	static_cast<void>(old_x);
	static_cast<void>(old_y);
}

void SwVideo::on_pushwall_to_wall(
	int old_x,
	int old_y,
	int new_x,
	int new_y)
{
	static_cast<void>(old_x);
	static_cast<void>(old_y);
	static_cast<void>(new_x);
	static_cast<void>(new_y);
}

void SwVideo::on_move_door(
	int door_index)
{
	static_cast<void>(door_index);
}

void SwVideo::on_update_door_lock(
	int bs_door_index)
{
	static_cast<void>(bs_door_index);
}

void SwVideo::on_remove_static(
	const statobj_t& bs_static)
{
	static_cast<void>(bs_static);
}

void SwVideo::on_remove_actor(
	const objtype& bs_actor)
{
	static_cast<void>(bs_actor);
}

void SwVideo::apply_vsync()
{
}

void SwVideo::apply_msaa()
{
}

void SwVideo::apply_texture_upscale()
{
}

void SwVideo::apply_external_textures()
{
}

void SwVideo::update_samplers()
{
}

const bstone::Ren3dDeviceFeatures& SwVideo::get_device_features() const noexcept
{
	return device_features_;
}

//
// HW

void SwVideo::log(
	bstone::LoggerMessageKind message_type,
	const std::string& message)
{
	bstone::logger_->write(message_type, log_prefix + message);
}

void SwVideo::log(
	const std::string& message)
{
	log(bstone::LoggerMessageKind::information, message);
}

void SwVideo::log_warning(
	const std::string& message)
{
	log(bstone::LoggerMessageKind::warning, message);
}

void SwVideo::log_error(
	const std::string& message)
{
	log(bstone::LoggerMessageKind::error, message);
}

void SwVideo::initialize_video()
{
	log("");
	log("Initializing software accelerated video system.");

	vid_initialize_common();
	calculate_dimensions();

	vid_initialize_vanilla_raycaster();

	create_window();
	initialize_renderer();
	initialize_textures();
	initialize_palette();
	initialize_vga_buffer();
	vid_initialize_ui_buffer();

	const auto window_title = vid_get_window_title_for_renderer(get_renderer_name());
	::SDL_SetWindowTitle(window_.get(), window_title.c_str());

	::SDL_ShowWindow(window_.get());
	in_grab_mouse(true);
}

void SwVideo::set_draw_color(
	std::uint8_t r,
	std::uint8_t g,
	std::uint8_t b)
{
	bstone::ensure_sdl_result(::SDL_SetRenderDrawColor(renderer_.get(), r, g, b, 0xFF));
}

void SwVideo::clear_rendering_target()
{
	bstone::ensure_sdl_result(::SDL_RenderClear(renderer_.get()));
}

void SwVideo::copy_texture_to_rendering_target(
	::SDL_Texture* sdl_texture,
	const ::SDL_Rect* src_rect,
	const ::SDL_Rect* dst_rect)
{
	bstone::ensure_sdl_result(::SDL_RenderCopy(
		renderer_.get(),
		sdl_texture,
		src_rect,
		dst_rect
	));
}

void SwVideo::enable_texture_blending(
	::SDL_Texture* sdl_texture,
	bool is_enable)
{
	const auto sdl_mode = (is_enable ? ::SDL_BLENDMODE_BLEND : ::SDL_BLENDMODE_NONE);

	bstone::ensure_sdl_result(::SDL_SetTextureBlendMode(sdl_texture, sdl_mode));
}

void SwVideo::fill_rects(
	::SDL_Rect* rects,
	int rect_count)
{
	bstone::ensure_sdl_result(::SDL_RenderFillRects(renderer_.get(), rects, rect_count));
}

void SwVideo::initialize_vga_buffer()
{
	const auto area = 2 * ::vga_width * ::vga_height;

	sw_vga_buffer_.resize(area);

	vga_memory = sw_vga_buffer_.data();
}

void SwVideo::create_window()
{
	log("Creating window.");


	const auto& vid_cfg = vid_cfg_get();

	const auto is_native_mode = vid_is_native_mode();

	auto window_x = SDL_WINDOWPOS_CENTERED;
	auto window_y = SDL_WINDOWPOS_CENTERED;

	if (!is_native_mode && vid_cfg.is_positioned_)
	{
		window_x = vid_cfg.x;
		window_y = vid_cfg.y;
	}

	const ::Uint32 window_flags =
		::SDL_WINDOW_SHOWN |
		::SDL_WINDOW_ALLOW_HIGHDPI |
		(is_native_mode ? ::SDL_WINDOW_FULLSCREEN_DESKTOP : ::SDL_WindowFlags{}) |
		0
	;

	const auto title = vid_get_game_name_and_game_version_string();

	window_ = bstone::SdlWindowUPtr{bstone::ensure_sdl_result(::SDL_CreateWindow(
		title.c_str(),
		window_x,
		window_y,
		vid_layout_.window_width,
		vid_layout_.window_height,
		window_flags
	))};

	bstone::sdl::fill_window_black(window_.get());
}

void SwVideo::initialize_renderer()
{
	log("");
	log("Initializing renderer.");

	{
		log("Available renderer drivers:");

		const auto driver_count = ::SDL_GetNumRenderDrivers();

		for (auto i = decltype(driver_count){}; i < driver_count; ++i)
		{
			auto info = ::SDL_RendererInfo{};

			bstone::ensure_sdl_result(::SDL_GetRenderDriverInfo(i, &info));

			log(std::to_string(i + 1) + ". " + info.name);
		}
	}


	auto renderer_flags = ::Uint32{};
	const char* renderer_driver = nullptr;

	{
		if (!vid_cfg_get().is_vsync_)
		{
			log("Skipping VSync.");
		}
		else
		{
			renderer_flags = ::SDL_RENDERER_PRESENTVSYNC;

			log("Using VSync.");
		}
	}

	{
		log("Creating renderer.");

		renderer_ = bstone::SdlRendererUPtr{bstone::ensure_sdl_result(::SDL_CreateRenderer(
			window_.get(),
			-1,
			renderer_flags
		))};
	}


	auto renderer_info = ::SDL_RendererInfo{};

	{
		log("Quering renderer for information.");

		bstone::ensure_sdl_result(::SDL_GetRendererInfo(
			renderer_.get(),
			&renderer_info));
	}


	{
		if (renderer_driver)
		{
			if (::SDL_strcasecmp(renderer_driver, renderer_info.name) != 0)
			{
				log_warning("Mismatch renderer: \"" + std::string{renderer_info.name} + "\".");
			}
		}
		else
		{
			log("Renderer: \"" + std::string{renderer_info.name} + "\".");
		}
	}


	auto pixel_format = ::Uint32{::SDL_PIXELFORMAT_UNKNOWN};

	{
		log("Looking up for a texture pixel format.");

		const auto format_count = renderer_info.num_texture_formats;

		for (auto i = decltype(format_count){}; i < format_count; ++i)
		{
			const auto format = renderer_info.texture_formats[i];

			if (
				SDL_PIXELTYPE(format) == ::SDL_PIXELTYPE_PACKED32 &&
				SDL_PIXELLAYOUT(format) == ::SDL_PACKEDLAYOUT_8888 &&
				SDL_ISPIXELFORMAT_ALPHA(format))
			{
				pixel_format = format;

				break;
			}
		}

		if (pixel_format == ::SDL_PIXELFORMAT_UNKNOWN)
		{
			log_warning("Falling back to a predefined pixel format.");

			pixel_format = ::SDL_PIXELFORMAT_ARGB8888;
		}

		const auto pixel_format_name = ::SDL_GetPixelFormatName(::SDL_PIXELFORMAT_ARGB8888);
		log(std::string{"Pixel format: \""} + pixel_format_name + '\"');

		log("Allocating a texture pixel format.");

		texture_pixel_format_ = bstone::SdlPixelFormatUPtr{bstone::ensure_sdl_result(::SDL_AllocFormat(pixel_format))};
	}
}

void SwVideo::create_screen_texture()
{
	log("Creating screen texture.");

	screen_texture_ = bstone::SdlTextureUPtr{bstone::ensure_sdl_result(::SDL_CreateTexture(
		renderer_.get(),
		texture_pixel_format_->format,
		::SDL_TEXTUREACCESS_STREAMING,
		vga_width,
		vga_height
	))};
}

void SwVideo::create_ui_texture()
{
	log("Creating UI texture.");

	ui_texture_ = bstone::SdlTextureUPtr{bstone::ensure_sdl_result(::SDL_CreateTexture(
		renderer_.get(),
		texture_pixel_format_->format,
		SDL_TEXTUREACCESS_STREAMING,
		vga_ref_width,
		vga_ref_height
	))};
}

void SwVideo::initialize_textures()
{
	log("");
	log("Initializing textures.");

	create_screen_texture();
	create_ui_texture();
}

void SwVideo::update_viewport()
{
	auto sdl_viewport = ::SDL_Rect{};
	sdl_viewport.w = vid_layout_.window_width;
	sdl_viewport.h = vid_layout_.window_height;

	bstone::ensure_sdl_result(::SDL_RenderSetViewport(renderer_.get(), &sdl_viewport));
}

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
	ui_whole_src_rect_ = ::SDL_Rect
	{
		0,
		0,
		vga_ref_width,
		vga_ref_height,
	};

	ui_whole_dst_rect_ = ::SDL_Rect
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_width_4x3,
		vid_layout_.screen_height,
	};


	// UI stretched rect
	//
	ui_stretched_dst_rect_ = ::SDL_Rect
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_width,
		vid_layout_.screen_height,
	};


	// UI top rect
	//
	ui_top_src_rect_ = ::SDL_Rect
	{
		0,
		0,
		vga_ref_width,
		ref_top_bar_height,
	};

	ui_top_dst_rect_ = ::SDL_Rect
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_width_4x3,
		vid_layout_.screen_top_filler_height,
	};


	// UI middle rect (stretched to full width)
	//
	ui_wide_middle_src_rect_ = ::SDL_Rect
	{
		0,
		ref_view_top_y,
		vga_ref_width,
		ref_view_height,
	};

	ui_wide_middle_dst_rect_ = ::SDL_Rect
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_top_filler_height,
		vid_layout_.screen_width,
		vid_layout_.screen_height,
	};


	// UI bottom rect
	//
	ui_bottom_src_rect_ = ::SDL_Rect
	{
		0,
		ref_view_bottom_y + 1,
		vga_ref_width,
		ref_bottom_bar_height,
	};

	ui_bottom_dst_rect_ = ::SDL_Rect
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_height - vid_layout_.screen_bottom_filler_height,
		vid_layout_.screen_width_4x3,
		vid_layout_.screen_bottom_filler_height,
	};


	// UI left bar
	filler_ui_rects_[0] = ::SDL_Rect
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_height,
	};

	// UI right bar
	filler_ui_rects_[1] = ::SDL_Rect
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_width - vid_layout_.screen_left_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_height,
	};

	// HUD upper left rect
	filler_hud_rects_[0] = ::SDL_Rect
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_top_filler_height,
	};

	// HUD upper right rect
	filler_hud_rects_[1] = ::SDL_Rect
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_width - vid_layout_.screen_right_filler_width,
		vid_layout_.window_viewport_top_height,
		vid_layout_.screen_right_filler_width,
		vid_layout_.screen_top_filler_height,
	};

	// HUD lower left rect
	filler_hud_rects_[2] = ::SDL_Rect
	{
		vid_layout_.window_viewport_left_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_height - vid_layout_.screen_bottom_filler_height,
		vid_layout_.screen_left_filler_width,
		vid_layout_.screen_bottom_filler_height,
	};

	// HUD lower right rect
	filler_hud_rects_[3] = ::SDL_Rect
	{
		vid_layout_.window_viewport_left_width + vid_layout_.screen_width - vid_layout_.screen_right_filler_width,
		vid_layout_.window_viewport_top_height + vid_layout_.screen_height - vid_layout_.screen_bottom_filler_height,
		vid_layout_.screen_right_filler_width,
		vid_layout_.screen_bottom_filler_height,
	};

	apply_filler_color_index();

	// Screen destination rect.

	const auto screen_left = (
		vid_cfg_get().is_widescreen ?
		0 :
		vid_layout_.window_viewport_left_width + vid_layout_.screen_left_filler_width);

	const auto screen_top = vid_layout_.window_viewport_top_height;
	const auto screen_width = (vid_cfg_get().is_widescreen ? vid_layout_.screen_width : vid_layout_.screen_width_4x3);
	const auto screen_height = vid_layout_.screen_height;

	screen_dst_rect_ = ::SDL_Rect
	{
		screen_left,
		screen_top,
		screen_width,
		screen_height,
	};
}

void SwVideo::uninitialize_vga_buffer()
{
	sw_vga_buffer_ = ::VgaBuffer{};

	vga_memory = nullptr;
}

void SwVideo::update_palette_from_vga(
	int offset,
	int count)
{
	for (auto i = 0; i < count; ++i)
	{
		const auto& vga_color = vga_palette_[offset + i];
		auto& sdl_color = palette_[offset + i];

		sdl_color = ::SDL_MapRGB(
			texture_pixel_format_.get(),
			(255 * vga_color[0]) / 63,
			(255 * vga_color[1]) / 63,
			(255 * vga_color[2]) / 63
		);
	}
}

// ==========================================================================


VideoUPtr make_sw_video()
{
	return std::make_unique<SwVideo>();
}


} // bstone
