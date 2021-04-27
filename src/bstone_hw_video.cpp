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
// Hardware accelerated video (HW).
//


#include "bstone_hw_video.h"

#include <unordered_map>
#include <unordered_set>

#include "glm/gtc/matrix_transform.hpp"

#include "3d_def.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_vh.h"
#include "id_vl.h"

#include "bstone_hw_shader_registry.h"
#include "bstone_logger.h"
#include "bstone_mod_value.h"
#include "bstone_video.h"

#include "bstone_ren_3d_cmd_buffer.h"
#include "bstone_ren_3d_limits.h"
#include "bstone_ren_3d_mgr.h"

#include "bstone_detail_ren_3d_utils.h"


namespace bstone
{


class HwVideo :
	public bstone::Video
{
public:
	HwVideo()
	{
		initialize_video();
	}

	~HwVideo() override
	{
		uninitialize_video();
	}


	bool is_hardware() const noexcept override
	{
		return true;
	}

	std::string get_renderer_name() override
	{
		return renderer_->get_name();
	}

	void clear_vga_buffer() override
	{
	}

	void take_screenshot(
		int width,
		int height,
		int stride_rgb_888,
		ScreenshotBuffer&& src_pixels_rgb_888) override
	{
		auto is_flipped_vertically = false;

		renderer_->read_pixels_rgb_888(src_pixels_rgb_888.get(), is_flipped_vertically);

		vid_schedule_save_screenshot_task(
			width,
			height,
			stride_rgb_888,
			std::move(src_pixels_rgb_888),
			is_flipped_vertically
		);
	}

	void present() override
	{
		if (renderer_ == nullptr)
		{
			return;
		}

		if (vid_is_hud && player != nullptr)
		{
			is_draw_3d_ = true;

			update_player();
			build_view_matrix();
		}

		present_common();
		present_3d();
		present_2d();

		renderer_->submit_commands(
			command_buffers_.data(),
			static_cast<int>(command_buffers_.size())
		);

		if (vid_is_take_screenshot_scheduled)
		{
			vid_take_screenshot();
		}

		renderer_->present();

		is_draw_3d_ = false;
	}

	void get_palette(
		int offset,
		int count,
		std::uint8_t* vga_palette) override
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

	void fill_palette(
		int r,
		int g,
		int b) override
	{
		for (auto& vga_color : vga_palette_)
		{
			vga_color[0] = static_cast<std::uint8_t>(r);
			vga_color[1] = static_cast<std::uint8_t>(g);
			vga_color[2] = static_cast<std::uint8_t>(b);
		}

		update_palette_from_vga(0, 256);
	}

	void set_palette(
		int offset,
		int count,
		const std::uint8_t* vga_palette) override
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

	void fade_out(
		int start,
		int end,
		int red,
		int green,
		int blue,
		int step_count) override
	{
		static_cast<void>(start);
		static_cast<void>(end);

		r2_fade_is_enabled_ = true;

		r2_fade_color_ = vga_color_to_rgba_8(red, green, blue);

		if (!g_no_fade_in_or_out)
		{
			const auto alpha = 0xFF;

			for (int i = 0; i < step_count; ++i)
			{
				const auto new_alpha = (i * alpha) / step_count;

				r2_fade_color_.a_ = static_cast<std::uint8_t>(new_alpha);

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
		r2_fade_color_.a_ = 0xFF;

		fill_palette(red, green, blue);

		present();

		if (!vid_has_vsync)
		{
			VL_WaitVBL(1);
		}

		r2_fade_is_enabled_ = false;

		screenfaded = true;
	}

	void fade_in(
		int start,
		int end,
		const std::uint8_t* palette,
		int step_count) override
	{
		static_cast<void>(start);
		static_cast<void>(end);

		r2_fade_is_enabled_ = true;
		screenfaded = false;
	
		VL_SetPalette(0, 256, palette);

		r2_fade_color_.a_ = 0xFF;

		if (!g_no_fade_in_or_out)
		{
			const auto alpha = 0xFF;

			for (int i = 0; i < step_count; ++i)
			{
				const auto new_alpha = ((step_count - 1 - i) * alpha) / step_count;

				r2_fade_color_.a_ = static_cast<std::uint8_t>(new_alpha);

				present();

				if (!vid_has_vsync)
				{
					VL_WaitVBL(1);
				}
			}
		}

		r2_fade_color_.a_ = 0x00;

		present();

		if (!vid_has_vsync)
		{
			VL_WaitVBL(1);
		}

		r2_fade_is_enabled_ = false;
	}

	void apply_widescreen() override
	{
		calculate_dimensions();
		SetViewSize();
		build_projection_matrix();
		build_player_weapon_projection_matrix();
		update_3d_fade_vb();
	}

	void apply_window_mode() override
	{
		calculate_dimensions();
		vid_initialize_vanilla_raycaster();

		auto param = bstone::Ren3dSetWindowModeParam{};
		param.is_windowed = vid_cfg_get().is_windowed;
		param.rect_2d_.extent_.width_ = vid_layout_.window_width;
		param.rect_2d_.extent_.height_ = vid_layout_.window_height;
		renderer_->set_window_mode(param);

		vid_initialize_common();

		uninitialize_2d();
		initialize_2d();

		uninitialize_3d_fade();
		initialize_3d_fade();

		uninitialize_player_weapon();
		initialize_player_weapon();

		build_matrices();
	}

	void apply_filler_color_index() override
	{
		destroy_2d_fillers_vi();
		destroy_2d_fillers_ib();
		destroy_2d_fillers_vb();

		create_2d_fillers_ib();
		create_2d_fillers_vb();
		create_2d_fillers_vi();
	}

	const bstone::Rgba8Palette& get_default_palette() override
	{
		return default_palette_;
	}

	void enable_fizzle_fx(
		bool is_enabled) override
	{
		fizzle_fx_is_enabled_ = is_enabled;
	}

	void enable_fizzle_fx_fading(
		bool is_fading) override
	{
		fizzle_fx_is_fading_ = is_fading;
	}

	void set_fizzle_fx_color_index(
		int color_index) override
	{
		fizzle_fx_color_index_ = color_index;
	}

	void set_fizzle_fx_ratio(
		float ratio) override
	{
		fizzle_fx_ratio_ = ratio;
	}

	void clear_wall_render_list() override
	{
		wall_last_xy_to_render_at_ = -1;
		walls_to_render_.clear();
	}

	void add_wall_render_item(
		int tile_x,
		int tile_y) override
	{
		const auto xy = encode_xy(tile_x, tile_y);

		if (wall_last_xy_to_render_at_ == xy)
		{
			return;
		}

		wall_last_xy_to_render_at_ = xy;

		const auto is_pushwall =
			bs_is_pushwall(tile_x, tile_y) ||
			xy_pushwall_map_.find(xy) != xy_pushwall_map_.cend();

		if (is_pushwall)
		{
			pushwalls_to_render_.insert(xy);
		}
		else
		{
			walls_to_render_.insert(xy);
		}
	}

	void clear_pushwall_render_list() override
	{
		pushwall_last_xy_to_render_at_ = -1;
		pushwalls_to_render_.clear();
	}

	void add_pushwall_render_item(
		int tile_x,
		int tile_y) override
	{
		const auto xy = encode_xy(tile_x, tile_y);

		if (pushwall_last_xy_to_render_at_ == xy)
		{
			return;
		}

		pushwall_last_xy_to_render_at_ = xy;

		pushwalls_to_render_.insert(xy);
	}

	void clear_door_render_list() override
	{
		door_last_xy_to_render_at_ = -1;
		doors_to_render_.clear();
	}

	void add_door_render_item(
		int tile_x,
		int tile_y) override
	{
		const auto xy = encode_xy(tile_x, tile_y);

		if (door_last_xy_to_render_at_ == xy)
		{
			return;
		}

		door_last_xy_to_render_at_ = xy;

		doors_to_render_.insert(xy);
	}

	void clear_static_render_list() override
	{
		statics_to_render_.clear();
	}

	void add_static_render_item(
		int bs_static_index) override
	{
		statics_to_render_.insert(bs_static_index);
	}

	void clear_actor_render_list() override
	{
		actors_to_render_.clear();
	}

	void add_actor_render_item(
		int bs_actor_index) override
	{
		actors_to_render_.emplace(bs_actor_index);
	}

	void on_load_level() override
	{
		precache_resources();

		build_pushwalls();
		build_walls();
		build_doors();
		build_sprites();
	}

	void on_update_wall_switch(
		int x,
		int y) override
	{
		assert(x >= 0 && x < MAPSIZE && y >= 0 && y < MAPSIZE);

		const auto xy = encode_xy(x, y);

		auto wall_it = xy_wall_map_.find(xy);

		if (wall_it == xy_wall_map_.cend())
		{
			const auto& assets_info = get_assets_info();

			if (assets_info.is_aog())
			{
				Quit("Expected wall at (" + std::to_string(x) + ", " + std::to_string(y) + ").");
			}
			else
			{
				// Operable non-directly.
				return;
			}
		}

		const auto tile_wall = tilemap[x][y] & tilemap_wall_mask;

		assert(tile_wall == OFF_SWITCH || tile_wall == ON_SWITCH);

		const auto horizontal_wall_id = horizwall[tile_wall];
		const auto vertical_wall_id = vertwall[tile_wall];

		auto& wall = wall_it->second;

		for (auto& side : wall.sides)
		{
			const auto& flags = side.flags;

			if (!flags.is_active || flags.is_door_track)
			{
				continue;
			}

			if (flags.is_vertical)
			{
				side.texture_id = vertical_wall_id;
			}
			else
			{
				side.texture_id = horizontal_wall_id;
			}
		}
	}

	void on_move_pushwall() override
	{
		translate_pushwall();
	}

	void on_step_pushwall(
		int old_x,
		int old_y) override
	{
		step_pushwall(old_x, old_y);
	}

	void on_pushwall_to_wall(
		int old_x,
		int old_y,
		int new_x,
		int new_y) override
	{
		//assert(old_x != new_x || old_y != new_y);

		const auto old_xy = encode_xy(old_x, old_y);
		const auto old_pushwall_it = xy_pushwall_map_.find(old_xy);

		if (old_pushwall_it == xy_pushwall_map_.cend())
		{
			Quit("Pushwall not found.");

			return;
		}

		xy_pushwall_map_.erase(old_pushwall_it);

		auto vertex_index = 0;

		map_xy_to_xwall(
			XyWallKind::solid,
			new_x,
			new_y,
			xy_wall_map_,
			vertex_index,
			pushwall_to_wall_vbi_
		);

		// Adjust vertex indices.
		//
		const auto new_xy = encode_xy(new_x, new_y);
		auto& wall = xy_wall_map_[new_xy];

		for (auto& wall_side : wall.sides)
		{
			if (!wall_side.flags.is_active)
			{
				continue;
			}

			wall_side.vertex_index += wall_vertex_count_;
		}

		update_vertex_buffer(
			wall_sides_vb_,
			wall_vertex_count_,
			vertex_index,
			pushwall_to_wall_vbi_.data()
		);

		wall_vertex_count_ += vertex_index;
	}

	void on_move_door(
		int door_index) override
	{
		const auto& bs_door = doorobjlist[door_index];

		const auto xy = encode_xy(bs_door.tilex, bs_door.tiley);

		const auto map_it = xy_door_map_.find(xy);

		if (map_it == xy_door_map_.cend())
		{
			Quit("Door mapping not found.");
		}

		auto& door = xy_door_map_[xy];

		auto vertex_index = door.vertex_index;
		const auto old_vertex_index = vertex_index;

		map_door_side(door.sides.front(), vertex_index, doors_vbi_);

		update_vertex_buffer(
			door_sides_vb_,
			old_vertex_index,
			vertices_per_door,
			&doors_vbi_[old_vertex_index]
		);
	}

	void on_update_door_lock(
		int bs_door_index) override
	{
		const auto& bs_door = doorobjlist[bs_door_index];

		const auto xy = encode_xy(bs_door.tilex, bs_door.tiley);

		const auto map_it = xy_door_map_.find(xy);

		if (map_it == xy_door_map_.cend())
		{
			Quit("Door mapping not found.");
		}

		auto& door = xy_door_map_[xy];

		auto front_face_page_number = 0;
		auto back_face_page_number = 0;

		door_get_page_numbers(bs_door, front_face_page_number, back_face_page_number);

		door.sides[0].texture_id = front_face_page_number;
		door.sides[1].texture_id = back_face_page_number;
	}

	void on_remove_static(
		const statobj_t& bs_static) override
	{
		const auto bs_static_index = get_static_index(bs_static);
		auto& hw_static = statics_[bs_static_index];
		hw_static = {};
	}

	void on_remove_actor(
		const objtype& bs_actor) override
	{
		const auto bs_actor_index = get_actor_index(bs_actor);
		auto& hw_actor = actors_[bs_actor_index];
		hw_actor = {};
	}

	void apply_vsync() override
	{
		renderer_->enable_vsync(vid_cfg_get().is_vsync_);
	}

	void apply_msaa() override
	{
		renderer_->set_anti_aliasing(vid_cfg_get().aa_kind_, vid_cfg_get().aa_degree_);
	}

	void apply_texture_upscale() override
	{
		log("");
		log("Applying texture upscale.");


		destroy_texture_upscale_resources();

		texture_mgr_->set_upscale_filter(
			vid_cfg_get().texture_upscale_kind_,
			vid_cfg_get().texture_upscale_xbrz_degree_
		);

		create_texture_upscale_resources();
	}

	void apply_external_textures() override
	{
		log("");
		log("Applying external textures.");


		destroy_external_textures_resources();
		texture_mgr_->enable_external_textures(vid_cfg_get().is_external_textures_enabled_);
		create_external_textures_resources();
	}

	void update_samplers() override
	{
		update_ui_sampler();
		update_sprite_sampler();
		update_wall_sampler();
		update_player_weapon_sampler();
	}

	const bstone::Ren3dDeviceFeatures& get_device_features() const noexcept override
	{
		return device_features_;
	}


private:
	static constexpr auto log_prefix = "[VIDHW] ";

	::VgaPalette vga_palette_{};


	[[noreturn]]
	static void fail(
		const std::string& message)
	{
		::Quit("[VIDHW] " + message);
	}

	static void log(
		bstone::LoggerMessageKind message_type,
		const std::string& message)
	{
		bstone::logger_->write(message_type, log_prefix + message);
	}

	static void log(
		const std::string& message)
	{
		log(bstone::LoggerMessageKind::information, message);
	}

	static void log_warning(
		const std::string& message)
	{
		log(bstone::LoggerMessageKind::warning, message);
	}

	static void log_error(
		const std::string& message)
	{
		log(bstone::LoggerMessageKind::error, message);
	}


enum class VertexAttribLocationId :
	unsigned char
{
	none,
	position,
	color,
	texture_coordinates,
}; // VertexAttribLocationId


template<
	typename T
>
static constexpr T map_dimension = static_cast<T>(MAPSIZE);

static constexpr auto map_dimension_f = map_dimension<float>;
static constexpr auto map_dimension_d = map_dimension<double>;
static constexpr auto map_height_f = 1.0F;

template<
	typename T
>
static constexpr auto tile_dimension = static_cast<T>(1);

static constexpr auto tile_dimension_f = tile_dimension<float>;

template<
	typename T
>
static constexpr auto tile_half_dimension = tile_dimension<T> / static_cast<T>(2);

static constexpr auto tile_half_dimension_d = tile_half_dimension<double>;


static constexpr auto vertices_per_triangle = 3;
static constexpr auto triangles_per_quad = 2;
static constexpr auto vertices_per_quad = triangles_per_quad * vertices_per_triangle;

static constexpr auto sides_per_wall = 4;
static constexpr auto indices_per_wall_side = 6;
static constexpr auto vertices_per_wall_side = 4;

static constexpr auto max_wall_sides_indices = 0x10000;

static constexpr auto sides_per_door = 2;
static constexpr auto door_halves_per_side = 2;
static constexpr auto door_halves_per_door = sides_per_door * door_halves_per_side;
static constexpr auto vertices_per_door_half = 4;
static constexpr auto vertices_per_door = sides_per_door * vertices_per_door_half;

static constexpr auto indices_per_door_half = 6;
static constexpr auto indices_per_door = 2 * indices_per_door_half;
static constexpr auto indices_per_door_side = 2 * indices_per_door;

static constexpr auto max_sprites = MAXSTATS + MAXACTORS;

static constexpr auto vertices_per_sprite = 4;
static constexpr auto indices_per_sprite = 6;

static constexpr auto max_statics_vertices = MAXSTATS * vertices_per_sprite;
static constexpr auto max_actors_vertices = MAXACTORS * vertices_per_sprite;
static constexpr auto max_sprites_vertices = max_statics_vertices + max_actors_vertices;
static constexpr auto statics_base_vertex_index = 0;
static constexpr auto actors_base_vertex_index = max_statics_vertices;

static constexpr auto max_statics_indices = MAXSTATS * indices_per_sprite;
static constexpr auto max_actor_indices = MAXACTORS * indices_per_sprite;
static constexpr auto max_sprites_indices = max_statics_indices + max_actor_indices;

static constexpr auto cloaked_actor_alpha_u8 = std::uint8_t{0x50};

static constexpr auto common_command_buffer_initial_size = 1'024;
static constexpr auto common_command_buffer_resize_delta_size = 1'024;

static constexpr auto r2_command_buffer_initial_size = 4'096;
static constexpr auto r2_command_buffer_resize_delta_size = 4'096;

static constexpr auto r3_command_buffer_initial_size = 16'384;
static constexpr auto r3_command_buffer_resize_delta_size = 16'384;


template<
	typename T,
	VertexAttribLocationId TLocationId,
	typename = int
>
struct VertexHasAttribute :
	std::false_type
{
}; // VertexHasAttribute

template<
	typename T
>
struct VertexHasAttribute<
	T,
	VertexAttribLocationId::position,
	decltype(static_cast<void>(T::xyz), 0)>
	:
	std::true_type
{
}; // VertexHasAttribute

template<
	typename T
>
struct VertexHasAttribute<
	T,
	VertexAttribLocationId::color,
	decltype(static_cast<void>(T::rgba8), 0)>
	:
	std::true_type
{
}; // VertexHasAttribute

template<
	typename T
>
struct VertexHasAttribute<
	T,
	VertexAttribLocationId::texture_coordinates,
	decltype(static_cast<void>(T::uv), 0)>
	:
	std::true_type
{
}; // VertexHasAttribute


template<
	typename T,
	VertexAttribLocationId TLocationId,
	typename = int
>
struct VertexAttributeTraits
{
	static constexpr auto is_valid = false;
	static constexpr auto offset = -1;
	static constexpr auto stride = -1;
}; // VertexAttributeTraits

template<
	typename T
>
struct VertexAttributeTraits<
	T,
	VertexAttribLocationId::position,
	decltype(static_cast<void>(T::xyz), 0)>
{
	static constexpr auto is_valid = true;
	static constexpr auto offset = static_cast<int>(offsetof(T, xyz));
	static constexpr auto stride = static_cast<int>(sizeof(T));
}; // VertexAttributeTraits

template<
	typename T
>
struct VertexAttributeTraits<
	T,
	VertexAttribLocationId::color,
	decltype(static_cast<void>(T::rgba8), 0)>
{
	static constexpr auto is_valid = true;
	static constexpr auto offset = static_cast<int>(offsetof(T, rgba8));
	static constexpr auto stride = static_cast<int>(sizeof(T));
}; // VertexAttributeTraits

template<
	typename T
>
struct VertexAttributeTraits<
	T,
	VertexAttribLocationId::texture_coordinates,
	decltype(static_cast<void>(T::uv), 0)>
{
	static constexpr auto is_valid = true;
	static constexpr auto offset = static_cast<int>(offsetof(T, uv));
	static constexpr auto stride = static_cast<int>(sizeof(T));
}; // VertexAttributeTraits


using VertexPosition = glm::vec3;
using VertexColor = bstone::Rgba8;
using VertexTextureCoordinates = glm::vec2;

struct VertexXyzUv
{
	VertexPosition xyz;
	VertexTextureCoordinates uv;
}; // VertexXyzUv

struct VertexXyzRgbaUv
{
	VertexPosition xyz;
	VertexColor rgba8;
	VertexTextureCoordinates uv;
}; // VertexXyzRgbaUv


struct Vertex : VertexXyzRgbaUv {};
struct WallVertex : VertexXyzUv {};
struct PushwallVertex : VertexXyzRgbaUv {};
struct FlooringVertex : VertexXyzUv {};
struct CeilingVertex : VertexXyzUv {};
struct DoorVertex : VertexXyzUv {};
struct SpriteVertex : VertexXyzRgbaUv {};
struct PlayerWeaponVertex : VertexXyzUv {};
struct FadeVertex : VertexXyzUv {};


struct QuadFlags
{
	using Value = unsigned char;


	Value is_vertical : 1;
	Value is_back_face : 1;
}; // QuadFlags


struct Wall;

using WallCPtr = const Wall*;

struct WallSideFlags
{
	using Type = unsigned char;


	Type is_active : 1;
	Type is_vertical : 1;
	Type is_door_track : 1;
}; // WallSideFlags

struct WallSide
{
	WallCPtr wall;

	WallSideFlags flags;
	int vertex_index;
	int texture_id;
}; // WallSide

using WallSideCPtr = const WallSide*;


struct Wall
{
	static constexpr auto max_sides = 4;

	using Sides = std::array<WallSide, max_sides>;


	int x;
	int y;

	Sides sides;
}; // Wall

using XyWallMap = std::unordered_map<int, Wall>;
using WallsToRenderList = std::unordered_set<int>;

enum XyWallKind
{
	solid,
	push,
}; // XyWallKind

struct WallSideDrawItem
{
	int texture_id;
	WallSideCPtr wall_side_;
}; // WallSideDrawItem


struct Door;

using DoorPtr = Door*;

struct DoorSide
{
	DoorPtr door;

	bool is_back_face;
	int texture_id;
}; // DoorSide

using DoorSideCPtr = const DoorSide*;


struct Door
{
	using Sides = std::array<DoorSide, sides_per_door>;

	int bs_door_index;
	int vertex_index;
	Sides sides;
}; // Door


struct DoorDrawItem
{
	int texture_id;
	DoorSideCPtr door_side;
}; // DoorDrawItem

using XyDoorMap = std::unordered_map<int, Door>;
using DoorsToRenderList = std::unordered_set<int>;
using DoorDrawItems = std::vector<DoorDrawItem>;

using WallSideDrawItems = std::vector<WallSideDrawItem>;

using WallSideIndexBuffer = std::vector<std::uint16_t>;
using DoorIndexBuffer = std::vector<std::uint16_t>;


enum class SpriteKind
{
	none,
	stat,
	actor,
}; // SpriteKind

struct SpriteFlags
{
	using Value = unsigned char;


	Value is_visible : 1;
}; // SpriteFlags

struct Sprite
{
	union BsObject
	{
		const statobj_t* stat;
		const objtype* actor;
	}; // BsObject


	double x;
	double y;
	int tile_x;
	int tile_y;
	int bs_sprite_id;
	double square_distance;

	SpriteKind kind;
	SpriteFlags flags;
	int vertex_index;
	BsObject bs_object;
}; // Sprite


using SpriteCPtr = const Sprite*;

struct SpriteDrawItem
{
	int texture_id;
	SpriteCPtr sprite;
}; // SpriteDrawItem

using Sprites = std::vector<Sprite>;

using StaticsToRenderList = std::unordered_set<int>;
using ActorsToRenderList = std::unordered_set<int>;
using SpritesDrawList = std::vector<SpriteDrawItem>;

using SpritesIndexBuffer = std::vector<std::uint16_t>;


template<
	typename TVertex
>
using VertexBufferImageT = std::vector<TVertex>;

using Vbi = VertexBufferImageT<Vertex>;
using WallsVbi = VertexBufferImageT<WallVertex>;
using PushwallsVbi = VertexBufferImageT<PushwallVertex>;
using DoorsVbi = VertexBufferImageT<DoorVertex>;
using PlayerWeaponVbi = VertexBufferImageT<PlayerWeaponVertex>;
using FadeVbi = VertexBufferImageT<FadeVertex>;


using SamplerVarMod = bstone::ModValue<int>;
SamplerVarMod sampler_var_;

using ShadingModeMod = bstone::ModValue<int>;
ShadingModeMod shading_mode_;

using BsShadeMaxMod = bstone::ModValue<int>;
BsShadeMaxMod bs_shade_max_;

using BsNormalShadeMod = bstone::ModValue<int>;
BsNormalShadeMod bs_normal_shade_;

using BsHeightNumeratorMod = bstone::ModValue<double>;
BsHeightNumeratorMod bs_height_numerator_;

using BsLightingMod = bstone::ModValue<int>;
BsLightingMod bs_lighting_;

using BsViewDirectionMod = bstone::ModValue<glm::vec2>;
BsViewDirectionMod bs_view_direction_;

using BsViewPositionMod = bstone::ModValue<glm::vec2>;
BsViewPositionMod bs_view_position_;


glm::mat4 r2_matrix_model_ = glm::mat4{};
glm::mat4 r2_matrix_view_ = glm::mat4{};
glm::mat4 r2_matrix_projection_ = glm::mat4{};


static constexpr auto r2_quad_count = 2;

static constexpr auto r2_index_count_ = r2_quad_count * 6;
static constexpr auto r2_stretched_index_offset_ = 0;
static constexpr auto r2_non_stretched_index_offset_ = 6;

static constexpr auto r2_vertex_count_ = r2_quad_count * 4;
static constexpr auto r2_stretched_vertex_offset_ = 0;
static constexpr auto r2_non_stretched_vertex_offset_ = 4;

using R2Vbi = std::array<Vertex, r2_vertex_count_>;


static constexpr auto r2_fillers_ui_quad_count = 2;
static constexpr auto r2_fillers_hud_quad_count = 4;
static constexpr auto r2_fillers_quad_count = r2_fillers_ui_quad_count + r2_fillers_hud_quad_count;

static constexpr auto r2_fillers_index_count_ = r2_fillers_quad_count * 6;
static constexpr auto r2_fillers_ui_index_offset_ = 0;
static constexpr auto r2_fillers_hud_index_offset_ = 6 * r2_fillers_ui_quad_count;

static constexpr auto r2_fillers_vertex_count_ = r2_fillers_quad_count * 4;



// Reference horizontal FOV of the camera (degrees).
// (Found out empirical.)
static constexpr double ref_camera_hfov_deg = 49.0;

// Vertical FOV in radians of the camera.
double camera_vfov_rad_ = 0.0;

// Vertical FOV in degrees of the camera.
double camera_vfov_deg_ = 0.0;

// Distance to the near plane of the camera.
double camera_near_distance_ = 0.0;

// Distance to the far plane of the camera.
double camera_far_distance_ = 0.0;


bool is_draw_3d_ = false;

bstone::Ren3dSamplerState ui_sampler_state_;
bstone::Ren3dSamplerUPtr ui_sampler_;

bstone::Ren3dSamplerState wall_sampler_state_;
bstone::Ren3dSamplerUPtr wall_sampler_;

bstone::Ren3dSamplerState sprite_sampler_state_;
bstone::Ren3dSamplerUPtr sprite_sampler_;

bstone::Ren3dSamplerState player_weapon_sampler_state_;
bstone::Ren3dSamplerUPtr player_weapon_sampler_;

bstone::Ren3dSamplerUPtr fade_sampler_;


R2Vbi r2_vertices_;

bstone::Ren3dDeviceFeatures device_features_;
bstone::Ren3dMgrUPtr renderer_mgr_;
bstone::Ren3dPtr renderer_ = nullptr;

bstone::HwTextureMgrUPtr texture_mgr_;

bstone::Rgba8Palette palette_;
bstone::Rgba8Palette default_palette_;

bstone::Ren3dCmdBufferUPtr common_command_buffer_;
bstone::Ren3dCmdBufferUPtr r2_command_buffer_;
bstone::Ren3dCmdBufferUPtr r3_command_buffer_;

using Ren3dCmdBuffers = std::array<bstone::Ren3dCmdBufferPtr, 3>;
Ren3dCmdBuffers command_buffers_;

bstone::Ren3dTexture2dPtr ui_t2d_ = nullptr;
bstone::Ren3dBufferUPtr ui_ib_;
bstone::Ren3dBufferUPtr ui_vb_;
bstone::Ren3dVertexInputUPtr ui_vi_;

bstone::Ren3dTexture2dPtr r2_black_t2d_1x1_ = nullptr;
bstone::Ren3dTexture2dPtr r2_white_t2d_1x1_ = nullptr;

bstone::Ren3dBufferUPtr r2_fillers_ib_;
bstone::Ren3dBufferUPtr r2_fillers_vb_;
bstone::Ren3dVertexInputUPtr r2_fillers_vi_;

bool r2_fade_is_enabled_{};
VertexColor r2_fade_color_{};
bstone::Ren3dTexture2dPtr r2_fade_t2d_{};


glm::mat4 r3_matrix_bs_to_r_{};
glm::mat4 r3_matrix_model_{};
glm::mat4 r3_matrix_view_{};
glm::mat4 r3_matrix_projection_{};


bstone::Ren3dBufferUPtr flooring_ib_;
bstone::Ren3dBufferUPtr flooring_vb_;
bstone::Ren3dVertexInputUPtr flooring_vi_;
bstone::Ren3dTexture2dPtr flooring_solid_t2d_ = nullptr;
bstone::Ren3dTexture2dPtr flooring_textured_t2d_ = nullptr;

bstone::Ren3dBufferUPtr ceiling_ib_;
bstone::Ren3dBufferUPtr ceiling_vb_;
bstone::Ren3dVertexInputUPtr ceiling_vi_;
bstone::Ren3dTexture2dPtr ceiling_solid_t2d_ = nullptr;
bstone::Ren3dTexture2dPtr ceiling_textured_t2d_ = nullptr;


double player_angle_rad_{};
glm::dvec2 view_direction_{};
glm::dvec2 player_position_{};
glm::dvec3 view_position_{};


bool has_active_pushwall_ = false;
int active_pushwall_next_x_ = 0;
int active_pushwall_next_y_ = 0;

int wall_count_ = 0;
int wall_side_count_ = 0;
int wall_vertex_count_ = 0;
int wall_last_xy_to_render_at_ = 0;
XyWallMap xy_wall_map_;
WallsToRenderList walls_to_render_;

int wall_side_draw_item_count_ = 0;
WallSideDrawItems wall_side_draw_items_;

bstone::Ren3dBufferUPtr wall_sides_ib_;
bstone::Ren3dBufferUPtr wall_sides_vb_;
bstone::Ren3dVertexInputUPtr wall_sides_vi_;

WallSideIndexBuffer wall_sides_ibi_;


int pushwall_count_ = 0;
int pushwall_side_count_ = 0;
int pushwall_last_xy_to_render_at_ = 0;
XyWallMap xy_pushwall_map_;
WallsVbi pushwall_to_wall_vbi_;
WallsToRenderList pushwalls_to_render_;

int pushwall_side_draw_item_count_ = 0;
WallSideDrawItems pushwall_side_draw_items_;

bstone::Ren3dBufferUPtr pushwall_sides_ib_;
bstone::Ren3dBufferUPtr pushwall_sides_vb_;
bstone::Ren3dVertexInputUPtr pushwall_sides_vi_;

WallSideIndexBuffer pushwall_sides_ibi_;
PushwallsVbi pushwalls_vbi_;


int door_count_ = 0;

XyDoorMap xy_door_map_;

int door_draw_item_count_ = 0;
int door_last_xy_to_render_at_ = 0;
DoorsToRenderList doors_to_render_;
DoorDrawItems door_draw_items_;

bstone::Ren3dBufferUPtr door_sides_ib_;
bstone::Ren3dBufferUPtr door_sides_vb_;
bstone::Ren3dVertexInputUPtr door_sides_vi_;

DoorIndexBuffer door_sides_ibi_;
DoorsVbi doors_vbi_;


Sprites statics_;
StaticsToRenderList statics_to_render_;

using ActorsToReposition = std::vector<Sprite>;
Sprites actors_;

int sprites_draw_count_ = 0;
ActorsToRenderList actors_to_render_;
SpritesDrawList sprites_draw_list_;

bstone::Ren3dBufferUPtr sprites_ib_;
bstone::Ren3dBufferUPtr sprites_vb_;
bstone::Ren3dVertexInputUPtr sprites_vi_;

SpritesIndexBuffer sprites_ibi_;
Vbi sprites_vbi_;


bstone::Ren3dBufferUPtr player_weapon_ib_;
bstone::Ren3dBufferUPtr player_weapon_vb_;
bstone::Ren3dVertexInputUPtr player_weapon_vi_;
glm::mat4 player_weapon_model_matrix_;
glm::mat4 player_weapon_view_matrix_;
glm::mat4 player_weapon_projection_matrix_;


bool r3_fade_is_enabled_ = false;
bstone::Ren3dBufferUPtr r3_fade_ib_;
bstone::Ren3dBufferUPtr r3_fade_vb_;
bstone::Ren3dVertexInputUPtr r3_fade_vi_;
bstone::Ren3dTexture2dPtr r3_fade_t2d_ = nullptr;

bool fizzle_fx_is_enabled_ = false;
bool fizzle_fx_is_fading_ = false;
int fizzle_fx_color_index_ = 0;
float fizzle_fx_ratio_ = 0.0F;

bstone::Ren3dShaderUPtr fragment_shader_;
bstone::Ren3dShaderUPtr vertex_shader_;
bstone::Ren3dShaderStageUPtr shader_stage_;

bstone::Ren3dShaderMat4VarPtr model_mat_uniform_;
bstone::Ren3dShaderMat4VarPtr view_mat_uniform_;
bstone::Ren3dShaderMat4VarPtr projection_mat_uniform_;

bstone::Ren3dShaderSampler2dVarPtr sampler_uniform_;
bstone::Ren3dShaderInt32VarPtr shading_mode_uniform_;
bstone::Ren3dShaderFloat32VarPtr shade_max_uniform_;
bstone::Ren3dShaderFloat32VarPtr normal_shade_uniform_;
bstone::Ren3dShaderFloat32VarPtr height_numerator_uniform_;
bstone::Ren3dShaderFloat32VarPtr extra_lighting_uniform_;
bstone::Ren3dShaderVec2VarPtr view_direction_uniform_;
bstone::Ren3dShaderVec2VarPtr view_position_uniform_;



static const bstone::Ren3dVec2& cast_glm_vec2(
	const glm::vec2& glm_vec2)
{
	return reinterpret_cast<const bstone::Ren3dVec2&>(glm_vec2);
}

static const bstone::Ren3dVec4& cast_glm_vec4(
	const glm::vec4& glm_vec4)
{
	return reinterpret_cast<const bstone::Ren3dVec4&>(glm_vec4);
}

static const bstone::Ren3dMat4& cast_glm_mat4(
	const glm::mat4& glm_mat4)
{
	return reinterpret_cast<const bstone::Ren3dMat4&>(glm_mat4);
}

bstone::Ren3dMipmapMode cfg_texture_mipmap_filter_to_renderer(
	bstone::Ren3dFilterKind filter_kind)
{
	switch (filter_kind)
	{
		case bstone::Ren3dFilterKind::nearest:
			return bstone::Ren3dMipmapMode::nearest;

		case bstone::Ren3dFilterKind::linear:
			return bstone::Ren3dMipmapMode::linear;

		default:
			Quit("Invalid mipmap mode.");
	}
}

int cfg_texture_anisotropy_to_renderer(
	int value)
{
	if (value < bstone::Ren3dLimits::min_anisotropy_off)
	{
		return bstone::Ren3dLimits::min_anisotropy_off;
	}
	else if (value > bstone::Ren3dLimits::max_anisotropy)
	{
		return bstone::Ren3dLimits::max_anisotropy;
	}
	else
	{
		return value;
	}
}

static int get_static_index(
	const statobj_t& bs_static)
{
	return static_cast<int>(&bs_static - statobjlist.data());
}

static int get_actor_index(
	const objtype& bs_actor)
{
	return static_cast<int>(&bs_actor - objlist);
}

constexpr int encode_xy(
	int x,
	int y)
{
	return (x << 8) | y;
}

VertexColor vga_color_to_rgba_8(
	int vga_red,
	int vga_green,
	int vga_blue)
{
	return VertexColor
	{
		static_cast<std::uint8_t>((255 * vga_red) / 63),
		static_cast<std::uint8_t>((255 * vga_green) / 63),
		static_cast<std::uint8_t>((255 * vga_blue) / 63),
		255
	};
}

void destroy_sampler(
	bstone::Ren3dSamplerUPtr& sampler)
{
	sampler = nullptr;
}

void destroy_index_buffer(
	bstone::Ren3dBufferUPtr& index_buffer)
{
	index_buffer = nullptr;
}

bstone::Ren3dBufferUPtr create_index_buffer(
	bstone::Ren3dBufferUsageKind usage_kind,
	int byte_depth,
	int index_count)
{
	const auto index_buffer_size = index_count * byte_depth;

	auto param = bstone::Ren3dCreateBufferParam{};
	param.kind = bstone::Ren3dBufferKind::index;
	param.usage_kind_ = usage_kind;
	param.size_ = index_buffer_size;

	return renderer_->create_buffer(param);
}

template<
	typename TIndex
>
void update_index_buffer(
	const bstone::Ren3dBufferUPtr& index_buffer,
	int index_offset,
	int index_count,
	const TIndex* indices)
{
	const auto byte_depth = static_cast<int>(sizeof(TIndex));
	const auto offset = index_offset * byte_depth;
	const auto size = index_count * byte_depth;

	auto param = bstone::Ren3dUpdateBufferParam{};
	param.offset_ = offset;
	param.size_ = size;
	param.data_ = indices;

	index_buffer->update(param);
}

void destroy_vertex_buffer(
	bstone::Ren3dBufferUPtr& vertex_buffer)
{
	vertex_buffer = nullptr;
}

template<
	typename TVertex
>
bstone::Ren3dBufferUPtr create_vertex_buffer(
	bstone::Ren3dBufferUsageKind usage_kind,
	int vertex_count)
{
	const auto vertex_size = static_cast<int>(sizeof(TVertex));
	const auto vertex_buffer_size = vertex_count * vertex_size;

	auto param = bstone::Ren3dCreateBufferParam{};
	param.kind = bstone::Ren3dBufferKind::vertex;
	param.usage_kind_ = usage_kind;
	param.size_ = vertex_buffer_size;

	return renderer_->create_buffer(param);
}

template<
	typename TVertex
>
void update_vertex_buffer(
	const bstone::Ren3dBufferUPtr& vertex_buffer,
	int vertex_offset,
	int vertex_count,
	const TVertex* vertices)
{
	const auto vertex_size = static_cast<int>(sizeof(TVertex));
	const auto offset = vertex_offset * vertex_size;
	const auto size = vertex_count * vertex_size;

	auto param = bstone::Ren3dUpdateBufferParam{};
	param.offset_ = offset;
	param.size_ = size;
	param.data_ = vertices;

	vertex_buffer->update(param);
}

void destroy_vertex_input(
	bstone::Ren3dVertexInputUPtr& vertex_input)
{
	vertex_input = nullptr;
}

template<
	typename TVertex,
	bool TIsExist = false
>
struct VertexInputAddAttribDescr
{
	void operator()(
		int location,
		bstone::Ren3dVertexAttribFormat format,
		int offset,
		int stride,
		const glm::vec4& default_value,
		const bstone::Ren3dBufferUPtr& vertex_buffer,
		bstone::Ren3dVertexAttribDescrs& attribute_descriptions) const
	{
		attribute_descriptions.emplace_back();

		auto& description = attribute_descriptions.back();
		description.is_default_ = true;
		description.location_ = location;
		description.format_ = bstone::Ren3dVertexAttribFormat::none;
		description.vertex_buffer_ = nullptr;
		description.offset_ = -1;
		description.stride_ = -1;
		description.default_value_ = cast_glm_vec4(default_value);
	}
}; // VertexInputAddAttribDescr

template<
	typename TVertex
>
struct VertexInputAddAttribDescr<TVertex, true>
{
	void operator()(
		int location,
		bstone::Ren3dVertexAttribFormat format,
		int offset,
		int stride,
		const glm::vec4& default_value,
		const bstone::Ren3dBufferUPtr& vertex_buffer,
		bstone::Ren3dVertexAttribDescrs& attribute_descriptions) const
	{
		attribute_descriptions.emplace_back();

		auto& description = attribute_descriptions.back();
		description.is_default_ = false;
		description.location_ = location;
		description.format_ = format;
		description.vertex_buffer_ = vertex_buffer.get();
		description.offset_ = offset;
		description.stride_ = stride;
		description.default_value_ = cast_glm_vec4(default_value);
	}
}; // VertexInputAddAttribDescr

template<
	typename TVertex,
	VertexAttribLocationId TLocationId
>
void add_vertex_input_attrib_descr(
	bstone::Ren3dVertexAttribFormat format,
	const glm::vec4& default_value,
	const bstone::Ren3dBufferUPtr& vertex_buffer,
	bstone::Ren3dVertexAttribDescrs& attribute_descriptions)
{
	const auto traits = VertexAttributeTraits<TVertex, TLocationId>{};
	const auto add_attribute = VertexInputAddAttribDescr<TVertex, traits.is_valid>{};

	auto location = 0;

	switch (TLocationId)
	{
		case VertexAttribLocationId::position:
			location = bstone::HwShaderRegistry::get_a_position_location();
			break;

		case VertexAttribLocationId::color:
			location = bstone::HwShaderRegistry::get_a_color_location();
			break;

		case VertexAttribLocationId::texture_coordinates:
			location = bstone::HwShaderRegistry::get_a_tx_coords_location();
			break;
	}

	add_attribute(
		location,
		format,
		traits.offset,
		traits.stride,
		default_value,
		vertex_buffer,
		attribute_descriptions
	);
}

template<
	typename TVertex
>
void create_vertex_input(
	const bstone::Ren3dBufferUPtr& index_buffer,
	const bstone::Ren3dBufferUPtr& vertex_buffer,
	bstone::Ren3dVertexInputUPtr& vertex_input)
{
	auto param = bstone::Ren3dCreateVertexInputParam{};
	param.index_buffer_ = index_buffer.get();

	auto& descriptions = param.attrib_descrs_;
	descriptions.reserve(3);

	add_vertex_input_attrib_descr<TVertex, VertexAttribLocationId::position>(
		bstone::Ren3dVertexAttribFormat::rgb_32_sfloat,
		glm::vec4{},
		vertex_buffer,
		descriptions
	);

	add_vertex_input_attrib_descr<TVertex, VertexAttribLocationId::color>(
		bstone::Ren3dVertexAttribFormat::rgba_8_unorm,
		glm::vec4{1.0F, 1.0F, 1.0F, 1.0F},
		vertex_buffer,
		descriptions
	);

	add_vertex_input_attrib_descr<TVertex, VertexAttribLocationId::texture_coordinates>(
		bstone::Ren3dVertexAttribFormat::rg_32_sfloat,
		glm::vec4{},
		vertex_buffer,
		descriptions
	);

	vertex_input = renderer_->create_vertex_input(param);
}

void update_player_direction()
{
	player_angle_rad_ = bstone::math::deg_to_rad(player->angle);

	view_direction_.x = std::cos(player_angle_rad_);
	view_direction_.y = -std::sin(player_angle_rad_);

	bs_view_direction_ = view_direction_;
}

void update_player_position()
{
	player_position_.x = player->x;
	player_position_.y = player->y;
}

void update_player_view_position()
{
	const auto focal_delta = glm::dvec2
	{
		view_direction_.x * focallength,
		view_direction_.y * focallength,
	};

	view_position_ = glm::dvec3{player_position_ - focal_delta, 0.5};

	bs_view_position_ = view_position_;
}

void update_player()
{
	update_player_direction();
	update_player_position();
	update_player_view_position();
}

void initialize_ui_buffer()
{
	vid_initialize_ui_buffer();
}

void destroy_shader(
	bstone::Ren3dShaderUPtr& shader)
{
	shader = nullptr;
}

void create_shader(
	bstone::Ren3dShaderKind kind,
	bstone::Ren3dShaderUPtr& shader)
{
	auto param = bstone::Ren3dCreateShaderParam{};
	param.kind = kind;

	const auto renderer_kind = renderer_->get_kind();

	switch (kind)
	{
		case bstone::Ren3dShaderKind::fragment:
			param.source_ = bstone::HwShaderRegistry::get_fragment(renderer_kind);
			break;

		case bstone::Ren3dShaderKind::vertex:
			param.source_ = bstone::HwShaderRegistry::get_vertex(renderer_kind);
			break;

		default:
			Quit("Unsupported shader kind.");
	}

	shader = renderer_->create_shader(param);
}

void destroy_fragment_shader()
{
	destroy_shader(fragment_shader_);
}

void create_fragment_shader()
{
	create_shader(bstone::Ren3dShaderKind::fragment, fragment_shader_);
}

void destroy_vertex_shader()
{
	destroy_shader(vertex_shader_);
}

void create_vertex_shader()
{
	create_shader(bstone::Ren3dShaderKind::vertex, vertex_shader_);
}

void destroy_shader_stage()
{
	shader_stage_ = nullptr;
}

void create_shader_stage()
{
	static const auto input_bindings = bstone::Ren3dShaderStageInputBindings
	{
		{0, bstone::HwShaderRegistry::get_a_position_name()},
		{1, bstone::HwShaderRegistry::get_a_color_name()},
		{2, bstone::HwShaderRegistry::get_a_tx_coords_name()},
	};

	auto param = bstone::Ren3dCreateShaderStageParam{};
	param.fragment_shader_ = fragment_shader_.get();
	param.vertex_shader_ = vertex_shader_.get();
	param.input_bindings_ = input_bindings;

	shader_stage_ = renderer_->create_shader_stage(param);
}

template<
	typename T
>
void uninitialize_uniform(
	T*& var)
{
	var = nullptr;
}

void uninitialize_model_mat_uniform()
{
	uninitialize_uniform(model_mat_uniform_);
}

struct InitializeInt32UniformTag{};
struct InitializeFloat32UniformTag{};
struct InitializeVec2UniformTag{};
struct InitializeMat4UniformTag{};
struct InitializeSampler2dUniformTag{};

void initialize_uniform(
	const std::string& name,
	bstone::Ren3dShaderInt32VarPtr& var,
	InitializeInt32UniformTag)
{
	var = shader_stage_->find_int32_var(name);
}

void initialize_uniform(
	const std::string& name,
	bstone::Ren3dShaderFloat32VarPtr& var,
	InitializeFloat32UniformTag)
{
	var = shader_stage_->find_float32_var(name);
}

void initialize_uniform(
	const std::string& name,
	bstone::Ren3dShaderVec2VarPtr& var,
	InitializeVec2UniformTag)
{
	var = shader_stage_->find_vec2_var(name);
}

void initialize_uniform(
	const std::string& name,
	bstone::Ren3dShaderMat4VarPtr& var,
	InitializeMat4UniformTag)
{
	var = shader_stage_->find_mat4_var(name);
}

void initialize_uniform(
	const std::string& name,
	bstone::Ren3dShaderSampler2dVarPtr& var,
	InitializeSampler2dUniformTag)
{
	var = shader_stage_->find_sampler_2d_var(name);
}

template<
	typename T
>
void initialize_uniform(
	const std::string& name,
	T*& var)
{
	using Tag = std::conditional_t<
		std::is_same<T, bstone::Ren3dShaderVarInt32>::value,
		InitializeInt32UniformTag,
		std::conditional_t<
			std::is_same<T, bstone::Ren3dShaderVarFloat32>::value,
			InitializeFloat32UniformTag,
			std::conditional_t<
				std::is_same<T, bstone::Ren3dShaderVarVec2>::value,
				InitializeVec2UniformTag,
				std::conditional_t<
					std::is_same<T, bstone::Ren3dShaderVarMat4>::value,
					InitializeMat4UniformTag,
					std::conditional_t<
						std::is_same<T, bstone::Ren3dShaderVarSampler2d>::value,
						InitializeSampler2dUniformTag,
						void
					>
				>
			>
		>
	>;

	static_assert(!std::is_same<Tag, void>::value, "Unsupported type.");

	initialize_uniform(name, var, Tag{});

	if (!var)
	{
		fail("Shader variable \"" + name + "\" not found.");
	}
}

void initialize_model_mat_uniform()
{
	initialize_uniform(
		bstone::HwShaderRegistry::get_u_model_mat_name(),
		model_mat_uniform_
	);
}

void uninitialize_view_mat_uniform()
{
	uninitialize_uniform(view_mat_uniform_);
}

void initialize_view_mat_uniform()
{
	initialize_uniform(
		bstone::HwShaderRegistry::get_u_view_mat_name(),
		view_mat_uniform_
	);
}

void uninitialize_projection_mat_uniform()
{
	uninitialize_uniform(projection_mat_uniform_);
}

void initialize_projection_mat_uniform()
{
	initialize_uniform(
		bstone::HwShaderRegistry::get_u_projection_mat_name(),
		projection_mat_uniform_
	);
}

void uninitialize_sampler_uniform()
{
	uninitialize_uniform(sampler_uniform_);
}

void initialize_sampler_uniform()
{
	sampler_var_.set_is_modified(true);

	initialize_uniform(
		bstone::HwShaderRegistry::get_u_sampler_name(),
		sampler_uniform_
	);
}

void uninitialize_shading_mode_uniform()
{
	uninitialize_uniform(shading_mode_uniform_);
}

void initialize_shading_mode_uniform()
{
	shading_mode_.set_is_modified(true);

	initialize_uniform(
		bstone::HwShaderRegistry::get_u_shading_mode_name(),
		shading_mode_uniform_
	);
}

void uninitialize_shade_max_uniform()
{
	uninitialize_uniform(shade_max_uniform_);
}

void initialize_shade_max_uniform()
{
	bs_shade_max_.set_is_modified(true);

	initialize_uniform(
		bstone::HwShaderRegistry::get_u_shade_max_name(),
		shade_max_uniform_
	);
}

void uninitialize_normal_shade_uniform()
{
	uninitialize_uniform(normal_shade_uniform_);
}

void initialize_normal_shade_uniform()
{
	bs_normal_shade_.set_is_modified(true);

	initialize_uniform(
		bstone::HwShaderRegistry::get_u_normal_shade_name(),
		normal_shade_uniform_
	);
}

void uninitialize_height_numerator_uniform()
{
	uninitialize_uniform(height_numerator_uniform_);
}

void initialize_height_numerator_uniform()
{
	bs_height_numerator_.set_is_modified(true);

	initialize_uniform(
		bstone::HwShaderRegistry::get_u_height_numerator_name(),
		height_numerator_uniform_
	);
}

void uninitialize_extra_lighting_uniform()
{
	uninitialize_uniform(extra_lighting_uniform_);
}

void initialize_extra_lighting_uniform()
{
	bs_lighting_.set_is_modified(true);

	initialize_uniform(
		bstone::HwShaderRegistry::get_u_extra_lighting_name(),
		extra_lighting_uniform_
	);
}

void uninitialize_view_direction_uniform()
{
	uninitialize_uniform(view_direction_uniform_);
}

void initialize_view_direction_uniform()
{
	bs_view_direction_.set_is_modified(true);

	initialize_uniform(
		bstone::HwShaderRegistry::get_u_view_direction_name(),
		view_direction_uniform_
	);
}

void uninitialize_view_position_uniform()
{
	uninitialize_uniform(view_position_uniform_);
}

void initialize_view_position_uniform()
{
	bs_view_position_.set_is_modified(true);

	initialize_uniform(
		bstone::HwShaderRegistry::get_u_view_position_name(),
		view_position_uniform_
	);
}

void uninitialize_shading_uniforms()
{
	uninitialize_sampler_uniform();
	uninitialize_shading_mode_uniform();
	uninitialize_shade_max_uniform();
	uninitialize_normal_shade_uniform();
	uninitialize_height_numerator_uniform();
	uninitialize_extra_lighting_uniform();
	uninitialize_view_direction_uniform();
	uninitialize_view_position_uniform();
}

void initialize_shading_uniforms()
{
	initialize_sampler_uniform();
	initialize_shading_mode_uniform();
	initialize_shade_max_uniform();
	initialize_normal_shade_uniform();
	initialize_height_numerator_uniform();
	initialize_extra_lighting_uniform();
	initialize_view_direction_uniform();
	initialize_view_position_uniform();
}

void uninitialize_uniforms()
{
	uninitialize_model_mat_uniform();
	uninitialize_view_mat_uniform();
	uninitialize_projection_mat_uniform();
	uninitialize_shading_uniforms();
}

void initialize_uniforms()
{
	log("Initializing shader variables.");

	initialize_model_mat_uniform();
	initialize_view_mat_uniform();
	initialize_projection_mat_uniform();
	initialize_shading_uniforms();
}

void uninitialize_program()
{
	uninitialize_uniforms();
	destroy_shader_stage();
	destroy_fragment_shader();
	destroy_vertex_shader();
}

void initialize_program()
{
	log("");
	log("Initializing shader program.");

	create_fragment_shader();
	create_vertex_shader();
	create_shader_stage();
	initialize_uniforms();
}

bstone::Ren3dKind get_renderer_kind(
	bstone::RendererKind renderer_kind)
{
	switch (renderer_kind)
	{
		case bstone::RendererKind::gl_2_0:
			return bstone::Ren3dKind::gl_2_0;

		case bstone::RendererKind::gl_3_2_core:
			return bstone::Ren3dKind::gl_3_2_core;

		case bstone::RendererKind::gles_2_0:
			return bstone::Ren3dKind::gles_2_0;

		default:
			throw bstone::Exception{"Unsupported renderer kind."};
	}
}

void log_device_features()
{
	const auto& device_features = renderer_->get_device_features();

	log("");
	log("Device features");
	log("===============");
	log("V-Sync: " + vid_to_string(device_features.is_vsync_available_));
	log("V-Sync requires restart: " + vid_to_string(device_features.is_vsync_requires_restart_));
	log("Texture max dimension: " + vid_to_string(device_features.max_texture_dimension_));
	log("Viewport max width: " + vid_to_string(device_features.max_viewport_width_));
	log("Viewport max height: " + vid_to_string(device_features.max_viewport_height_));
	log("Anisotropy: " + vid_to_string(device_features.is_anisotropy_available_));
	log("Anisotropy max degree: " + vid_to_string(device_features.max_anisotropy_degree_));
	log("Non-power-of-two textures: " + vid_to_string(device_features.is_npot_available_));
	log("Mipmap auto-generation: " + vid_to_string(device_features.is_mipmap_available_));
	log("Samplers: " + vid_to_string(device_features.is_mipmap_available_));
	log("MSAA: " + vid_to_string(device_features.is_msaa_available_));
	log("MSAA (render-to-window): " + vid_to_string(device_features.is_msaa_render_to_window_));
	log("MSAA requires restart: " + vid_to_string(device_features.is_msaa_requires_restart_));
	log("MSAA max degree: " + vid_to_string(device_features.max_msaa_degree_));
	log("Vertex input max locations: " + vid_to_string(device_features.max_vertex_input_locations_));
}

void initialize_renderer()
{
	log("");
	log("Initializing 3D renderer.");


	const auto title = vid_get_game_name_and_game_version_string();


	// Initialization parameter.
	//
	auto param = bstone::Ren3dCreateParam{};

	param.aa_kind_ = vid_cfg_get().aa_kind_;
	param.aa_value_ = vid_cfg_get().aa_degree_;

	param.is_vsync_ = vid_cfg_get().is_vsync_;

#ifdef __vita__
	param.window_.is_visible = true;
#endif // __vita__

	param.window_.rect_2d_.extent_.width_ = vid_layout_.window_width;
	param.window_.rect_2d_.extent_.height_ = vid_layout_.window_height;

	if (!vid_cfg_get().is_windowed)
	{
		param.window_.is_borderless_ = true;
		param.window_.is_fake_fullscreen_ = true;
	}

	param.window_.is_positioned_ = vid_cfg_get().is_positioned_;
	param.window_.rect_2d_.offset_.x = vid_cfg_get().windowed_x_;
	param.window_.rect_2d_.offset_.y = vid_cfg_get().windowed_y_;

	param.window_.title_ = title;

	using RendererKindList = std::vector<bstone::Ren3dKind>;

	auto renderer_kind_list = RendererKindList{};

	const auto is_auto_detect = (vid_cfg_get().renderer_kind_ == bstone::RendererKind::auto_detect);

	if (is_auto_detect)
	{
#ifndef BSTONE_REN_3D_TEST_NO_GL
		renderer_kind_list =
		{
#ifndef BSTONE_REN_3D_TEST_NO_GL_3_2_C
			bstone::Ren3dKind::gl_3_2_core,
#endif // !BSTONE_REN_3D_TEST_NO_GL_3_2_C

#ifndef BSTONE_REN_3D_TEST_NO_GL_2_0
			bstone::Ren3dKind::gl_2_0,
#endif // !BSTONE_REN_3D_TEST_NO_GL_2_0

#ifndef BSTONE_REN_3D_TEST_NO_GLES_2_0
			bstone::Ren3dKind::gles_2_0,
#endif // !BSTONE_REN_3D_TEST_NO_GLES_2_0
		};
#endif // BSTONE_REN_3D_TEST_NO_GL
	}
	else
	{
		renderer_kind_list = {get_renderer_kind(vid_cfg_get().renderer_kind_)};
	}

	for (const auto renderer_kind : renderer_kind_list)
	{
		param.renderer_kind_ = renderer_kind;

		log("Trying to initialize \"" + vid_to_string(renderer_kind) + "\".");

		try
		{
			renderer_ = renderer_mgr_->renderer_initialize(param);

			return;
		}
		catch (const bstone::Exception& ex)
		{
			log_error(ex.what());
		}
	}

	throw bstone::Exception{"Not found any 3D renderer."};
}

void destroy_ui_ib()
{
	destroy_index_buffer(ui_ib_);
}

void create_ui_ib()
{
	log("Creating UI index buffer.");

	ui_ib_ = create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_static,
		1,
		r2_index_count_
	);


	using Indices = std::array<std::uint8_t, r2_index_count_>;

	const auto indices = Indices
	{
		// Stretched quad.
		//
		(4 * 0) + 0, (4 * 0) + 1, (4 * 0) + 2,
		(4 * 0) + 0, (4 * 0) + 2, (4 * 0) + 3,

		// Non-stretched quad.
		//
		(4 * 1) + 0, (4 * 1) + 1, (4 * 1) + 2,
		(4 * 1) + 0, (4 * 1) + 2, (4 * 1) + 3,
	};

	update_index_buffer(
		ui_ib_,
		0,
		r2_index_count_,
		indices.data());
}

void destroy_ui_vi()
{
	destroy_vertex_input(ui_vi_);
}

void create_ui_vi()
{
	log("Creating UI vertex input.");

	create_vertex_input<Vertex>(
		ui_ib_,
		ui_vb_,
		ui_vi_);
}

void fill_2d_vb(
	float left_f,
	float right_f,
	float width_f,
	int vertex_offset)
{
	static_cast<void>(width_f);

	auto vertex_index = vertex_offset;
	auto& vertices = r2_vertices_;

	const auto height_f = static_cast<float>(vid_layout_.screen_height);

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_f, 0.0F, 0.0F};
		vertex.rgba8 = VertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_f, 0.0F, 0.0F};
		vertex.rgba8 = VertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv = VertexTextureCoordinates{1.0F, 0.0F};
	}

	// Upper right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_f, height_f, 0.0F};
		vertex.rgba8 = VertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv = VertexTextureCoordinates{1.0F, 1.0F};
	}

	// Upper left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_f, height_f, 0.0F};
		vertex.rgba8 = VertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv = VertexTextureCoordinates{0.0F, 1.0F};
	}
}

void fill_2d_vb_stretched()
{
	const auto left_f = 0.0F;
	const auto right_f = static_cast<float>(vid_layout_.screen_width);
	const auto width_f = static_cast<float>(vid_layout_.screen_width);

	fill_2d_vb(left_f, right_f, width_f, r2_stretched_vertex_offset_);
}

void fill_2d_vb_non_stretched()
{
	const auto left_f = static_cast<float>(vid_layout_.screen_left_filler_width);
	const auto right_f = static_cast<float>(vid_layout_.screen_width - vid_layout_.screen_right_filler_width);
	const auto width_f = static_cast<float>(vid_layout_.screen_width_4x3);

	fill_2d_vb(left_f, right_f, width_f, r2_non_stretched_vertex_offset_);
}

void destroy_ui_vb()
{
	destroy_vertex_buffer(ui_vb_);
}

void create_ui_vb()
{
	log("Creating UI vertex buffer.");

	ui_vb_ = create_vertex_buffer<Vertex>(
		bstone::Ren3dBufferUsageKind::draw_static,
		r2_vertex_count_
	);

	fill_2d_vb_stretched();
	fill_2d_vb_non_stretched();

	update_vertex_buffer(
		ui_vb_,
		0,
		r2_vertex_count_,
		r2_vertices_.data()
	);
}

void destroy_2d_fillers_ib()
{
	destroy_index_buffer(r2_fillers_ib_);
}

void create_2d_fillers_ib()
{
	log("Creating 2D filler index buffer.");

	r2_fillers_ib_ = create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_static,
		1,
		r2_fillers_index_count_
	);

	using Indices = std::array<std::uint8_t, r2_fillers_index_count_>;

	const auto& indices = Indices
	{
		// Vertical left.
		//
		(4 * 0) + 0, (4 * 0) + 1, (4 * 0) + 2,
		(4 * 0) + 0, (4 * 0) + 2, (4 * 0) + 3,

		// Vertical right.
		//
		(4 * 1) + 0, (4 * 1) + 1, (4 * 1) + 2,
		(4 * 1) + 0, (4 * 1) + 2, (4 * 1) + 3,

		// Bottom left.
		//
		(4 * 2) + 0, (4 * 2) + 1, (4 * 2) + 2,
		(4 * 2) + 0, (4 * 2) + 2, (4 * 2) + 3,

		// Bottom right.
		//
		(4 * 3) + 0, (4 * 3) + 1, (4 * 3) + 2,
		(4 * 3) + 0, (4 * 3) + 2, (4 * 3) + 3,

		// Top right.
		//
		(4 * 4) + 0, (4 * 4) + 1, (4 * 4) + 2,
		(4 * 4) + 0, (4 * 4) + 2, (4 * 4) + 3,

		// Top left.
		//
		(4 * 5) + 0, (4 * 5) + 1, (4 * 5) + 2,
		(4 * 5) + 0, (4 * 5) + 2, (4 * 5) + 3,
	};

	update_index_buffer(
		r2_fillers_ib_,
		0,
		r2_fillers_index_count_,
		indices.data());
}

void destroy_2d_fillers_vb()
{
	destroy_vertex_buffer(r2_fillers_vb_);
}

void create_2d_fillers_vb()
{
	log("Creating 2D filler vertex buffer.");

	r2_fillers_vb_ = create_vertex_buffer<Vertex>(
		bstone::Ren3dBufferUsageKind::draw_static,
		r2_fillers_vertex_count_
	);

	const auto& filler_color = vga_color_to_rgba_8(
		vgapal[(vid_cfg_get().filler_color_index * 3) + 0],
		vgapal[(vid_cfg_get().filler_color_index * 3) + 1],
		vgapal[(vid_cfg_get().filler_color_index * 3) + 2]
	);

	const auto left_left_f = static_cast<float>(0.0F);
	const auto left_right_f = static_cast<float>(vid_layout_.screen_left_filler_width);

	const auto right_left_f = static_cast<float>(vid_layout_.screen_width - vid_layout_.screen_right_filler_width);
	const auto right_right_f = static_cast<float>(vid_layout_.screen_width);

	const auto top_top_f = static_cast<float>(vid_layout_.screen_height);
	const auto top_bottom_f = static_cast<float>(vid_layout_.screen_height - vid_layout_.screen_top_filler_height);

	const auto bottom_top_f = static_cast<float>(vid_layout_.screen_bottom_filler_height);
	const auto bottom_bottom_f = static_cast<float>(0.0F);

	auto vertex_index = 0;

	using R2FillersVertices = std::array<Vertex, r2_fillers_vertex_count_>;
	auto vertices = R2FillersVertices{};


	// ======================================================================
	// UI fillers.
	//

	// ----------------------------------------------------------------------
	// Vertical left.
	//
	// +--+============+--+
	// |xx|            |  |
	// |xx|            |  |
	// |xx|            |  |
	// |xx|            |  |
	// |xx|            |  |
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_right_f, top_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_left_f, top_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 1.0F};
	}

	//
	// Vertical left.
	// ----------------------------------------------------------------------


	// ----------------------------------------------------------------------
	// Vertical right.
	//
	// +--+============+--+
	// |  |            |xx|
	// |  |            |xx|
	// |  |            |xx|
	// |  |            |xx|
	// |  |            |xx|
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_right_f, top_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_left_f, top_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 1.0F};
	}

	//
	// Vertical right.
	// ----------------------------------------------------------------------

	//
	// UI fillers.
	// ======================================================================


	// ======================================================================
	// HUD fillers.
	//

	// ----------------------------------------------------------------------
	// Bottom left.
	//
	// +--+============+--+
	// |  |            |  |
	// +--+            +--+
	// |  |            |  |
	// +--+            +--+
	// |xx|            |  |
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_right_f, bottom_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_left_f, bottom_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 1.0F};
	}

	//
	// Bottom left.
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// Bottom right.
	//
	// +--+============+--+
	// |  |            |  |
	// +--+            +--+
	// |  |            |  |
	// +--+            +--+
	// |  |            |xx|
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_left_f, bottom_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_right_f, bottom_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_right_f, bottom_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_left_f, bottom_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 1.0F};
	}

	//
	// Bottom right.
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// Top right.
	//
	// +--+============+--+
	// |  |            |xx|
	// +--+            +--+
	// |  |            |  |
	// +--+            +--+
	// |  |            |  |
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_left_f, top_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_right_f, top_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_right_f, top_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{right_left_f, top_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 1.0F};
	}

	//
	// Top right.
	// ----------------------------------------------------------------------

	// ----------------------------------------------------------------------
	// Top left.
	//
	// +--+============+--+
	// |xx|            |  |
	// +--+            +--+
	// |  |            |  |
	// +--+            +--+
	// |  |            |  |
	// +--+============+--+
	//

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_left_f, top_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_right_f, top_bottom_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_right_f, top_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{left_left_f, top_top_f, 0.0F};
		vertex.rgba8 = filler_color;
		vertex.uv = VertexTextureCoordinates{0.0F, 1.0F};
	}

	//
	// Top left.
	// ----------------------------------------------------------------------

	//
	// HUD fillers.
	// ======================================================================


	update_vertex_buffer(
		r2_fillers_vb_,
		0,
		r2_fillers_vertex_count_,
		vertices.data()
	);
}

void destroy_2d_fillers_vi()
{
	destroy_vertex_input(r2_fillers_vi_);
}

void create_2d_fillers_vi()
{
	log("Creating 2D filler vertex input.");

	create_vertex_input<Vertex>(
		r2_fillers_ib_,
		r2_fillers_vb_,
		r2_fillers_vi_
	);
}

void destroy_black_1x1_2d_texture()
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::black);
	}

	r2_black_t2d_1x1_ = nullptr;
}

void create_black_1x1_2d_texture()
{
	log("Creating 2D 1x1 black texture.");

	texture_mgr_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::black);
	r2_black_t2d_1x1_ = texture_mgr_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::black);
}

void destroy_white_1x1_2d_texture()
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::white);
	}

	r2_white_t2d_1x1_ = nullptr;
}

void create_white_1x1_2d_texture()
{
	log("Creating 2D 1x1 white texture.");

	texture_mgr_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::white);
	r2_white_t2d_1x1_ = texture_mgr_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::white);
}

void destroy_fade_1x1_2d_texture()
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_2d);
	}

	r2_fade_t2d_ = nullptr;
}

void create_fade_1x1_2d_texture()
{
	log("Creating 2D 1x1 fade texture.");

	texture_mgr_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_2d);
	r2_fade_t2d_ = texture_mgr_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_2d);
}

void destroy_ui_texture()
{
	if (ui_t2d_ == nullptr)
	{
		return;
	}

	texture_mgr_->destroy_ui();
	ui_t2d_ = nullptr;
}

void create_ui_texture()
{
	log("Creating UI texture.");

	texture_mgr_->create_ui(vid_ui_buffer_.data(), vid_mask_buffer_.data(), &palette_);
	ui_t2d_ = texture_mgr_->get_ui();
}

void uninitialize_2d()
{
	destroy_ui_texture();

	destroy_ui_vi();
	destroy_ui_ib();
	destroy_ui_vb();

	destroy_2d_fillers_vi();
	destroy_2d_fillers_ib();
	destroy_2d_fillers_vb();

	destroy_black_1x1_2d_texture();
	destroy_white_1x1_2d_texture();
	destroy_fade_1x1_2d_texture();
}

void initialize_2d()
{
	log("");
	log("Initializing 2D resources.");

	create_ui_ib();
	create_ui_vb();
	create_ui_vi();

	create_2d_fillers_ib();
	create_2d_fillers_vb();
	create_2d_fillers_vi();

	create_ui_texture();
	create_black_1x1_2d_texture();
	create_white_1x1_2d_texture();
	create_fade_1x1_2d_texture();
}

void destroy_flooring_ib()
{
	destroy_index_buffer(flooring_ib_);
}

void create_flooring_ib()
{
	log("Creating 3D flooring index buffer.");

	const auto index_count = 6;

	{
		flooring_ib_ = create_index_buffer(
			bstone::Ren3dBufferUsageKind::draw_static,
			1,
			index_count
		);
	}

	{
		using Indices = std::array<std::uint8_t, index_count>;

		const auto& indices = Indices
		{
			0, 1, 2,
			0, 2, 3,
		};

		update_index_buffer(
			flooring_ib_,
			0,
			index_count,
			indices.data()
		);
	}
}

void destroy_flooring_vb()
{
	destroy_vertex_buffer(flooring_vb_);
}

void create_flooring_vb()
{
	log("Creating 3D flooring vertex buffer.");

	const auto vertex_count = 4;

	{
		flooring_vb_ = create_vertex_buffer<FlooringVertex>(
			bstone::Ren3dBufferUsageKind::draw_static,
			vertex_count
		);
	}

	{
		const auto map_dimension_f = static_cast<float>(MAPSIZE);

		using Vertices = std::array<FlooringVertex, vertex_count>;

		auto vertices = Vertices{};

		auto vertex_index = 0;

		// Bottom-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz = VertexPosition{0.0F, 0.0F, 0.0F};
			vertex.uv = VertexTextureCoordinates{0.0F, map_dimension_f};
		}

		// Bottom-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz = VertexPosition{0.0F, map_dimension_f, 0.0F};
			vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
		}

		// Top-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz = VertexPosition{map_dimension_f, map_dimension_f, 0.0F};
			vertex.uv = VertexTextureCoordinates{map_dimension_f, 0.0F};
		}

		// Top-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz = VertexPosition{map_dimension_f, 0.0F, 0.0F};
			vertex.uv = VertexTextureCoordinates{map_dimension_f, map_dimension_f};
		}

		update_vertex_buffer(
			flooring_vb_,
			0,
			vertex_count,
			vertices.data()
		);
	}
}

void destroy_flooring_vi()
{
	destroy_vertex_input(flooring_vi_);
}

void create_flooring_vi()
{
	log("Creating 3D flooring vertex input.");

	create_vertex_input<FlooringVertex>(
		flooring_ib_,
		flooring_vb_,
		flooring_vi_);
}

void destroy_solid_flooring_texture_2d()
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::flooring);
	}

	flooring_solid_t2d_ = nullptr;
}

void create_solid_flooring_texture_2d()
{
	log("Creating 3D flooring 1x1 solid texture.");

	texture_mgr_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::flooring);
	flooring_solid_t2d_ = texture_mgr_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::flooring);
}

void uninitialize_flooring()
{
	destroy_flooring_vi();
	destroy_flooring_ib();
	destroy_flooring_vb();

	destroy_solid_flooring_texture_2d();

	// Managed by texture manager. No need to destroy.
	flooring_textured_t2d_ = nullptr;
}

void initialize_flooring()
{
	log("");
	log("Initializing 3D flooring.");

	create_flooring_ib();
	create_flooring_vb();
	create_flooring_vi();
	create_solid_flooring_texture_2d();
}

void destroy_ceiling_ib()
{
	destroy_index_buffer(ceiling_ib_);
}

void create_ceiling_ib()
{
	log("Creating 3D ceiling index buffer.");

	const auto index_count = 6;

	{
		ceiling_ib_ = create_index_buffer(
			bstone::Ren3dBufferUsageKind::draw_static,
			1,
			index_count
		);
	}

	{
		using Indices = std::array<std::uint8_t, index_count>;

		const auto& indices = Indices
		{
			0, 2, 1,
			0, 3, 2,
		};

		update_index_buffer(
			ceiling_ib_,
			0,
			index_count,
			indices.data()
		);
	}
}

void destroy_ceiling_vb()
{
	destroy_vertex_buffer(ceiling_vb_);
}

void create_ceiling_vb()
{
	log("Creating 3D ceiling vertex buffer.");

	const auto vertex_count = 4;

	{
		ceiling_vb_ = create_vertex_buffer<CeilingVertex>(
			bstone::Ren3dBufferUsageKind::draw_static,
			vertex_count
		);
	}

	{
		using Vertices = std::array<CeilingVertex, vertex_count>;

		auto vertices = Vertices{};

		auto vertex_index = 0;

		// Bottom-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz = VertexPosition{0.0F, 0.0F, map_height_f};
			vertex.uv = VertexTextureCoordinates{0.0F, map_dimension_f};
		}

		// Bottom-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz = VertexPosition{0.0F, map_dimension_f, map_height_f};
			vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
		}

		// Top-right.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz = VertexPosition{map_dimension_f, map_dimension_f, map_height_f};
			vertex.uv = VertexTextureCoordinates{map_dimension_f, 0.0F};
		}

		// Top-left.
		{
			auto& vertex = vertices[vertex_index++];
			vertex.xyz = VertexPosition{map_dimension_f, 0.0F, map_height_f};
			vertex.uv = VertexTextureCoordinates{map_dimension_f, map_dimension_f};
		}

		update_vertex_buffer(
			ceiling_vb_,
			0,
			vertex_count,
			vertices.data()
		);
	}
}

void destroy_ceiling_vi()
{
	destroy_vertex_input(ceiling_vi_);
}

void create_ceiling_vi()
{
	log("Creating 3D ceiling vertex input.");

	create_vertex_input<CeilingVertex>(
		ceiling_ib_,
		ceiling_vb_,
		ceiling_vi_);
}

void destroy_solid_ceiling_texture_2d()
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::ceiling);
	}

	ceiling_solid_t2d_ = nullptr;
}

void create_solid_ceiling_texture_2d()
{
	texture_mgr_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::ceiling);
	ceiling_solid_t2d_ = texture_mgr_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::ceiling);
}

void initialize_ceiling()
{
	log("");
	log("Initializing 3D ceiling.");

	create_ceiling_ib();
	create_ceiling_vb();
	create_ceiling_vi();
	create_solid_ceiling_texture_2d();
}

void uninitialize_ceiling()
{
	destroy_ceiling_vi();
	destroy_ceiling_ib();
	destroy_ceiling_vb();

	destroy_solid_ceiling_texture_2d();

	// Managed by texture manager. No need to destroy.
	ceiling_textured_t2d_ = nullptr;
}

void create_walls_ib()
{
	log("Creating wall index buffer.");

	const auto index_count = wall_side_count_ * indices_per_wall_side;

	wall_sides_ib_ = create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_streaming,
		2,
		index_count
	);

	wall_sides_ibi_.clear();
	wall_sides_ibi_.resize(index_count);
}

void destroy_walls_ib()
{
	destroy_index_buffer(wall_sides_ib_);
	wall_sides_ibi_.clear();
}

void create_walls_vb()
{
	log("Creating wall vertex buffer.");

	const auto vertex_count = wall_side_count_ * vertices_per_wall_side;

	wall_sides_vb_ = create_vertex_buffer<WallVertex>(
		bstone::Ren3dBufferUsageKind::draw_static,
		vertex_count
	);
}

void destroy_walls_vi()
{
	destroy_vertex_input(wall_sides_vi_);
}

void create_walls_vi()
{
	log("Creating wall vertex input.");

	create_vertex_input<WallVertex>(
		wall_sides_ib_,
		wall_sides_vb_,
		wall_sides_vi_);
}

void destroy_walls_vb()
{
	destroy_vertex_buffer(wall_sides_vb_);
}

void initialize_walls()
{
	log("");
	log("Initializing walls.");

	xy_wall_map_.reserve(wall_count_);

	walls_to_render_.clear();

	wall_side_draw_item_count_ = 0;
	wall_side_draw_items_.clear();
	wall_side_draw_items_.resize(wall_side_count_);

	create_walls_ib();
	create_walls_vb();
	create_walls_vi();
}

void uninitialize_walls()
{
	wall_count_ = 0;
	wall_side_count_ = 0;
	xy_wall_map_.clear();

	wall_side_draw_item_count_ = 0;
	wall_side_draw_items_.clear();

	destroy_walls_ib();
	destroy_walls_vb();
	destroy_walls_vi();
}

void create_pushwalls_ibi()
{
	const auto index_count = pushwall_side_count_ * indices_per_wall_side;

	pushwall_sides_ibi_.clear();
	pushwall_sides_ibi_.resize(index_count);
}

void create_pushwalls_ib()
{
	log("Creating pushwall index buffer.");

	const auto index_count = pushwall_side_count_ * indices_per_wall_side;

	pushwall_sides_ib_ = create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_streaming,
		2,
		index_count
	);
}

void destroy_pushwalls_ibi()
{
	pushwall_sides_ibi_.clear();
}

void destroy_pushwalls_ib()
{
	destroy_index_buffer(pushwall_sides_ib_);
}

void create_pushwalls_vb()
{
	log("Creating pushwall vertex buffer.");

	const auto vertex_count = pushwall_side_count_ * vertices_per_wall_side;

	pushwall_sides_vb_ = create_vertex_buffer<PushwallVertex>(
		bstone::Ren3dBufferUsageKind::draw_dynamic,
		vertex_count
	);
}

void destroy_pushwalls_vi()
{
	destroy_vertex_input(pushwall_sides_vi_);
}

void create_pushwalls_vi()
{
	log("Creating pushwall vertex input.");

	create_vertex_input<PushwallVertex>(
		pushwall_sides_ib_,
		pushwall_sides_vb_,
		pushwall_sides_vi_);
}

void destroy_pushwalls_vb()
{
	destroy_vertex_buffer(pushwall_sides_vb_);
}

void initialize_pushwalls()
{
	log("");
	log("Initializing pushwalls.");

	xy_pushwall_map_.reserve(pushwall_count_);

	pushwall_to_wall_vbi_.clear();
	pushwall_to_wall_vbi_.resize(sides_per_wall * vertices_per_wall_side);

	pushwalls_to_render_.clear();

	pushwall_side_draw_item_count_ = 0;
	pushwall_side_draw_items_.clear();
	pushwall_side_draw_items_.resize(pushwall_side_count_);

	create_pushwalls_ibi();
	create_pushwalls_ib();
	create_pushwalls_vb();
	create_pushwalls_vi();
}

void uninitialize_pushwalls()
{
	pushwall_count_ = 0;
	pushwall_side_count_ = 0;
	xy_pushwall_map_.clear();

	pushwall_side_draw_item_count_ = 0;
	pushwall_side_draw_items_.clear();

	destroy_pushwalls_vi();

	destroy_pushwalls_ibi();
	destroy_pushwalls_ib();

	destroy_pushwalls_vb();
}

void create_door_sides_ibi()
{
	const auto index_count = door_count_ * indices_per_door_side;

	door_sides_ibi_.clear();
	door_sides_ibi_.resize(index_count);
}

void create_door_sides_ib()
{
	log("Creating door index buffer.");

	const auto index_count = door_count_ * indices_per_door_side;

	door_sides_ib_ = create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_streaming,
		2,
		index_count
	);
}

void destroy_door_sides_ibi()
{
	door_sides_ibi_.clear();
}

void destroy_door_sides_ib()
{
	destroy_index_buffer(door_sides_ib_);

	door_sides_ibi_.clear();
}

void create_door_sides_vb()
{
	log("Creating door vertex buffer.");

	const auto vertex_count = door_count_ * indices_per_door_side;

	door_sides_vb_ = create_vertex_buffer<DoorVertex>(
		bstone::Ren3dBufferUsageKind::draw_dynamic,
		vertex_count
	);
}

void destroy_door_sides_vb()
{
	destroy_vertex_buffer(door_sides_vb_);
}

void destroy_door_sides_vi()
{
	destroy_vertex_input(door_sides_vi_);
}

void create_door_sides_vi()
{
	log("Creating door vertex input.");

	create_vertex_input<DoorVertex>(
		door_sides_ib_,
		door_sides_vb_,
		door_sides_vi_);
}

void initialize_door_sides()
{
	log("");
	log("Initializing doors.");

	xy_door_map_.reserve(door_count_);

	const auto max_draw_item_count = door_count_ * door_halves_per_door;

	door_draw_item_count_ = 0;
	door_draw_items_.clear();
	door_draw_items_.resize(max_draw_item_count);

	create_door_sides_ibi();
	create_door_sides_ib();
	create_door_sides_vb();
	create_door_sides_vi();
}

void uninitialize_door_sides()
{
	xy_door_map_.clear();

	door_draw_item_count_ = 0;
	door_draw_items_.clear();

	destroy_door_sides_vi();

	destroy_door_sides_ibi();
	destroy_door_sides_ib();

	destroy_door_sides_vb();
}

void update_palette_from_vga(
	int offset,
	int count)
{
	for (auto i = 0; i < count; ++i)
	{
		const auto& vga_color = vga_palette_[offset + i];
		auto& hw_color = palette_[offset + i];

		hw_color = vga_color_to_rgba_8(
			vga_color[0],
			vga_color[1],
			vga_color[2]
		);
	}
}

void initialize_palette()
{
	vga_palette_ = {};
	palette_ = {};
	default_palette_ = {};

	for (int i = 0; i < bstone::RgbPalette::get_max_color_count(); ++i)
	{
		const auto vga_color = vgapal + (i * 3);
		auto& color = default_palette_[i];

		color = vga_color_to_rgba_8(vga_color[0], vga_color[1], vga_color[2]);
	}
}

void calculate_dimensions()
{
	auto src_param = vid_create_screen_size_param();

	vid_calculate_window_elements_dimensions(src_param, vid_layout_);

	vid_calculate_vga_dimensions();
}

void build_2d_model_matrix()
{
	r2_matrix_model_ = glm::identity<glm::mat4>();
}

void build_2d_view_matrix()
{
	r2_matrix_view_ = glm::identity<glm::mat4>();
}

void build_2d_projection_matrix()
{
	r2_matrix_projection_ = glm::orthoRH_NO(
		0.0, // left
		static_cast<double>(vid_layout_.screen_width), // right
		0.0, // bottom
		static_cast<double>(vid_layout_.screen_height), // top
		0.0, // zNear
		1.0 // zFar
	);
}

void build_2d_matrices()
{
	build_2d_model_matrix();
	build_2d_view_matrix();
	build_2d_projection_matrix();
}

void calculate_camera_parameters()
{
	// Vertical FOV.
	//
	// V = 2 * arctan(tan(H / 2) * (h / w))
	//

	const auto ref_r_ratio = static_cast<double>(vga_ref_height_4x3) / static_cast<double>(vga_ref_width);

	const auto half_hfov_deg = ref_camera_hfov_deg / 2.0;
	const auto half_hfov_rad = bstone::math::deg_to_rad(half_hfov_deg);
	const auto tan_half_hfov_rad = std::tan(half_hfov_rad);
	const auto half_vfov_rad = tan_half_hfov_rad * ref_r_ratio;

	// Radians.
	const auto vfov_rad = 2.0 * half_vfov_rad;
	camera_vfov_rad_ = vfov_rad;

	// Degrees.
	const auto half_vfov_deg = bstone::math::rad_to_deg(half_vfov_rad);
	const auto vfov_deg = 2.0 * half_vfov_deg;
	camera_vfov_deg_ = vfov_deg;


	// Distances to the planes.
	//
	camera_near_distance_ = 0.05;
	camera_far_distance_ = (std::sqrt(2.0) * map_dimension_d) + 0.5;
}

void build_bs_to_ren_matrix()
{
	//
	// |  0 y   0   0 |
	// |  0 0 z*1.2 0 |
	// | -x 0   0   0 |
	// |  0 0   0   1 |
	//

	const auto m_11 = 0.0F;
	const auto m_12 = 1.0F;
	const auto m_13 = 0.0F;
	const auto m_14 = 0.0F;

	const auto m_21 = 0.0F;
	const auto m_22 = 0.0F;
	const auto m_23 = static_cast<float>(height_compensation_factor);
	const auto m_24 = 0.0F;

	const auto m_31 = -1.0F;
	const auto m_32 = 0.0F;
	const auto m_33 = 0.0F;
	const auto m_34 = 0.0F;

	const auto m_41 = 0.0F;
	const auto m_42 = 0.0F;
	const auto m_43 = 0.0F;
	const auto m_44 = 1.0F;

	r3_matrix_bs_to_r_ = glm::mat4
	{
		m_11, m_21, m_31, m_41,
		m_12, m_22, m_32, m_42,
		m_13, m_23, m_33, m_43,
		m_14, m_24, m_34, m_44,
	};
}

void build_model_matrix()
{
	r3_matrix_model_ = glm::identity<glm::mat4>();
}

void build_view_matrix()
{
	if (!player)
	{
		r3_matrix_view_ = glm::identity<glm::mat4>();

		return;
	}

	auto view_matrix = glm::identity<glm::dmat4>();

	view_matrix = glm::rotate(view_matrix, player_angle_rad_, glm::dvec3{0.0, 0.0, 1.0});
	view_matrix = glm::translate(view_matrix, -view_position_);

	r3_matrix_view_ = view_matrix;
}

void build_projection_matrix()
{
	const auto perspective = glm::perspectiveFovRH_NO(
		static_cast<float>(camera_vfov_rad_),
		static_cast<float>(vid_layout_.screen_viewport_width),
		static_cast<float>(vid_layout_.screen_viewport_height),
		static_cast<float>(camera_near_distance_),
		static_cast<float>(camera_far_distance_)
	);

	r3_matrix_projection_ = perspective * r3_matrix_bs_to_r_;
}

void build_3d_matrices()
{
	calculate_camera_parameters();

	build_bs_to_ren_matrix();
	build_model_matrix();
	build_view_matrix();
	build_projection_matrix();
}

void build_matrices()
{
	build_2d_matrices();
	build_3d_matrices();
}

void set_ui_sampler_default_state()
{
	ui_sampler_state_.min_filter_ = bstone::Ren3dFilterKind::nearest;
	ui_sampler_state_.mag_filter_ = bstone::Ren3dFilterKind::nearest;
	ui_sampler_state_.mipmap_mode_ = bstone::Ren3dMipmapMode::none;
	ui_sampler_state_.address_mode_u_ = bstone::Ren3dAddressMode::clamp;
	ui_sampler_state_.address_mode_v_ = bstone::Ren3dAddressMode::clamp;
	ui_sampler_state_.anisotropy_ = bstone::Ren3dLimits::min_anisotropy_off;
}

void update_ui_sampler_state()
{
	ui_sampler_state_.min_filter_ = vid_cfg_get().d2_texture_filter_;
	ui_sampler_state_.mag_filter_ = vid_cfg_get().d2_texture_filter_;
}

void update_ui_sampler()
{
	update_ui_sampler_state();

	if (ui_sampler_ != nullptr)
	{
		auto param = bstone::Ren3dSamplerUpdateParam{};
		param.state_ = ui_sampler_state_;
		ui_sampler_->update(param);
	}
}

void destroy_ui_sampler()
{
	ui_sampler_ = nullptr;
}

void create_ui_sampler()
{
	log("Creating UI sampler.");

	update_ui_sampler_state();

	auto param = bstone::Ren3dCreateSamplerParam{};
	param.state_ = ui_sampler_state_;

	ui_sampler_ = renderer_->create_sampler(param);
}

void set_sprite_sampler_default_state()
{
	sprite_sampler_state_.min_filter_ = bstone::Ren3dFilterKind::nearest;
	sprite_sampler_state_.mag_filter_ = bstone::Ren3dFilterKind::nearest;
	sprite_sampler_state_.mipmap_mode_ = bstone::Ren3dMipmapMode::nearest;
	sprite_sampler_state_.address_mode_u_ = bstone::Ren3dAddressMode::clamp;
	sprite_sampler_state_.address_mode_v_ = bstone::Ren3dAddressMode::clamp;
	sprite_sampler_state_.anisotropy_ = bstone::Ren3dLimits::min_anisotropy_off;
}

void update_sprite_sampler_state()
{
	sprite_sampler_state_.min_filter_ = vid_cfg_get().d3_texture_image_filter_;
	sprite_sampler_state_.mag_filter_ = vid_cfg_get().d3_texture_image_filter_;

	sprite_sampler_state_.mipmap_mode_ = cfg_texture_mipmap_filter_to_renderer(
		vid_cfg_get().d3_texture_mipmap_filter_);

	sprite_sampler_state_.anisotropy_ = cfg_texture_anisotropy_to_renderer(
		vid_cfg_get().d3_texture_anisotropy_
	);
}

void update_sprite_sampler()
{
	update_sprite_sampler_state();

	if (sprite_sampler_)
	{
		auto param = bstone::Ren3dSamplerUpdateParam{};
		param.state_ = sprite_sampler_state_;
		sprite_sampler_->update(param);
	}
}

void destroy_sprite_sampler()
{
	destroy_sampler(sprite_sampler_);
}

void create_sprite_sampler()
{
	log("Creating sprite sampler.");

	update_sprite_sampler_state();

	auto param = bstone::Ren3dCreateSamplerParam{};
	param.state_ = sprite_sampler_state_;

	sprite_sampler_ = renderer_->create_sampler(param);
}

void set_wall_sampler_default_state()
{
	wall_sampler_state_.min_filter_ = bstone::Ren3dFilterKind::nearest;
	wall_sampler_state_.mag_filter_ = bstone::Ren3dFilterKind::nearest;
	wall_sampler_state_.mipmap_mode_ = bstone::Ren3dMipmapMode::nearest;
	wall_sampler_state_.address_mode_u_ = bstone::Ren3dAddressMode::repeat;
	wall_sampler_state_.address_mode_v_ = bstone::Ren3dAddressMode::repeat;
	wall_sampler_state_.anisotropy_ = bstone::Ren3dLimits::min_anisotropy_off;
}

void update_wall_sampler_state()
{
	wall_sampler_state_.min_filter_ = vid_cfg_get().d3_texture_image_filter_;
	wall_sampler_state_.mag_filter_ = vid_cfg_get().d3_texture_image_filter_;

	wall_sampler_state_.mipmap_mode_ = cfg_texture_mipmap_filter_to_renderer(
		vid_cfg_get().d3_texture_mipmap_filter_);

	wall_sampler_state_.anisotropy_ = cfg_texture_anisotropy_to_renderer(
		vid_cfg_get().d3_texture_anisotropy_
	);
}

void update_wall_sampler()
{
	update_wall_sampler_state();

	if (wall_sampler_ != nullptr)
	{
		auto param = bstone::Ren3dSamplerUpdateParam{};
		param.state_ = wall_sampler_state_;
		wall_sampler_->update(param);
	}
}

void destroy_wall_sampler()
{
	destroy_sampler(wall_sampler_);
}

void create_wall_sampler()
{
	log("Creating wall side sampler.");

	update_wall_sampler_state();

	auto param = bstone::Ren3dCreateSamplerParam{};
	param.state_ = wall_sampler_state_;

	wall_sampler_ = renderer_->create_sampler(param);
}

void update_player_weapon_vb()
{
	auto vertices = PlayerWeaponVbi{};
	vertices.resize(vertices_per_sprite);

	const auto dimension = static_cast<float>(bstone::Sprite::dimension);
	const auto half_dimension = 0.5F * dimension;

	auto vertex_index = 0;

	// Bottom-left.
	//
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{-half_dimension, 0.0F, 0.0F};
		vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom-right.
	//
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{half_dimension, 0.0F, 0.0F};
		vertex.uv = VertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top-right.
	//
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{half_dimension, dimension, 0.0F};
		vertex.uv = VertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top-left.
	//
	{
		auto& vertex = vertices[vertex_index];
		vertex.xyz = VertexPosition{-half_dimension, dimension, 0.0F};
		vertex.uv = VertexTextureCoordinates{0.0F, 1.0F};
	}

	// Update vertex buffer.
	//
	update_vertex_buffer(
		player_weapon_vb_,
		0,
		vertices_per_sprite,
		vertices.data()
	);
}

void destroy_player_weapon_ib()
{
	destroy_index_buffer(player_weapon_ib_);
}

void create_player_weapon_ib()
{
	log("Creating player's weapon index buffer.");

	player_weapon_ib_ = create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_static,
		1,
		indices_per_sprite
	);
}

void update_player_weapon_ib()
{
	using Indices = std::array<std::uint8_t, indices_per_sprite>;

	auto indices = Indices
	{
		0, 1, 2,
		0, 2, 3,
	}; // indices

	update_index_buffer(
		player_weapon_ib_,
		0,
		indices_per_sprite,
		indices.data()
	);
}

void destroy_player_weapon_vb()
{
	destroy_vertex_buffer(player_weapon_vb_);
}

void create_player_weapon_vb()
{
	log("Creating player's weapon vertex buffer.");

	player_weapon_vb_ = create_vertex_buffer<PlayerWeaponVertex>(
		bstone::Ren3dBufferUsageKind::draw_static,
		vertices_per_sprite
	);
}

void destroy_player_weapon_vi()
{
	destroy_vertex_input(player_weapon_vi_);
}

void create_player_weapon_vi()
{
	log("Creating player's weapon vertex input.");

	create_vertex_input<PlayerWeaponVertex>(
		player_weapon_ib_,
		player_weapon_vb_,
		player_weapon_vi_);
}

void update_player_weapon_model_matrix()
{
	const auto& assets_info = get_assets_info();

	const auto aog_scale = 128.0 / 64.0;
	const auto ps_scale = 88.0 / 64.0;

	const auto game_scalar = (assets_info.is_ps() ? ps_scale : aog_scale);
	const auto scalar = game_scalar * vga_height_scale;

	const auto translate_x = 0.5 * static_cast<double>(vid_layout_.screen_viewport_width);

	const auto is_bobbing_enabled = (!g_no_weapon_bobbing && assets_info.is_ps());
	const auto bounce_offset = (is_bobbing_enabled ? -player_get_weapon_bounce_offset() : 0.0);
	const auto translate_y = vga_height_scale * bounce_offset;

	const auto translate_v = glm::vec3
	{
		static_cast<float>(translate_x),
		static_cast<float>(height_compensation_factor * translate_y),
		0.0F
	};

	const auto& identity = glm::identity<glm::mat4>();
	const auto& translate = glm::translate(identity, translate_v);

	const auto& scale = glm::scale(
		identity,
		glm::vec3{scalar, height_compensation_factor * scalar, 0.0F}
	);

	player_weapon_model_matrix_ = translate * scale;
}

void update_player_weapon_view_matrix()
{
	player_weapon_view_matrix_ = glm::identity<glm::mat4>();
}

void build_player_weapon_projection_matrix()
{
	const auto ortho = glm::orthoRH_NO(
		0.0F, // left
		static_cast<float>(vid_layout_.screen_viewport_width), // right
		0.0F, // bottom
		static_cast<float>(vid_layout_.screen_viewport_height), // top
		0.0F, // zNear
		1.0F // zFar
	);

	player_weapon_projection_matrix_ = ortho;
}

void set_player_weapon_sampler_default_state()
{
	player_weapon_sampler_state_.min_filter_ = bstone::Ren3dFilterKind::nearest;
	player_weapon_sampler_state_.mag_filter_ = bstone::Ren3dFilterKind::nearest;
	player_weapon_sampler_state_.mipmap_mode_ = bstone::Ren3dMipmapMode::none;
	player_weapon_sampler_state_.address_mode_u_ = bstone::Ren3dAddressMode::clamp;
	player_weapon_sampler_state_.address_mode_v_ = bstone::Ren3dAddressMode::clamp;
	player_weapon_sampler_state_.anisotropy_ = bstone::Ren3dLimits::min_anisotropy_off;
}

void update_player_weapon_sampler_state()
{
	player_weapon_sampler_state_.min_filter_ = vid_cfg_get().d3_texture_image_filter_;
	player_weapon_sampler_state_.mag_filter_ = vid_cfg_get().d3_texture_image_filter_;
}

void update_player_weapon_sampler()
{
	update_player_weapon_sampler_state();

	if (player_weapon_sampler_ != nullptr)
	{
		auto param = bstone::Ren3dSamplerUpdateParam{};
		param.state_ = player_weapon_sampler_state_;
		player_weapon_sampler_->update(param);
	}
}

void destroy_player_weapon_sampler()
{
	player_weapon_sampler_ = nullptr;
}

void create_player_weapon_sampler()
{
	log("Creating player's weapon sampler.");

	update_player_weapon_sampler_state();

	auto param = bstone::Ren3dCreateSamplerParam{};
	param.state_ = player_weapon_sampler_state_;

	player_weapon_sampler_ = renderer_->create_sampler(param);
}

void uninitialize_player_weapon()
{
	destroy_player_weapon_vi();
	destroy_player_weapon_ib();
	destroy_player_weapon_vb();
	destroy_player_weapon_sampler();
}

void initialize_player_weapon()
{
	log("");
	log("Initializing player's weapon.");

	create_player_weapon_ib();
	create_player_weapon_vb();
	create_player_weapon_vi();
	create_player_weapon_sampler();

	update_player_weapon_ib();
	update_player_weapon_vb();

	update_player_weapon_model_matrix();
	update_player_weapon_view_matrix();
	build_player_weapon_projection_matrix();
}

void destroy_fade_sampler()
{
	destroy_sampler(fade_sampler_);
}

void create_fade_sampler()
{
	log("Creating fade sampler.");

	auto param = bstone::Ren3dCreateSamplerParam{};
	param.state_.min_filter_ = bstone::Ren3dFilterKind::nearest;
	param.state_.mag_filter_ = bstone::Ren3dFilterKind::nearest;
	param.state_.mipmap_mode_ = bstone::Ren3dMipmapMode::none;
	param.state_.address_mode_u_ = bstone::Ren3dAddressMode::repeat;
	param.state_.address_mode_v_ = bstone::Ren3dAddressMode::repeat;
	param.state_.anisotropy_ = bstone::Ren3dLimits::min_anisotropy_off;

	fade_sampler_ = renderer_->create_sampler(param);
}

void set_samplers_default_states()
{
	set_ui_sampler_default_state();
	set_sprite_sampler_default_state();
	set_wall_sampler_default_state();
}

void uninitialize_samplers()
{
	destroy_ui_sampler();
	destroy_sprite_sampler();
	destroy_wall_sampler();
	destroy_fade_sampler();
}

void initialize_samplers()
{
	log("");
	log("Initializing samplers.");

	create_ui_sampler();
	create_sprite_sampler();
	create_wall_sampler();
	create_fade_sampler();
}

void destroy_command_buffers()
{
	command_buffers_.fill(nullptr);
}

void create_command_buffers()
{
	log("Initializing command buffers.");

	command_buffers_[0] = common_command_buffer_.get();
	command_buffers_[1] = r3_command_buffer_.get();
	command_buffers_[2] = r2_command_buffer_.get();
}

void destroy_common_command_buffer()
{
	common_command_buffer_ = nullptr;
}

void create_common_command_buffer()
{
	log("Creating common command buffer.");

	auto param = bstone::Ren3dCreateCmdBufferParam{};
	param.initial_size_ = common_command_buffer_initial_size;
	param.resize_delta_size_ = common_command_buffer_resize_delta_size;

	common_command_buffer_ = bstone::Ren3dCmdBufferFactory::create(param);
}

void destroy_2d_command_buffer()
{
	r2_command_buffer_ = nullptr;
}

void create_2d_command_buffer()
{
	log("Creating 2D command buffer.");

	auto param = bstone::Ren3dCreateCmdBufferParam{};
	param.initial_size_ = r2_command_buffer_initial_size;
	param.resize_delta_size_ = r2_command_buffer_resize_delta_size;

	r2_command_buffer_ = bstone::Ren3dCmdBufferFactory::create(param);
}

void destroy_3d_command_buffer()
{
	r3_command_buffer_ = nullptr;
}

void create_3d_command_buffer()
{
	log("Creating 3D command buffer.");

	auto param = bstone::Ren3dCreateCmdBufferParam{};
	param.initial_size_ = r3_command_buffer_initial_size;
	param.resize_delta_size_ = r3_command_buffer_resize_delta_size;

	r3_command_buffer_ = bstone::Ren3dCmdBufferFactory::create(param);
}

void uninitialize_command_buffers()
{
	destroy_3d_command_buffer();
	destroy_2d_command_buffer();
	destroy_common_command_buffer();
	destroy_command_buffers();
}

void initialize_command_buffers()
{
	log("");
	log("Initializing command buffers.");

	create_common_command_buffer();
	create_3d_command_buffer();
	create_2d_command_buffer();
	create_command_buffers();
}

void destroy_3d_fade_ib()
{
	destroy_index_buffer(r3_fade_ib_);
}

void create_3d_fade_ib()
{
	log("Creating 3D fade index buffer.");

	r3_fade_ib_ = create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_static,
		1,
		6
	);
}

void destroy_3d_fade_vb()
{
	destroy_vertex_buffer(r3_fade_vb_);
}

void create_3d_fade_vb()
{
	log("Creating 3D fade vertex buffer.");

	r3_fade_vb_ = create_vertex_buffer<FadeVertex>(
		bstone::Ren3dBufferUsageKind::draw_static,
		4
	);
}

void destroy_3d_fade_vi()
{
	destroy_vertex_input(r3_fade_vi_);
}

void create_3d_fade_vi()
{
	log("Creating 3D fade vertex input.");

	create_vertex_input<FadeVertex>(
		r3_fade_ib_,
		r3_fade_vb_,
		r3_fade_vi_);
}

void update_3d_fade_ib()
{
	using Indices = std::array<std::uint8_t, 6>;

	const auto& indices = Indices
	{
		0, 1, 2,
		0, 2, 3,
	}; // indices

	update_index_buffer(
		r3_fade_ib_,
		0,
		6,
		indices.data()
	);
}

void update_3d_fade_vb()
{
	auto vertices = FadeVbi{};
	vertices.resize(4);

	auto vertex_index = 0;

	const auto width_f = vid_layout_.screen_viewport_width;
	const auto height_f = vid_layout_.screen_viewport_height;

	// Bottom left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{0.0F, 0.0F, 0.0F};
		vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{width_f, 0.0F, 0.0F};
		vertex.uv = VertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top right.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{width_f, height_f, 0.0F};
		vertex.uv = VertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top left.
	{
		auto& vertex = vertices[vertex_index++];
		vertex.xyz = VertexPosition{0.0F, height_f, 0.0F};
		vertex.uv = VertexTextureCoordinates{0.0F, 1.0F};
	}

	update_vertex_buffer<FadeVertex>(
		r3_fade_vb_,
		0,
		4,
		vertices.data()
	);
}

void destroy_3d_fade_texture_2d()
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->destroy_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_3d);
	}

	r3_fade_t2d_ = nullptr;
}

void create_3d_fade_texture_2d()
{
	log("Create 3D fade texture.");

	texture_mgr_->create_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_3d);
	r3_fade_t2d_ = texture_mgr_->get_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_3d);
}

void uninitialize_3d_fade()
{
	destroy_3d_fade_vi();
	destroy_3d_fade_vb();
	destroy_3d_fade_ib();
	destroy_3d_fade_texture_2d();
}

void destroy_texture_manager()
{
	texture_mgr_ = nullptr;
}

void create_texture_manager()
{
	log("");
	log("Creating texture manager.");

	texture_mgr_ = bstone::HwTextureMgrFactory::create(
		renderer_,
		&vid_sprite_cache,
		mt_task_manager_
	);
}

void initialize_3d_fade()
{
	log("");
	log("Initializing 3D fade.");

	create_3d_fade_ib();
	create_3d_fade_vb();
	create_3d_fade_vi();
	create_3d_fade_texture_2d();

	update_3d_fade_ib();
	update_3d_fade_vb();
}

void present_common()
{
	sampler_var_ = 0;
	shading_mode_ = 0;
	bs_shade_max_ = shade_max;
	bs_normal_shade_ = normalshade;
	bs_height_numerator_ = heightnumerator;
	bs_lighting_ = 0;


	auto& command_buffer = common_command_buffer_;

	command_buffer->enable(true);

	command_buffer->begin_write();

	// Build commands.
	//

	// Clear buffers.
	//
	{
		static const auto clear_color = bstone::Rgba8{};
		auto& clear = *command_buffer->write_clear();
		clear.clear_.color_ = clear_color;
	}

	// Set shader's 2D sampler.
	//
	if (sampler_var_.is_modified())
	{
		sampler_var_.set_is_modified(false);

		auto command = command_buffer->write_set_sampler_2d_uniform();
		command->var_ = sampler_uniform_;
		command->value_ = sampler_var_;
	}

	// Set shader stage.
	//
	{
		auto& shader_stage = command_buffer->write_set_shader_stage()->shader_stage_;
		shader_stage = shader_stage_.get();
	}

	// Set shading mode.
	//
	if (shading_mode_.is_modified())
	{
		shading_mode_.set_is_modified(false);

		auto& command = *command_buffer->write_set_int32_uniform();
		command.var_ = shading_mode_uniform_;
		command.value_ = shading_mode_;
	}

	// Set shade_max.
	//
	if (bs_shade_max_.is_modified())
	{
		bs_shade_max_.set_is_modified(false);

		auto& command = *command_buffer->write_set_float32_uniform();
		command.var_ = shade_max_uniform_;
		command.value_ = static_cast<float>(bs_shade_max_);
	}

	// Set normal_shade.
	//
	if (bs_normal_shade_.is_modified())
	{
		bs_normal_shade_.set_is_modified(false);

		auto& command = *command_buffer->write_set_float32_uniform();
		command.var_ = normal_shade_uniform_;
		command.value_ = static_cast<float>(bs_normal_shade_);
	}

	// Set height_numerator.
	//
	if (bs_height_numerator_.is_modified())
	{
		bs_height_numerator_.set_is_modified(false);

		auto& command = *command_buffer->write_set_float32_uniform();
		command.var_ = height_numerator_uniform_;
		command.value_ = static_cast<float>(bs_height_numerator_);
	}

	// Set extra_lighting.
	//
	if (bs_lighting_.is_modified())
	{
		bs_lighting_.set_is_modified(false);

		auto& command = *command_buffer->write_set_float32_uniform();
		command.var_ = extra_lighting_uniform_;
		command.value_ = static_cast<float>(bs_lighting_);
	}

	// Set view_direction.
	//
	if (is_draw_3d_ && bs_view_direction_.is_modified())
	{
		bs_view_direction_.set_is_modified(false);

		auto& command = *command_buffer->write_set_vec2_uniform();
		command.var_ = view_direction_uniform_;
		command.value_ = cast_glm_vec2(bs_view_direction_);
	}

	// Set view_position.
	//
	if (is_draw_3d_ && bs_view_position_.is_modified())
	{
		bs_view_position_.set_is_modified(false);

		auto& command = *command_buffer->write_set_vec2_uniform();
		command.var_ = view_position_uniform_;
		command.value_ = cast_glm_vec2(bs_view_position_);
	}

	// Finalize.
	//
	command_buffer->end_write();
}

void present_2d()
{
	// Update 2D texture.
	//
	{
		texture_mgr_->update_ui();
	}

	// Update fade color.
	//
	if (r2_fade_is_enabled_)
	{
		texture_mgr_->update_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_2d, r2_fade_color_);
	}


	auto& command_buffer = r2_command_buffer_;

	command_buffer->enable(true);

	command_buffer->begin_write();

	// Build commands.
	//

	// Disable back-face culling.
	//
	{
		auto& command = *command_buffer->write_enable_culling();
		command.is_enable_ = false;
	}

	// Disable depth test.
	//
	{
		auto& command = *command_buffer->write_enable_depth_test();
		command.is_enable_ = false;
	}

	// Set viewport.
	//
	{
		auto& viewport = command_buffer->write_set_viewport()->viewport_;
		viewport.x = vid_layout_.window_viewport_left_width;
		viewport.y = vid_layout_.window_viewport_bottom_height;
		viewport.width_ = vid_layout_.screen_width;
		viewport.height_ = vid_layout_.screen_height;
		viewport.min_depth_ = 0.0F;
		viewport.max_depth_ = 0.0F;
	}

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_set_sampler();
		command.sampler_ = ui_sampler_.get();
	}

	// Set model matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = model_mat_uniform_;
		command.value_ = cast_glm_mat4(r2_matrix_model_);
	}

	// Set view matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = view_mat_uniform_;
		command.value_ = cast_glm_mat4(r2_matrix_view_);
	}

	// Set projection matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = projection_mat_uniform_;
		command.value_ = cast_glm_mat4(r2_matrix_projection_);
	}

	// Fillers.
	//
	if (!vid_cfg_get().is_ui_stretched_)
	{
		{
			auto& command = *command_buffer->write_set_texture();
			command.texture_2d_ = (vid_is_movie ? r2_black_t2d_1x1_ : r2_white_t2d_1x1_);
		}

		{
			auto& command = *command_buffer->write_set_vertex_input();
			command.vertex_input_ = r2_fillers_vi_.get();
		}

		{
			auto count = 0;
			auto index_offset = 0;

			if (vid_is_hud)
			{
				count = r2_fillers_hud_quad_count;
				index_offset = r2_fillers_hud_index_offset_;
			}
			else
			{
				count = r2_fillers_ui_quad_count;
				index_offset = r2_fillers_ui_index_offset_;
			}

			auto& command = *command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed_;

			param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
			param.vertex_count_ = count * vertices_per_quad;
			param.index_byte_depth_ = 1;
			param.index_buffer_offset_ = 0;
			param.index_offset_ = index_offset;
		}
	}

	// Draw 2D (UI, menu, etc.).
	//
	{
		if (vid_is_hud)
		{
			{
				auto& command = *command_buffer->write_enable_blending();
				command.is_enable_ = true;
			}

			// Set blending function.
			//
			{
				auto& blending_func = command_buffer->write_set_blending_func()->blending_func_;
				blending_func.src_factor_ = bstone::Ren3dBlendingFactor::src_alpha;
				blending_func.dst_factor_ = bstone::Ren3dBlendingFactor::one_minus_src_alpha;
			}
		}

		{
			auto& command = *command_buffer->write_set_texture();
			command.texture_2d_ = ui_t2d_;
		}

		{
			auto& command = *command_buffer->write_set_vertex_input();
			command.vertex_input_ = ui_vi_.get();
		}

		{
			const auto index_offset = (vid_cfg_get().is_ui_stretched_
				?
				r2_stretched_index_offset_
				:
				r2_non_stretched_index_offset_
			);

			auto& command = *command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed_;

			param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
			param.vertex_count_ = vertices_per_quad;
			param.index_byte_depth_ = 1;
			param.index_buffer_offset_ = 0;
			param.index_offset_ = index_offset;
		}

		if (vid_is_hud)
		{
			auto& command = *command_buffer->write_enable_blending();
			command.is_enable_ = false;
		}
	}

	// 2D fade in or out.
	//
	if (r2_fade_is_enabled_)
	{
		// Enable blending.
		//
		{
			auto& command = *command_buffer->write_enable_blending();
			command.is_enable_ = true;
		}

		// Set blending function.
		//
		{
			auto& blending_func = command_buffer->write_set_blending_func()->blending_func_;
			blending_func.src_factor_ = bstone::Ren3dBlendingFactor::src_alpha;
			blending_func.dst_factor_ = bstone::Ren3dBlendingFactor::one_minus_src_alpha;
		}

		// Set texture.
		//
		{
			auto& command = *command_buffer->write_set_texture();
			command.texture_2d_ = r2_fade_t2d_;
		}

		// Set sampler.
		//
		{
			auto& command = *command_buffer->write_set_sampler();
			command.sampler_ = fade_sampler_.get();
		}

		// Set vertex input.
		//
		{
			auto& command = *command_buffer->write_set_vertex_input();
			command.vertex_input_ = ui_vi_.get();
		}

		// Draw the quad.
		//
		{
			const auto index_offset = (vid_cfg_get().is_ui_stretched_ || is_draw_3d_
				?
				r2_stretched_index_offset_
				:
				r2_non_stretched_index_offset_
			);

			auto& command = *command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed_;

			param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
			param.vertex_count_ = vertices_per_quad;
			param.index_byte_depth_ = 1;
			param.index_buffer_offset_ = 0;
			param.index_offset_ = index_offset;
		}

		// Disable blending.
		//
		{
			auto& command = *command_buffer->write_enable_blending();
			command.is_enable_ = false;
		}
	}

	// Finalize.
	//
	command_buffer->end_write();
}

void render_walls()
{
	if (wall_count_ <= 0)
	{
		return;
	}

	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = wall_side_draw_items_;

	if (walls_to_render_.empty())
	{
		return;
	}

	const auto wall_map_end_it = xy_wall_map_.cend();

	for (const auto wall_xy : walls_to_render_)
	{
		const auto wall_map_it = xy_wall_map_.find(wall_xy);

		if (wall_map_it == wall_map_end_it)
		{
			continue;
		}

		for (const auto& side : wall_map_it->second.sides)
		{
			if (!side.flags.is_active)
			{
				continue;
			}

			auto& draw_item = draw_items[draw_side_index++];

			draw_item.texture_id = side.texture_id;
			draw_item.wall_side_ = &side;
		}
	}

	// Sort by texture.
	//
	std::sort(
		draw_items.begin(),
		draw_items.begin() + draw_side_index,
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.texture_id < rhs.texture_id;
		}
	);

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = wall_sides_ibi_;

		for (int i = 0; i < draw_side_index; ++i)
		{
			const auto& wall_side = *draw_items[i].wall_side_;

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 1);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 2);

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 2);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 3);
		}

		update_index_buffer(
			wall_sides_ib_,
			0,
			ib_index,
			ib_buffer.data()
		);
	}

	// Add render commands.
	//
	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset_ = 0;
	auto& command_buffer = r3_command_buffer_;

	while (draw_index < draw_side_index)
	{
		auto is_first = true;
		auto last_texture_id = 0;

		draw_quad_count = 0;

		while (draw_index < draw_side_index)
		{
			if (is_first)
			{
				is_first = false;

				last_texture_id = draw_items[draw_index].texture_id;
			}
			else if (last_texture_id == draw_items[draw_index].texture_id)
			{
				++draw_quad_count;
				++draw_index;
			}
			else
			{
				break;
			}
		}

		if (draw_quad_count > 0)
		{
			{
				const auto texture_2d = texture_mgr_->get_wall(last_texture_id);
				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = texture_2d;
			}

			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = wall_sides_vi_.get();
			}

			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = draw_quad_count * vertices_per_quad;
				param.index_byte_depth_ = 2;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = draw_index_offset_;

				draw_index_offset_ += indices_per_wall_side * draw_quad_count;
			}
		}
	}

	wall_side_draw_item_count_ = draw_side_index;
}

void render_pushwalls()
{
	if (pushwall_count_ <= 0)
	{
		return;
	}

	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = pushwall_side_draw_items_;

	if (pushwalls_to_render_.empty())
	{
		return;
	}

	const auto pushwall_map_end_it = xy_pushwall_map_.cend();

	for (const auto pushwall_xy : pushwalls_to_render_)
	{
		const auto pushwall_map_it = xy_pushwall_map_.find(pushwall_xy);

		if (pushwall_map_it == pushwall_map_end_it)
		{
			continue;
		}

		for (const auto& side : pushwall_map_it->second.sides)
		{
			if (!side.flags.is_active)
			{
				continue;
			}

			auto& draw_item = draw_items[draw_side_index++];

			draw_item.texture_id = side.texture_id;
			draw_item.wall_side_ = &side;
		}
	}

	// Sort by texture.
	//
	std::sort(
		draw_items.begin(),
		draw_items.begin() + draw_side_index,
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.texture_id < rhs.texture_id;
		}
	);

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = pushwall_sides_ibi_;

		for (int i = 0; i < draw_side_index; ++i)
		{
			const auto& wall_side = *draw_items[i].wall_side_;

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 1);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 2);

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 2);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 3);
		}

		update_index_buffer(
			pushwall_sides_ib_,
			0,
			ib_index,
			ib_buffer.data()
		);
	}

	// Add render commands.
	//
	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset_ = 0;
	auto& command_buffer = r3_command_buffer_;

	while (draw_index < draw_side_index)
	{
		auto is_first = true;
		auto last_texture_id = 0;

		draw_quad_count = 0;

		while (draw_index < draw_side_index)
		{
			if (is_first)
			{
				is_first = false;

				last_texture_id = draw_items[draw_index].texture_id;
			}
			else if (last_texture_id == draw_items[draw_index].texture_id)
			{
				++draw_quad_count;
				++draw_index;
			}
			else
			{
				break;
			}
		}

		if (draw_quad_count > 0)
		{
			{
				const auto texture_2d = texture_mgr_->get_wall(last_texture_id);
				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = texture_2d;
			}

			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = pushwall_sides_vi_.get();
			}

			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = draw_quad_count * vertices_per_quad;
				param.index_byte_depth_ = 2;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = draw_index_offset_;

				draw_index_offset_ += indices_per_wall_side * draw_quad_count;
			}
		}
	}

	pushwall_side_draw_item_count_ = draw_side_index;
}

void render_doors()
{
	if (door_count_ == 0)
	{
		return;
	}

	// Build draw list.
	//
	auto draw_side_index = 0;
	auto& draw_items = door_draw_items_;

	if (doors_to_render_.empty())
	{
		return;
	}

	const auto door_map_end_it = xy_door_map_.cend();

	for (const auto door_xy : doors_to_render_)
	{
		const auto door_map_it = xy_door_map_.find(door_xy);

		if (door_map_it == door_map_end_it)
		{
			continue;
		}

		for (const auto& side : door_map_it->second.sides)
		{
			auto& draw_item = draw_items[draw_side_index++];

			draw_item.texture_id = side.texture_id;
			draw_item.door_side = &side;
		}
	}

	// Sort by texture.
	//
	std::sort(
		draw_items.begin(),
		draw_items.begin() + draw_side_index,
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.texture_id < rhs.texture_id;
		}
	);

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = door_sides_ibi_;

		for (int i = 0; i < draw_side_index; ++i)
		{
			const auto& door_side = *draw_items[i].door_side;
			const auto& door = *door_side.door;

			if (door_side.is_back_face)
			{
				auto vertex_index = door.vertex_index + 4;

				for (int i_quad = 0; i_quad < 2; ++i_quad)
				{
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 1);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 0);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 3);

					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 1);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 3);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 2);

					vertex_index -= 4;
				}
			}
			else
			{
				auto vertex_index = door.vertex_index;

				for (int i_quad = 0; i_quad < 2; ++i_quad)
				{
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 0);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 1);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 2);

					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 0);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 2);
					ib_buffer[ib_index++] = static_cast<std::uint16_t>(vertex_index + 3);

					vertex_index += 4;
				}
			}
		}

		update_index_buffer(
			door_sides_ib_,
			0,
			ib_index,
			ib_buffer.data()
		);
	}

	// Add render commands.
	//
	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset = 0;
	auto& command_buffer = r3_command_buffer_;

	while (draw_index < draw_side_index)
	{
		auto is_first = true;
		auto last_texture_id = 0;

		draw_quad_count = 0;

		while (draw_index < draw_side_index)
		{
			if (is_first)
			{
				is_first = false;

				last_texture_id = draw_items[draw_index].texture_id;
			}
			else if (last_texture_id == draw_items[draw_index].texture_id)
			{
				draw_quad_count += 2;
				++draw_index;
			}
			else
			{
				break;
			}
		}

		if (draw_quad_count > 0)
		{
			{
				const auto texture_2d = texture_mgr_->get_wall(last_texture_id);
				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = texture_2d;
			}

			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = door_sides_vi_.get();
			}

			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = draw_quad_count * vertices_per_quad;
				param.index_byte_depth_ = 2;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = draw_index_offset;

				draw_index_offset += 6 * draw_quad_count;
			}
		}
	}

	door_draw_item_count_ = draw_side_index;
}

void update_cloaked_actor(
	const Sprite& sprite)
{
	if (!sprite.flags.is_visible)
	{
		return;
	}

	if (sprite.kind != SpriteKind::actor)
	{
		return;
	}

	const auto& actor = *sprite.bs_object.actor;

	const auto is_cloaked = ((actor.flags2 & (FL2_CLOAKED | FL2_DAMAGE_CLOAK)) == FL2_CLOAKED);

	const auto vertex_color = (
		is_cloaked
		?
		VertexColor{0x00, 0x00, 0x00, cloaked_actor_alpha_u8}
		:
		VertexColor{0xFF, 0xFF, 0xFF, 0xFF}
	);


	auto vertex_index = sprite.vertex_index;

	// Bottom-left.
	{
		auto& vertex = sprites_vbi_[vertex_index++];
		vertex.rgba8 = vertex_color;
	}

	// Bottom-right.
	{
		auto& vertex = sprites_vbi_[vertex_index++];
		vertex.rgba8 = vertex_color;
	}

	// Top-right.
	{
		auto& vertex = sprites_vbi_[vertex_index++];
		vertex.rgba8 = vertex_color;
	}

	// Top-left.
	{
		auto& vertex = sprites_vbi_[vertex_index++];
		vertex.rgba8 = vertex_color;
	}
}

int calculate_actor_anim_rotation(
	const objtype& bs_actor)
{
	auto dir = bs_actor.dir;

	const auto view_dir_x = bs_actor.x - player->x;
	const auto view_dir_y = -bs_actor.y + player->y;

	const auto view_angle_rad = std::atan2(view_dir_y, view_dir_x);
	const auto view_angle = bstone::math::rad_to_deg(view_angle_rad);

	if (dir == nodir)
	{
		dir = static_cast<dirtype>(bs_actor.trydir & 127);
	}

	auto target_angle = (view_angle - 180) - dirangle[dir];

	target_angle += ANGLES / 16;

	while (target_angle >= ANGLES)
	{
		target_angle -= ANGLES;
	}

	while (target_angle < 0)
	{
		target_angle += ANGLES;
	}

	if ((bs_actor.state->flags & SF_PAINFRAME) != 0)
	{
		// 2 rotation pain frame
		return static_cast<int>(4 * (target_angle / (ANGLES / 2))); // seperated by 3 (art layout...)

	}

	return static_cast<int>(target_angle / (ANGLES / 8));
}

int get_bs_actor_sprite_id(
	const objtype& bs_actor)
{
	assert(bs_actor.state);

	auto result = bs_actor.state->shapenum;

	if ((bs_actor.flags & FL_OFFSET_STATES) != 0)
	{
		result += bs_actor.temp1;
	}

	if (result == -1)
	{
		result = bs_actor.temp1;
	}

	if ((bs_actor.state->flags & SF_ROTATE) != 0)
	{
		result += calculate_actor_anim_rotation(bs_actor);
	}

	return result;
}

void update_actor(
	std::intptr_t bs_actor_index)
{
	auto& actor = actors_[bs_actor_index];
	const auto& bs_actor = objlist[bs_actor_index];

	if (actor.kind == SpriteKind::none)
	{
		map_actor(bs_actor);
	}

	const auto new_bs_sprite_id = get_bs_actor_sprite_id(bs_actor);

	if (actor.bs_sprite_id == 0 || actor.bs_sprite_id != new_bs_sprite_id)
	{
		actor.bs_sprite_id = new_bs_sprite_id;
	}

	if (actor.x != bs_actor.x || actor.y != bs_actor.y)
	{
		actor.x = bs_actor.x;
		actor.y = bs_actor.y;
	}
}

void orient_sprite(
	Sprite& sprite)
{
	sprite.flags.is_visible = false;

	if (sprite.bs_sprite_id <= 0)
	{
		return;
	}

	if (sprite.kind == SpriteKind::actor)
	{
		if (sprite.bs_sprite_id <= 0)
		{
			return;
		}

		if (sprite.bs_object.actor->obclass == nothing)
		{
			return;
		}
	}

	auto sprite_origin = glm::dvec2{};

	if (sprite.kind == SpriteKind::actor)
	{
		sprite_origin[0] = sprite.x;
		sprite_origin[1] = sprite.y;
	}
	else
	{
		sprite_origin[0] = static_cast<double>(sprite.tile_x) + 0.5;
		sprite_origin[1] = static_cast<double>(sprite.tile_y) + 0.5;
	};

	auto direction = player_position_ - sprite_origin;

	sprite.flags.is_visible = true;


	auto bottom_left_vertex = sprite_origin;
	auto bottom_right_vertex = sprite_origin;

	const auto square_distance = glm::dot(direction, direction);

	sprite.square_distance = square_distance;

	// Orient the sprite along the player's line of sight (inverted).
	//
	direction[0] = -view_direction_[0];
	direction[1] = -view_direction_[1];

	const auto perpendicular_dx = tile_half_dimension_d * direction[1];
	const auto perpendicular_dy = tile_half_dimension_d * direction[0];

	bottom_left_vertex[0] += -perpendicular_dx;
	bottom_left_vertex[1] += +perpendicular_dy;

	bottom_right_vertex[0] += +perpendicular_dx;
	bottom_right_vertex[1] += -perpendicular_dy;


	auto vertex_index = sprite.vertex_index;

	// Bottom-left.
	{
		auto& vertex = sprites_vbi_[vertex_index++];

		vertex.xyz = VertexPosition
		{
			static_cast<float>(bottom_left_vertex[0]),
			static_cast<float>(bottom_left_vertex[1]),
			0.0F
		};
	}

	// Bottom-right.
	{
		auto& vertex = sprites_vbi_[vertex_index++];

		vertex.xyz = VertexPosition
		{
			static_cast<float>(bottom_right_vertex[0]),
			static_cast<float>(bottom_right_vertex[1]),
			0.0F
		};
	}

	// Top-right.
	{
		auto& vertex = sprites_vbi_[vertex_index++];

		vertex.xyz = VertexPosition
		{
			static_cast<float>(bottom_right_vertex[0]),
			static_cast<float>(bottom_right_vertex[1]),
			tile_dimension_f
		};
	}

	// Top-left.
	{
		auto& vertex = sprites_vbi_[vertex_index++];

		vertex.xyz = VertexPosition
		{
			static_cast<float>(bottom_left_vertex[0]),
			static_cast<float>(bottom_left_vertex[1]),
			tile_dimension_f
		};
	}

	update_cloaked_actor(sprite);
}

void update_static(
	int bs_static_index)
{
	auto& sprite = statics_[bs_static_index];
	const auto& bs_static = statobjlist[bs_static_index];
	auto& hw_static = statics_[bs_static_index];

	if (sprite.kind == SpriteKind::none)
	{
		map_static(bs_static);
	}
	else
	{
		hw_static.bs_sprite_id = bs_static.shapenum;
	}

	orient_sprite(hw_static);
}

void render_sprites()
{
	// Build draw list.
	//
	auto draw_sprite_index = 0;
	auto& draw_items = sprites_draw_list_;

	auto min_vertex_index = max_sprites_vertices;
	auto max_vertex_index = 0;

	for (const auto bs_static_index : statics_to_render_)
	{
		update_static(bs_static_index);

		auto& hw_static = statics_[bs_static_index];

		auto& draw_item = draw_items[draw_sprite_index++];
		draw_item.texture_id = hw_static.bs_sprite_id;
		draw_item.sprite = &hw_static;

		min_vertex_index = std::min(hw_static.vertex_index, min_vertex_index);
		max_vertex_index = std::max(hw_static.vertex_index, max_vertex_index);
	}

	for (const auto bs_actor_index : actors_to_render_)
	{
		update_actor(bs_actor_index);

		auto& actor = actors_[bs_actor_index];
		orient_sprite(actor);

		auto& draw_item = draw_items[draw_sprite_index++];
		draw_item.texture_id = actor.bs_sprite_id;
		draw_item.sprite = &actor;

		min_vertex_index = std::min(actor.vertex_index, min_vertex_index);
		max_vertex_index = std::max(actor.vertex_index, max_vertex_index);
	}

	if (draw_sprite_index == 0)
	{
		return;
	}

	const auto vertex_count = max_vertex_index - min_vertex_index + vertices_per_sprite;

	update_vertex_buffer(
		sprites_vb_,
		min_vertex_index,
		vertex_count,
		&sprites_vbi_[min_vertex_index]
	);

	// Sort by distance (farthest -> nearest).
	//
	std::sort(
		draw_items.begin(),
		draw_items.begin() + draw_sprite_index,
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.sprite->square_distance > rhs.sprite->square_distance;
		}
	);

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = sprites_ibi_;

		for (int i = 0; i < draw_sprite_index; ++i)
		{
			const auto& sprite = *draw_items[i].sprite;

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index + 1);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index + 2);

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index + 2);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(sprite.vertex_index + 3);
		}

		update_index_buffer(
			sprites_ib_,
			0,
			ib_index,
			ib_buffer.data()
		);
	}

	// Add render commands.
	//
	auto& command_buffer = r3_command_buffer_;

	// Disable depth write.
	//
	{
		auto& command = *command_buffer->write_enable_depth_write();
		command.is_enable_ = false;
	}

	// Enable blending.
	//
	{
		auto& command = *command_buffer->write_enable_blending();
		command.is_enable_ = true;
	}

	using CurrentTextureId = bstone::ModValue<int>;

	auto draw_index = 0;
	auto draw_quad_count = 0;
	auto draw_index_offset_ = 0;
	auto current_texture_id = CurrentTextureId{};

	while (draw_index < draw_sprite_index)
	{
		auto is_first = true;
		auto last_texture_id = 0;
		auto last_lighting = 0;

		draw_quad_count = 0;

		while (draw_index < draw_sprite_index)
		{
			const auto& draw_item = draw_items[draw_index];

			const auto texture_id = draw_item.texture_id;

			auto lighting = 0;

			if (!gp_no_shading_)
			{
				const auto& sprite = *draw_item.sprite;

				switch (sprite.kind)
				{
					case SpriteKind::actor:
						lighting = sprite.bs_object.actor->lighting;
						break;

					case SpriteKind::stat:
						lighting = sprite.bs_object.stat->lighting;
						break;

					default:
						assert(!"Invalid sprite kind.");
						break;
				}

				if (lighting > 0)
				{
					lighting = 0;
				}
			}

			if (is_first)
			{
				is_first = false;

				last_texture_id = texture_id;
				last_lighting = lighting;
			}
			else if (last_texture_id == texture_id && last_lighting == lighting)
			{
				++draw_quad_count;
				++draw_index;
			}
			else
			{
				break;
			}
		}

		if (draw_quad_count > 0)
		{
			// Set extra lighting.
			//
			if (!gp_no_shading_)
			{
				bs_lighting_ = last_lighting;

				if (bs_lighting_.is_modified())
				{
					bs_lighting_.set_is_modified(false);

					auto& command = *command_buffer->write_set_float32_uniform();
					command.var_ = extra_lighting_uniform_;
					command.value_ = static_cast<float>(bs_lighting_);
				}
			}

			current_texture_id = last_texture_id;

			if (current_texture_id.is_modified())
			{
				current_texture_id.set_is_modified(false);
				const auto texture_2d = texture_mgr_->get_sprite(current_texture_id);

				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = texture_2d;
			}

			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = sprites_vi_.get();
			}

			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = draw_quad_count * vertices_per_quad;
				param.index_byte_depth_ = 2;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = draw_index_offset_;

				draw_index_offset_ += indices_per_sprite * draw_quad_count;
			}
		}
	}

	// Enable depth write.
	//
	{
		auto& command = *command_buffer->write_enable_depth_write();
		command.is_enable_ = true;
	}

	// Disable blending.
	//
	{
		auto& command = *command_buffer->write_enable_blending();
		command.is_enable_ = false;
	}

	sprites_draw_count_ = draw_sprite_index;
}

void update_3d_fade()
{
	r3_fade_is_enabled_ = false;

	auto r_f = 1.0F;
	auto g_f = 1.0F;
	auto b_f = 1.0F;
	auto a_f = 1.0F;

	const auto& palette_shift_info = palette_shift_get_info();

	if (fizzle_fx_is_enabled_)
	{
		r3_fade_is_enabled_ = true;

		auto ratio = fizzle_fx_ratio_;

		if (!fizzle_fx_is_fading_)
		{
			ratio = 1.0F - ratio;
		}

		const auto vga_color = vgapal + (3 * fizzle_fx_color_index_);
		const auto& color_32 = vga_color_to_rgba_8(vga_color[0], vga_color[1], vga_color[2]);

		r_f = static_cast<float>(color_32.r_) / 255.0F;
		g_f = static_cast<float>(color_32.g_) / 255.0F;
		b_f = static_cast<float>(color_32.b_) / 255.0F;
		a_f = ratio;
	}
	else if (palette_shift_info.is_bonus_shifted_ || palette_shift_info.is_damage_shifted_)
	{
		r3_fade_is_enabled_ = true;

		if (palette_shift_info.is_bonus_shifted_)
		{
			r_f *= palette_shift_info.bonus_r_ / 255.0F;
			g_f *= palette_shift_info.bonus_g_ / 255.0F;
			b_f *= palette_shift_info.bonus_b_ / 255.0F;
			a_f *= palette_shift_info.bonus_a_ / 255.0F;
		}

		if (palette_shift_info.is_damage_shifted_)
		{
			r_f *= palette_shift_info.damage_r_ / 255.0F;
			g_f *= palette_shift_info.damage_g_ / 255.0F;
			b_f *= palette_shift_info.damage_b_ / 255.0F;
			a_f *= palette_shift_info.damage_a_ / 255.0F;
		}
	}

	if (!r3_fade_is_enabled_)
	{
		return;
	}

	const auto r = static_cast<std::uint8_t>(a_f * r_f * 255.0F);
	const auto g = static_cast<std::uint8_t>(a_f * g_f * 255.0F);
	const auto b = static_cast<std::uint8_t>(a_f * b_f * 255.0F);
	const auto a = static_cast<std::uint8_t>(a_f * 255.0F);

	const auto rgba_8 = bstone::Rgba8{r, g, b, a};

	texture_mgr_->update_solid_1x1(bstone::HwTextureMgrSolid1x1Id::fade_3d, rgba_8);
}

void present_3d()
{
	auto& command_buffer = r3_command_buffer_;

	command_buffer->enable(false);

	if (!is_draw_3d_ || screenfaded)
	{
		return;
	}

	const auto is_shading = (!gp_no_shading_);

	const auto& assets_info = get_assets_info();

	update_3d_fade();

	command_buffer->enable(true);

	command_buffer->begin_write();

	// Set viewport.
	//
	{
		auto& viewport = command_buffer->write_set_viewport()->viewport_;
		viewport.x = vid_layout_.screen_viewport_left_width;
		viewport.y = vid_layout_.window_viewport_bottom_height + vid_layout_.screen_viewport_bottom_height;
		viewport.width_ = vid_layout_.screen_viewport_width;
		viewport.height_ = vid_layout_.screen_viewport_height;
		viewport.min_depth_ = 0.0F;
		viewport.max_depth_ = 1.0F;
	}

	// Enable back-face culling.
	//
	{
		auto& command = *command_buffer->write_enable_culling();
		command.is_enable_ = true;
	}

	// Enable depth test.
	//
	{
		auto& command = *command_buffer->write_enable_depth_test();
		command.is_enable_ = true;
	}

	// Enable depth write.
	//
	{
		auto& command = *command_buffer->write_enable_depth_write();
		command.is_enable_ = true;
	}

	// Set model matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = model_mat_uniform_;
		command.value_ = cast_glm_mat4(r3_matrix_model_);
	}

	// Set view matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = view_mat_uniform_;
		command.value_ = cast_glm_mat4(r3_matrix_view_);
	}

	// Set projection matrix.
	//
	{
		auto& command = *command_buffer->write_set_mat4_uniform();
		command.var_ = projection_mat_uniform_;
		command.value_ = cast_glm_mat4(r3_matrix_projection_);
	}

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_set_sampler();
		command.sampler_ = wall_sampler_.get();
	}

	// Set shading mode.
	//
	{
		shading_mode_ = is_shading;

		if (shading_mode_.is_modified())
		{
			shading_mode_.set_is_modified(false);

			auto& command = *command_buffer->write_set_int32_uniform();
			command.var_ = shading_mode_uniform_;
			command.value_ = shading_mode_;
		}
	}

	// Draw solid walls.
	//
	render_walls();

	// Draw pushwalls.
	//
	render_pushwalls();

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_set_sampler();
		command.sampler_ = sprite_sampler_.get();
	}

	// Draw doors.
	//
	render_doors();

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_set_sampler();
		command.sampler_ = wall_sampler_.get();
	}

	// Draw flooring.
	//
	{
		auto texture_2d = (!gp_is_flooring_solid_
			?
			flooring_textured_t2d_
			:
			flooring_solid_t2d_
		);

		{
			auto& command = *command_buffer->write_set_texture();
			command.texture_2d_ = texture_2d;
		}

		{
			auto& command = *command_buffer->write_set_vertex_input();
			command.vertex_input_ = flooring_vi_.get();
		}

		{
			auto& command = *command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed_;

			param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
			param.vertex_count_ = vertices_per_quad;
			param.index_byte_depth_ = 1;
			param.index_buffer_offset_ = 0;
			param.index_offset_ = 0;
		}
	}

	// Draw ceiling.
	//
	{
		auto texture_2d = (!gp_is_ceiling_solid_
			?
			ceiling_textured_t2d_
			:
			ceiling_solid_t2d_
		);

		{
			auto& command = *command_buffer->write_set_texture();
			command.texture_2d_ = texture_2d;
		}

		{
			auto& command = *command_buffer->write_set_vertex_input();
			command.vertex_input_ = ceiling_vi_.get();
		}

		{
			auto& command = *command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed_;

			param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
			param.vertex_count_ = vertices_per_quad;
			param.index_byte_depth_ = 1;
			param.index_buffer_offset_ = 0;
			param.index_offset_ = 0;
		}
	}

	// Set sampler.
	//
	{
		auto& command = *command_buffer->write_set_sampler();
		command.sampler_ = sprite_sampler_.get();
	}

	// Draw statics and actors.
	//
	render_sprites();

	// Disable back-face culling.
	//
	{
		auto& command = *command_buffer->write_enable_culling();
		command.is_enable_ = false;
	}

	// Disable depth test.
	//
	{
		auto& command = *command_buffer->write_enable_depth_test();
		command.is_enable_ = false;
	}

	// Disable shading mode.
	//
	if (is_shading)
	{
		shading_mode_ = 0;

		if (shading_mode_.is_modified())
		{
			shading_mode_.set_is_modified(false);

			auto& command = *command_buffer->write_set_int32_uniform();
			command.var_ = shading_mode_uniform_;
			command.value_ = shading_mode_;
		}
	}

	// Draw player's weapon.
	//
	if (vid_is_hud)
	{
		const auto player_weapon_sprite_id = player_get_weapon_sprite_id();

		if (player_weapon_sprite_id > 0 || r3_fade_is_enabled_)
		{
			// Set projection matrix.
			//
			{
				auto& command = *command_buffer->write_set_mat4_uniform();
				command.var_ = projection_mat_uniform_;
				command.value_ = cast_glm_mat4(player_weapon_projection_matrix_);
			}
		}

		if (player_weapon_sprite_id > 0)
		{
			const auto player_weapon_texture = texture_mgr_->get_sprite(player_weapon_sprite_id);

			if (assets_info.is_ps())
			{
				update_player_weapon_model_matrix();
			}

			// Set model matrix.
			//
			{
				auto& command = *command_buffer->write_set_mat4_uniform();
				command.var_ = model_mat_uniform_;
				command.value_ = cast_glm_mat4(player_weapon_model_matrix_);
			}

			// Set view matrix.
			//
			{
				auto& command = *command_buffer->write_set_mat4_uniform();
				command.var_ = view_mat_uniform_;
				command.value_ = cast_glm_mat4(player_weapon_view_matrix_);
			}

			// Set texture.
			//
			{
				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = player_weapon_texture;
			}

			// Set sampler.
			//
			{
				auto& command = *command_buffer->write_set_sampler();
				command.sampler_ = player_weapon_sampler_.get();
			}

			// Set vertex input.
			//
			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = player_weapon_vi_.get();
			}

			// Enable blending.
			//
			{
				auto& command = *command_buffer->write_enable_blending();
				command.is_enable_ = true;
			}

			// Set blending function.
			//
			{
				auto& blending_func = command_buffer->write_set_blending_func()->blending_func_;
				blending_func.src_factor_ = bstone::Ren3dBlendingFactor::src_alpha;
				blending_func.dst_factor_ = bstone::Ren3dBlendingFactor::one_minus_src_alpha;
			}

			// Draw the weapon.
			//
			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = vertices_per_quad;
				param.index_byte_depth_ = 1;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = 0;
			}

			// Disable blending.
			//
			{
				auto& command = *command_buffer->write_enable_blending();
				command.is_enable_ = false;
			}
		}


		// 3D fade (bonus, damage, death, etc).
		//
		if (r3_fade_is_enabled_)
		{
			// Set model matrix.
			//
			{
				auto& command = *command_buffer->write_set_mat4_uniform();
				command.var_ = model_mat_uniform_;
				command.value_ = cast_glm_mat4(glm::identity<glm::mat4>());
			}

			// Set view matrix.
			//
			{
				auto& command = *command_buffer->write_set_mat4_uniform();
				command.var_ = view_mat_uniform_;
				command.value_ = cast_glm_mat4(glm::identity<glm::mat4>());
			}

			// Enable blending.
			//
			{
				auto& command = *command_buffer->write_enable_blending();
				command.is_enable_ = true;
			}

			// Set blending function.
			//
			{
				auto& blending_func = command_buffer->write_set_blending_func()->blending_func_;
				blending_func.src_factor_ = bstone::Ren3dBlendingFactor::src_alpha;
				blending_func.dst_factor_ = bstone::Ren3dBlendingFactor::one_minus_src_alpha;
			}

			// Set texture.
			//
			{
				auto& command = *command_buffer->write_set_texture();
				command.texture_2d_ = r3_fade_t2d_;
			}

			// Set sampler.
			//
			{
				auto& command = *command_buffer->write_set_sampler();
				command.sampler_ = fade_sampler_.get();
			}

			// Set vertex input.
			//
			{
				auto& command = *command_buffer->write_set_vertex_input();
				command.vertex_input_ = r3_fade_vi_.get();
			}

			// Draw the quad.
			//
			{
				auto& command = *command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed_;

				param.primitive_topology_ = bstone::Ren3dPrimitiveTopology::triangle_list;
				param.vertex_count_ = vertices_per_quad;
				param.index_byte_depth_ = 1;
				param.index_buffer_offset_ = 0;
				param.index_offset_ = 0;
			}

			// Disable blending.
			//
			{
				auto& command = *command_buffer->write_enable_blending();
				command.is_enable_ = false;
			}
		}
	}

	// Finalize.
	//
	command_buffer->end_write();
}

bool bs_is_activated_pushwall_tile(
	int tile)
{
	return (tile & tilemap_door_flags) == tilemap_door_flags;
}

bool bs_is_door(
	int tile)
{
	return (tile & tilemap_door_flags) == tilemap_door_flag;
}

bool bs_is_door_track(
	int tile)
{
	return (tile & tilemap_door_flags) == tilemap_door_track_flag;
}

bool bs_is_solid_wall(
	int tile)
{
	if (tile == 0)
	{
		return false;
	}

	if (bs_is_activated_pushwall_tile(tile))
	{
		return false;
	}

	if (bs_is_door(tile))
	{
		return false;
	}

	return true;
}

bool bs_is_pushwall(
	int x,
	int y)
{
	if (x < 0 || x >= MAPSIZE || y < 0 || y >= MAPSIZE)
	{
		return false;
	}

	const auto tile_wall = tilemap[x][y];

	if (tile_wall == 0)
	{
		return false;
	}

	if (bs_is_activated_pushwall_tile(tile_wall))
	{
		return true;
	}

	const auto tile_object = mapsegs[1][(MAPSIZE * y) + x];

	if (tile_object == PUSHABLETILE)
	{
		return true;
	}

	return false;
}

bool bs_is_solid_wall(
	int x,
	int y)
{
	if (x < 0 || x >= MAPSIZE || y < 0 || y >= MAPSIZE)
	{
		return true;
	}

	const auto tile_wall = tilemap[x][y];

	if (!bs_is_solid_wall(tile_wall))
	{
		return false;
	}

	if (bs_is_pushwall(x, y))
	{
		return false;
	}

	if (has_active_pushwall_ &&
		active_pushwall_next_x_ == x &&
		active_pushwall_next_y_ == y)
	{
		return false;
	}

	return true;
}

int get_solid_wall_side_count(
	int x,
	int y)
{
	assert(x >= 0 && x < MAPSIZE);
	assert(y >= 0 && y < MAPSIZE);
	assert(bs_is_solid_wall(x, y));

	auto side_count = 4;

	side_count -= bs_is_solid_wall(x + 0, y - 1); // north
	side_count -= bs_is_solid_wall(x + 1, y + 0); // east
	side_count -= bs_is_solid_wall(x + 0, y + 1); // south
	side_count -= bs_is_solid_wall(x - 1, y + 0); // west

	return side_count;
}

int get_door_track_wall_id(
	int x,
	int y,
	controldir_t direction)
{
	auto bs_door_x = x;
	auto bs_door_y = y;

	switch (direction)
	{
		case di_north:
			bs_door_y -= 1;
			break;

		case di_east:
			bs_door_x += 1;
			break;

		case di_south:
			bs_door_y += 1;
			break;

		case di_west:
			bs_door_x -= 1;
			break;

		default:
			fail("Invalid direction.");
	}

	if (bs_door_x < 0 || bs_door_x >= MAPSIZE || bs_door_y < 0 || bs_door_y >= MAPSIZE)
	{
		return -1;
	}

	const auto bs_door_tile = tilemap[bs_door_x][bs_door_y];

	if (!bs_is_door(bs_door_tile))
	{
		return -1;
	}

	const auto door_index = bs_door_tile & tilemap_wall_mask;
	const auto& door = doorobjlist[door_index];

	return door_get_track_texture_id(door);
}

void precache_flooring()
{
	texture_mgr_->cache_wall(FloorTile);
	flooring_textured_t2d_ = texture_mgr_->get_wall(FloorTile);

	const auto vga_index = BottomColor & 0xFF;
	const auto vga_color = vgapal + (3 * vga_index);

	const auto renderer_color = vga_color_to_rgba_8(
		vga_color[0],
		vga_color[1],
		vga_color[2]
	);

	texture_mgr_->update_solid_1x1(bstone::HwTextureMgrSolid1x1Id::flooring, renderer_color);
}

void precache_ceiling()
{
	texture_mgr_->cache_wall(CeilingTile);
	ceiling_textured_t2d_ = texture_mgr_->get_wall(CeilingTile);

	const auto vga_index = TopColor & 0xFF;
	const auto vga_color = vgapal + (3 * vga_index);

	const auto renderer_color = vga_color_to_rgba_8(
		vga_color[0],
		vga_color[1],
		vga_color[2]
	);

	texture_mgr_->update_solid_1x1(bstone::HwTextureMgrSolid1x1Id::ceiling, renderer_color);
}

void precache_wall(
	int wall_id)
{
	texture_mgr_->cache_wall(wall_id);
}

void precache_horizontal_wall(
	int tile_wall)
{
	const auto wall_id = horizwall[tile_wall];

	precache_wall(wall_id);
}

void precache_vertical_wall(
	int tile_wall)
{
	const auto wall_id = vertwall[tile_wall];

	precache_wall(wall_id);
}

void precache_switches()
{
	precache_horizontal_wall(OFF_SWITCH);
	precache_vertical_wall(OFF_SWITCH);
	precache_horizontal_wall(ON_SWITCH);
	precache_vertical_wall(ON_SWITCH);
}

void precache_door_track(
	int x,
	int y)
{
	const auto tile = tilemap[x][y];
	const auto tile_wall = tile & tilemap_wall_mask;

	const auto& bs_door = doorobjlist[tile_wall];

	if (bs_door.tilex != x || bs_door.tiley != y)
	{
		fail("Expected a door at (" + std::to_string(x) + ", " + std::to_string(y) + ").");
	}

	const auto wall_id = door_get_track_texture_id(bs_door);

	precache_wall(wall_id);
}

void precache_walls()
{
	auto has_switch = false;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			const auto tile = tilemap[x][y];
			const auto tile_wall = tile & tilemap_wall_mask;

			if (bs_is_door(tile))
			{
				precache_door_track(x, y);

				continue;
			}

			if (!bs_is_solid_wall(x, y))
			{
				continue;
			}

			if (tile_wall == ON_SWITCH || tile_wall == OFF_SWITCH)
			{
				has_switch = true;
			}

			precache_horizontal_wall(tile_wall);
			precache_vertical_wall(tile_wall);
		}
	}

	if (has_switch)
	{
		precache_switches();
	}
}

void precache_pushwalls()
{
	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!bs_is_pushwall(x, y))
			{
				continue;
			}

			const auto tile = tilemap[x][y];
			const auto tile_wall = tile & tilemap_wall_mask;

			precache_horizontal_wall(tile_wall);
			precache_vertical_wall(tile_wall);
		}
	}
}

void precache_door_side(
	int page_number)
{
	texture_mgr_->cache_wall(page_number);
}

void precache_door(
	const doorobj_t& door)
{
	auto horizontal_locked_page_number = 0;
	auto horizontal_unlocked_page_number = 0;
	auto vertical_locked_page_number = 0;
	auto vertical_unlocked_page_number = 0;

	door_get_page_numbers_for_caching(
		door,
		horizontal_locked_page_number,
		horizontal_unlocked_page_number,
		vertical_locked_page_number,
		vertical_unlocked_page_number
	);

	precache_door_side(horizontal_locked_page_number);
	precache_door_side(horizontal_unlocked_page_number);
	precache_door_side(vertical_locked_page_number);
	precache_door_side(vertical_unlocked_page_number);
}

void precache_doors()
{
	door_count_ = 0;

	for (auto bs_door = doorobjlist; lastdoorobj && bs_door != lastdoorobj; ++bs_door)
	{
		++door_count_;

		precache_door(*bs_door);
	}
}

template<
	typename TVertex,
	bool TIsExists = false
>
struct UpdateVertexXyz
{
	void operator()(
		TVertex& vertex,
		const VertexPosition& xyz) const
	{
		static_cast<void>(xyz);
	}
}; // UpdateVertexXyz

template<
	typename TVertex
>
struct UpdateVertexXyz<TVertex, true>
{
	void operator()(
		TVertex& vertex,
		const VertexPosition& xyz) const
	{
		vertex.xyz = xyz;
	}
}; // UpdateVertexXyz

template<
	typename TVertex
>
void update_vertex_xyz(
	TVertex& vertex,
	const VertexPosition& xyz)
{
	const auto traits = VertexAttributeTraits<TVertex, VertexAttribLocationId::position>{};

	UpdateVertexXyz<TVertex, traits.is_valid>{}(vertex, xyz);
}

template<
	typename TVertex,
	bool TIsExists = false
>
struct UpdateVertexRgba
{
	void operator()(
		TVertex& vertex,
		const VertexColor& rgba_8) const
	{
		static_cast<void>(vertex);
		static_cast<void>(rgba_8);
	}
}; // UpdateVertexRgba

template<
	typename TVertex
>
struct UpdateVertexRgba<TVertex, true>
{
	void operator()(
		TVertex& vertex,
		const VertexColor& rgba_8) const
	{
		vertex.rgba8 = rgba_8;
	}
}; // UpdateVertexRgba

template<
	typename TVertex
>
void update_vertex_rgba(
	TVertex& vertex,
	const VertexColor& rgba_8)
{
	const auto traits = VertexAttributeTraits<TVertex, VertexAttribLocationId::color>{};

	UpdateVertexRgba<TVertex, traits.is_valid>{}(vertex, rgba_8);
}

template<
	typename TVertex,
	bool TIsExists = false
>
struct UpdateVertexUv
{
	void operator()(
		TVertex& vertex,
		const VertexTextureCoordinates& uv) const
	{
		static_cast<void>(uv);
	}
}; // UpdateVertexUv

template<
	typename TVertex
>
struct UpdateVertexUv<TVertex, true>
{
	void operator()(
		TVertex& vertex,
		const VertexTextureCoordinates& uv) const
	{
		vertex.uv = uv;
	}
}; // UpdateVertexUv

template<
	typename TVertex
>
void update_vertex_uv(
	TVertex& vertex,
	const VertexTextureCoordinates& uv)
{
	const auto traits = VertexAttributeTraits<TVertex, VertexAttribLocationId::texture_coordinates>{};

	UpdateVertexUv<TVertex, traits.is_valid>{}(vertex, uv);
}

template<
	typename TVertex
>
void map_wall_side(
	controldir_t side_direction,
	Wall& wall,
	int& vertex_index,
	VertexBufferImageT<TVertex>& vb_buffer)
{
	static const float all_vertex_offsets[4][4] =
	{
		{tile_dimension_f, 0.0F, 0.0F, 0.0F,},
		{tile_dimension_f, tile_dimension_f, tile_dimension_f, 0.0F,},
		{0.0F, tile_dimension_f, tile_dimension_f, tile_dimension_f,},
		{0.0F, 0.0F, 0.0F, tile_dimension_f,},
	};


	const auto x = wall.x;
	const auto y = wall.y;

	const auto tile = tilemap[x][y];
	const auto wall_id = tile & tilemap_wall_mask;
	const auto has_door_tracks = bs_is_door_track(tile);

	auto is_vertical = false;
	auto wall_texture_id = 0;

	switch (side_direction)
	{
	case di_north:
	case di_south:
		wall_texture_id = horizwall[wall_id];
		break;

	case di_east:
	case di_west:
		is_vertical = true;
		wall_texture_id = vertwall[wall_id];
		break;

	default:
		fail("Invalid direction.");
	}

	auto bs_is_door_track = false;

	if (has_door_tracks)
	{
		const auto door_track_wall_id = get_door_track_wall_id(x, y, side_direction);

		if (door_track_wall_id >= 0)
		{
			bs_is_door_track = true;

			wall_texture_id = door_track_wall_id;
		}
	}

	const auto& vertex_offsets = all_vertex_offsets[side_direction];

	auto& side = wall.sides[side_direction];

	side.flags.is_active = true;
	side.flags.is_vertical = is_vertical;
	side.flags.is_door_track = bs_is_door_track;
	side.vertex_index = vertex_index;
	side.texture_id = wall_texture_id;
	side.wall = &wall;

	const auto& rgba_8 = VertexColor{0xFF, 0xFF, 0xFF, 0xFF};

	// Bottom-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		const auto& xyz = VertexPosition
		{
			static_cast<float>(x) + vertex_offsets[0],
			static_cast<float>(y) + vertex_offsets[1],
			0.0F,
		};

		const auto& uv = VertexTextureCoordinates{0.0F, 0.0F};

		update_vertex_xyz(vertex, xyz);
		update_vertex_rgba(vertex, rgba_8);
		update_vertex_uv(vertex, uv);
	}

	// Bottom-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		const auto& xyz = VertexPosition
		{
			static_cast<float>(x) + vertex_offsets[2],
			static_cast<float>(y) + vertex_offsets[3],
			0.0F,
		};

		const auto& uv = VertexTextureCoordinates{1.0F, 0.0F};

		update_vertex_xyz(vertex, xyz);
		update_vertex_rgba(vertex, rgba_8);
		update_vertex_uv(vertex, uv);
	}

	// Top-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		const auto& xyz = VertexPosition
		{
			static_cast<float>(x) + vertex_offsets[2],
			static_cast<float>(y) + vertex_offsets[3],
			map_height_f,
		};

		const auto& uv = VertexTextureCoordinates{1.0F, 1.0F};

		update_vertex_xyz(vertex, xyz);
		update_vertex_rgba(vertex, rgba_8);
		update_vertex_uv(vertex, uv);
	}

	// Top-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		const auto& xyz = VertexPosition
		{
			static_cast<float>(x) + vertex_offsets[0],
			static_cast<float>(y) + vertex_offsets[1],
			map_height_f,
		};

		const auto& uv = VertexTextureCoordinates{0.0F, 1.0F};

		update_vertex_xyz(vertex, xyz);
		update_vertex_rgba(vertex, rgba_8);
		update_vertex_uv(vertex, uv);
	}
}

template<
	typename TVertex
>
void map_xy_to_xwall(
	XyWallKind wall_kind,
	int x,
	int y,
	XyWallMap& map,
	int& vertex_index,
	VertexBufferImageT<TVertex>& vb_buffer)
{
	switch (wall_kind)
	{
	case XyWallKind::solid:
	case XyWallKind::push:
		break;

	default:
		fail("Invalid wall kind.");
	}

	auto is_north_solid = false;
	auto is_east_solid = false;
	auto is_south_solid = false;
	auto is_west_solid = false;

	if (wall_kind == XyWallKind::solid)
	{
		is_north_solid = bs_is_solid_wall(x + 0, y - 1);
		is_east_solid = bs_is_solid_wall(x + 1, y + 0);
		is_south_solid = bs_is_solid_wall(x + 0, y + 1);
		is_west_solid = bs_is_solid_wall(x - 1, y + 0);
	}

	if (is_north_solid && is_east_solid && is_south_solid && is_west_solid)
	{
		// Nothing to draw.
		// This solid wall is surrounded by other solid ones.

		return;
	}

	const auto xy = encode_xy(x, y);

	if (map.find(xy) != map.cend())
	{
		Quit("Wall mapping already exist.");
	}

	map[xy] = Wall{};
	auto& wall = map[xy];

	wall.x = x;
	wall.y = y;

	// A north side.
	if (!is_north_solid)
	{
		map_wall_side(
			di_north,
			wall,
			vertex_index,
			vb_buffer
		);
	}

	// An east side.
	if (!is_east_solid)
	{
		map_wall_side(
			di_east,
			wall,
			vertex_index,
			vb_buffer
		);
	}

	// An south side.
	if (!is_south_solid)
	{
		map_wall_side(
			di_south,
			wall,
			vertex_index,
			vb_buffer
		);
	}

	// A west side.
	if (!is_west_solid)
	{
		map_wall_side(
			di_west,
			wall,
			vertex_index,
			vb_buffer
		);
	}
}

void build_walls()
{
	uninitialize_walls();

	// Check for moving pushwall.
	//
	has_active_pushwall_ = (pwallstate != 0.0);

	active_pushwall_next_x_ = 0;
	active_pushwall_next_y_ = 0;

	if (has_active_pushwall_)
	{
		active_pushwall_next_x_ = pwallx;
		active_pushwall_next_y_ = pwally;

		switch (pwalldir)
		{
		case di_north:
			--active_pushwall_next_y_;
			break;

		case di_east:
			++active_pushwall_next_x_;
			break;

		case di_south:
			++active_pushwall_next_y_;
			break;

		case di_west:
			--active_pushwall_next_x_;
			break;

		default:
			Quit("Invalid direction.");
		}
	}

	// Count walls and their sides.
	//
	wall_count_ = 0;
	wall_side_count_ = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!bs_is_solid_wall(x, y))
			{
				continue;
			}

			if (has_active_pushwall_ &&
				x == active_pushwall_next_x_ &&
				y == active_pushwall_next_y_)
			{
				continue;
			}

			wall_count_ += 1;
			wall_side_count_ += get_solid_wall_side_count(x, y);
		}
	}

	// Check for maximums.
	//
	wall_count_ += pushwall_count_;
	wall_side_count_ += pushwall_count_ * sides_per_wall;

	const auto index_count = wall_side_count_ * indices_per_wall_side;

	if (index_count == 0)
	{
		return;
	}

	if (index_count > max_wall_sides_indices)
	{
		fail("Too many wall indices.");
	}

	// Create index an vertex buffers.
	//
	initialize_walls();

	// Build the map (XY to wall).
	//
	const auto vertex_count = wall_side_count_ * vertices_per_wall_side;

	auto vb_buffer = WallsVbi{};
	vb_buffer.resize(vertex_count);

	xy_wall_map_.clear();

	auto vertex_index = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!bs_is_solid_wall(x, y))
			{
				continue;
			}

			if (has_active_pushwall_ &&
				x == active_pushwall_next_x_ &&
				y == active_pushwall_next_y_)
			{
				continue;
			}

			map_xy_to_xwall(
				XyWallKind::solid,
				x,
				y,
				xy_wall_map_,
				vertex_index,
				vb_buffer
			);
		}
	}

	wall_vertex_count_ = vertex_index;

	// Update vertex buffer.
	//
	update_vertex_buffer(
		wall_sides_vb_,
		0,
		vertex_count,
		vb_buffer.data()
	);

	walls_to_render_.reserve(wall_count_);
}

void translate_pushwall_side(
	float translate_x,
	float translate_y,
	controldir_t side_direction,
	const Wall& wall,
	int& vertex_index,
	PushwallsVbi& vb_buffer)
{
	static const float all_vertex_offsets[4][4] =
	{
		{tile_dimension_f, 0.0F, 0.0F, 0.0F,},
		{tile_dimension_f, tile_dimension_f, tile_dimension_f, 0.0F,},
		{0.0F, tile_dimension_f, tile_dimension_f, tile_dimension_f,},
		{0.0F, 0.0F, 0.0F, tile_dimension_f,},
	};


	switch (side_direction)
	{
	case di_north:
	case di_south:
	case di_east:
	case di_west:
		break;

	default:
		fail("Invalid direction.");
	}

	const auto& vertex_offsets = all_vertex_offsets[side_direction];

	const auto x_f = static_cast<float>(wall.x) + translate_x;
	const auto y_f = static_cast<float>(wall.y) + translate_y;

	// Bottom-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz = VertexPosition
		{
			x_f + vertex_offsets[0],
			y_f + vertex_offsets[1],
			0.0F,
		};
	}

	// Bottom-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz = VertexPosition
		{
			x_f + vertex_offsets[2],
			y_f + vertex_offsets[3],
			0.0F,
		};
	}

	// Top-right (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz = VertexPosition
		{
			x_f + vertex_offsets[2],
			y_f + vertex_offsets[3],
			map_height_f,
		};
	}

	// Top-left (when looking at face side).
	{
		auto& vertex = vb_buffer[vertex_index++];

		vertex.xyz = VertexPosition
		{
			x_f + vertex_offsets[0],
			y_f + vertex_offsets[1],
			map_height_f,
		};
	}
}

void translate_pushwall(
	const Wall& wall,
	int& vertex_index,
	PushwallsVbi& vb_buffer)
{
	auto translate_distance = static_cast<float>(pwallpos);

	auto translate_x = 0.0F;
	auto translate_y = 0.0F;

	switch (pwalldir)
	{
	case di_north:
		translate_y = -translate_distance;
		break;

	case di_east:
		translate_x = +translate_distance;
		break;

	case di_south:
		translate_y = +translate_distance;
		break;

	case di_west:
		translate_x = -translate_distance;
		break;

	default:
		fail("Invalid direction.");
	}

	translate_pushwall_side(
		translate_x,
		translate_y,
		di_north,
		wall,
		vertex_index,
		vb_buffer
	);

	translate_pushwall_side(
		translate_x,
		translate_y,
		di_east,
		wall,
		vertex_index,
		vb_buffer
	);

	translate_pushwall_side(
		translate_x,
		translate_y,
		di_south,
		wall,
		vertex_index,
		vb_buffer
	);

	translate_pushwall_side(
		translate_x,
		translate_y,
		di_west,
		wall,
		vertex_index,
		vb_buffer
	);
}

void translate_pushwall()
{
	const auto xy = encode_xy(pwallx, pwally);

	const auto wall_item_it = xy_pushwall_map_.find(xy);

	if (wall_item_it == xy_pushwall_map_.cend())
	{
		fail("Pushwall mapping not found.");
	}

	const auto& wall = wall_item_it->second;

	const auto first_vertex_index = wall.sides.front().vertex_index;

	auto vertex_index = first_vertex_index;

	translate_pushwall(wall, vertex_index, pushwalls_vbi_);

	const auto vertex_count = vertex_index - first_vertex_index;

	update_vertex_buffer(
		pushwall_sides_vb_,
		first_vertex_index,
		vertex_count,
		&pushwalls_vbi_[first_vertex_index]
	);
}

void step_pushwall(
	int old_x,
	int old_y)
{
	const auto old_xy = encode_xy(old_x, old_y);

	const auto old_wall_item_it = xy_pushwall_map_.find(old_xy);

	if (old_wall_item_it == xy_pushwall_map_.cend())
	{
		fail("Pushwall mapping not found.");
	}

	const auto new_xy = encode_xy(pwallx, pwally);

	auto wall = old_wall_item_it->second;
	wall.x = pwallx;
	wall.y = pwally;

	static_cast<void>(xy_pushwall_map_.erase(old_xy));
	xy_pushwall_map_[new_xy] = wall;

	translate_pushwall();
}

void build_pushwalls()
{
	uninitialize_pushwalls();

	// Count pushwalls and their sides.
	//
	pushwall_count_ = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!bs_is_pushwall(x, y))
			{
				continue;
			}

			pushwall_count_ += 1;
		}
	}

	pushwall_side_count_ = pushwall_count_ * sides_per_wall;

	// Check for maximums.
	//
	const auto index_count = pushwall_side_count_ * indices_per_wall_side;

	if (index_count > max_wall_sides_indices)
	{
		fail("Too many pushwall indices.");
	}

	if (pushwall_count_ == 0)
	{
		return;
	}

	// Create index an vertex buffers.
	//
	initialize_pushwalls();

	// Build the map (XY to pushwall).
	//
	const auto vertex_count = pushwall_side_count_ * vertices_per_wall_side;

	pushwalls_vbi_.clear();
	pushwalls_vbi_.resize(vertex_count);

	xy_pushwall_map_.clear();

	auto vertex_index = 0;

	for (int y = 0; y < MAPSIZE; ++y)
	{
		for (int x = 0; x < MAPSIZE; ++x)
		{
			if (!bs_is_pushwall(x, y))
			{
				continue;
			}

			map_xy_to_xwall(
				XyWallKind::push,
				x,
				y,
				xy_pushwall_map_,
				vertex_index,
				pushwalls_vbi_
			);
		}
	}

	// Update vertex buffer.
	//
	update_vertex_buffer(
		pushwall_sides_vb_,
		0,
		vertex_count,
		pushwalls_vbi_.data()
	);


	pushwalls_to_render_.reserve(pushwall_count_);
}

template<
	typename TVertex
>
void update_quad_vertices(
	QuadFlags flags,
	const VertexPosition& origin,
	const glm::vec2& size,
	int& vertex_index,
	VertexBufferImageT<TVertex>& vb_buffer)
{
	//
	// Front face order:
	//    bottom-left -> bottom-right -> top-right -> top-left
	//
	// Back face order:
	//    bottom-right -> bottom-left -> top-left -> top-right
	//

	const auto axis_index = (flags.is_vertical ? 1 : 0);

	for (int i = 0; i < 4; ++i)
	{
		auto& xyz = vb_buffer[vertex_index + i].xyz;

		xyz = origin;

		switch (i)
		{
		case 0:
			if (flags.is_back_face)
			{
				xyz[axis_index] += size[0];
			}

			break;

		case 1:
			if (!flags.is_back_face)
			{
				xyz[axis_index] += size[0];
			}

			break;

		case 2:
			if (!flags.is_back_face)
			{
				xyz[axis_index] += size[0];
			}

			xyz[2] = 1.0F;

			break;

		case 3:
			if (flags.is_back_face)
			{
				xyz[axis_index] += size[0];
			}

			xyz[2] = 1.0F;

			break;
		}
	}

	vertex_index += 4;
}

void map_door_side(
	DoorSide& door_side,
	int& vertex_index,
	DoorsVbi& vb_buffer)
{
	const auto& door = *door_side.door;
	const auto bs_door_index = door.bs_door_index;
	const auto& bs_door = doorobjlist[bs_door_index];
	const auto door_offset = static_cast<float>(0.5 * doorposition[bs_door_index]);

	auto flags = QuadFlags{};
	flags.is_back_face = door_side.is_back_face;
	flags.is_vertical = bs_door.vertical;

	const auto origin_axis_index = (flags.is_vertical ? 1 : 0);

	const auto size = glm::vec2{0.5F, 1.0F};

	const auto tile_center = VertexPosition{bs_door.tilex + 0.5F, bs_door.tiley + 0.5F, 0.0F};

	const auto left_offset = -(0.5F + door_offset);
	const auto right_offset = door_offset;

	for (int i = 0; i < 2; ++i)
	{
		auto origin = tile_center;
		auto offset = 0.0F;

		if (i == 0)
		{
			offset = left_offset;
		}
		else
		{
			offset = right_offset;
		}

		origin[origin_axis_index] += offset;

		update_quad_vertices(
			flags,
			origin,
			size,
			vertex_index,
			vb_buffer
		);
	}
}

void map_xy_to_door(
	const doorobj_t& bs_door,
	int& vertex_index,
	DoorsVbi& vb_buffer)
{
	const auto xy = encode_xy(bs_door.tilex, bs_door.tiley);

	const auto map_it = xy_door_map_.find(xy);

	if (map_it != xy_door_map_.cend())
	{
		fail("Door already mapped.");
	}

	xy_door_map_[xy] = Door{};
	auto& door = xy_door_map_[xy];

	const auto bs_door_index = static_cast<int>(&bs_door - doorobjlist);
	door.bs_door_index = bs_door_index;
	door.vertex_index = vertex_index;

	for (int i = 0; i < door_halves_per_side; ++i)
	{
		auto u_0 = 0.0F;
		auto u_1 = 0.0F;

		switch (i)
		{
		case 0:
			u_0 = 0.0F;
			u_1 = 0.5F;
			break;

		case 1:
			u_0 = 0.5F;
			u_1 = 1.0F;
			break;
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.uv = VertexTextureCoordinates{u_0, 0.0F};
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.uv = VertexTextureCoordinates{u_1, 0.0F};
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.uv = VertexTextureCoordinates{u_1, 1.0F};
		}

		{
			auto& vertex = vb_buffer[vertex_index++];
			vertex.uv = VertexTextureCoordinates{u_0, 1.0F};
		}
	}

	auto is_back_face = false;

	for (auto& door_side : door.sides)
	{
		door_side.door = &door;
		door_side.is_back_face = is_back_face;

		is_back_face = !is_back_face;
	}

	vertex_index = door.vertex_index;

	map_door_side(door.sides.front(), vertex_index, vb_buffer);

	auto front_face_page_number = 0;
	auto back_face_page_number = 0;

	door_get_page_numbers(bs_door, front_face_page_number, back_face_page_number);

	door.sides[0].texture_id = front_face_page_number;
	door.sides[1].texture_id = back_face_page_number;
}

void build_doors()
{
	uninitialize_door_sides();

	if (door_count_ == 0)
	{
		return;
	}

	// Create index an vertex buffers.
	//
	initialize_door_sides();

	// Build the map (XY to door).
	//
	const auto vertex_count = vertices_per_door * door_count_;

	doors_vbi_.clear();
	doors_vbi_.resize(vertex_count);

	xy_door_map_.clear();

	auto vertex_index = 0;

	for (auto bs_door = doorobjlist; bs_door != lastdoorobj; ++bs_door)
	{
		map_xy_to_door(
			*bs_door,
			vertex_index,
			doors_vbi_
		);
	}

	// Update vertex buffer.
	//
	update_vertex_buffer(
		door_sides_vb_,
		0,
		vertex_count,
		doors_vbi_.data()
	);

	doors_to_render_.reserve(door_count_);
}

void initialize_sprites_ibi()
{
	log("Creating sprite index buffer.");

	const auto index_count = max_sprites_indices;

	sprites_ib_ = create_index_buffer(
		bstone::Ren3dBufferUsageKind::draw_streaming,
		2,
		index_count
	);

	sprites_ibi_.clear();
	sprites_ibi_.resize(index_count);
}

void uninitialize_sprites_ib()
{
	destroy_index_buffer(sprites_ib_);
	sprites_ibi_.clear();
}

void initialize_sprites_vb()
{
	log("Creating sprite vertex buffer.");

	const auto vertex_count = max_sprites_vertices;

	sprites_vb_ = create_vertex_buffer<SpriteVertex>(
		bstone::Ren3dBufferUsageKind::draw_streaming,
		vertex_count
	);

	sprites_vbi_.resize(vertex_count);
}

void uninitialize_sprites_vb()
{
	destroy_vertex_buffer(sprites_vb_);
	sprites_vbi_.clear();
}

void uninitialize_sprites_vi()
{
	destroy_vertex_input(sprites_vi_);
}

void initialize_sprites_vi()
{
	log("Creating sprite vertex input.");

	create_vertex_input<SpriteVertex>(
		sprites_ib_,
		sprites_vb_,
		sprites_vi_);
}

void initialize_statics()
{
	statics_.resize(MAXSTATS);

	statics_to_render_.clear();
	statics_to_render_.reserve(MAXSTATS);
}

void initialize_actors()
{
	actors_.resize(MAXACTORS);

	actors_to_render_.clear();
	actors_to_render_.reserve(MAXACTORS);
}

void initialize_sprites()
{
	log("Initializing sprites.");

	sprites_draw_count_ = 0;
	sprites_draw_list_.clear();
	sprites_draw_list_.resize(max_sprites);

	initialize_sprites_ibi();
	initialize_sprites_vb();
	initialize_sprites_vi();
	initialize_statics();
	initialize_actors();
}

void uninitialize_statics()
{
	statics_.clear();
}

void uninitialize_actors()
{
	actors_.clear();
}

void uninitialize_sprites()
{
	uninitialize_statics();
	uninitialize_actors();

	sprites_draw_count_ = 0;
	sprites_draw_list_.clear();

	uninitialize_sprites_vi();
	uninitialize_sprites_ib();
	uninitialize_sprites_vb();
}

void map_sprite(
	SpriteKind sprite_kind,
	int vertex_index,
	Sprite& sprite)
{
	sprite.kind = sprite_kind;
	sprite.vertex_index = vertex_index;

	// Bottom-left.
	//
	{
		auto& vertex = sprites_vbi_[vertex_index++];
		vertex.rgba8 = VertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv = VertexTextureCoordinates{0.0F, 0.0F};
	}

	// Bottom-right.
	//
	{
		auto& vertex = sprites_vbi_[vertex_index++];
		vertex.rgba8 = VertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv = VertexTextureCoordinates{1.0F, 0.0F};
	}

	// Top-right.
	//
	{
		auto& vertex = sprites_vbi_[vertex_index++];
		vertex.rgba8 = VertexColor{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv = VertexTextureCoordinates{1.0F, 1.0F};
	}

	// Top-left.
	//
	{
		auto& vertex = sprites_vbi_[vertex_index];
		vertex.rgba8 = bstone::Rgba8{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv = VertexTextureCoordinates{0.0F, 1.0F};
	}
}

void map_static(
	const statobj_t& bs_static)
{
	const auto bs_static_index = static_cast<int>(&bs_static - statobjlist.data());

	auto vertex_index = statics_base_vertex_index;
	vertex_index += (bs_static_index * vertices_per_sprite);

	auto& sprite = statics_[bs_static_index];
	sprite.tile_x = bs_static.tilex;
	sprite.tile_y = bs_static.tiley;
	sprite.bs_object.stat = &bs_static;
	sprite.bs_sprite_id = bs_static.shapenum;

	map_sprite(SpriteKind::stat, vertex_index, sprite);
}

void cache_sprite(
	int bs_sprite_id)
{
	texture_mgr_->cache_sprite(bs_sprite_id);
}

void precache_water_bowl()
{
	cache_sprite(SPR_STAT_40); // Full.
	cache_sprite(SPR_STAT_41); // Empty.
}

void precache_chicken_leg()
{
	cache_sprite(SPR_STAT_42); // Intact.
	cache_sprite(SPR_STAT_43); // Gnawed.
}

void precache_ham()
{
	cache_sprite(SPR_STAT_44); // Intact.
	cache_sprite(SPR_STAT_45); // Bone.
}

void precache_candy_bar()
{
	cache_sprite(SPR_CANDY_BAR); // Intact.
	cache_sprite(SPR_CANDY_WRAPER); // Wrapper.
}

void precache_sandwich()
{
	cache_sprite(SPR_SANDWICH); // Intact.
	cache_sprite(SPR_SANDWICH_WRAPER); // Wrapper.
}

void precache_plasma_detonator_explosion()
{
	cache_sprite(SPR_DETONATOR_EXP1);
	cache_sprite(SPR_DETONATOR_EXP2);
	cache_sprite(SPR_DETONATOR_EXP3);
	cache_sprite(SPR_DETONATOR_EXP4);
	cache_sprite(SPR_DETONATOR_EXP5);
	cache_sprite(SPR_DETONATOR_EXP6);
	cache_sprite(SPR_DETONATOR_EXP7);
	cache_sprite(SPR_DETONATOR_EXP8);
}

void precache_plasma_detonator()
{
	cache_sprite(SPR_DOORBOMB);
	cache_sprite(SPR_ALT_DOORBOMB);

	precache_plasma_detonator_explosion();
}

void precache_static(
	const statobj_t& bs_static)
{
	const auto sprite_number = bs_static.shapenum;

	if (false)
	{
	}
	else if (sprite_number == SPR_STAT_40 || sprite_number == SPR_STAT_41)
	{
		precache_water_bowl();
	}
	else if (sprite_number == SPR_STAT_42 || sprite_number == SPR_STAT_43)
	{
		precache_chicken_leg();
	}
	else if (sprite_number == SPR_STAT_44 || sprite_number == SPR_STAT_45)
	{
		precache_ham();
	}
	else if (sprite_number == SPR_CANDY_BAR || sprite_number == SPR_CANDY_WRAPER)
	{
		precache_candy_bar();
	}
	else if (sprite_number == SPR_SANDWICH || sprite_number == SPR_SANDWICH_WRAPER)
	{
		precache_sandwich();
	}
	else if (sprite_number == SPR_DOORBOMB || sprite_number == SPR_ALT_DOORBOMB)
	{
		precache_plasma_detonator();
	}
	else
	{
		cache_sprite(sprite_number);
	}
}

void precache_statics()
{
	for (auto bs_static = statobjlist.data(); laststatobj && bs_static != laststatobj; ++bs_static)
	{
		if (bs_static->shapenum == -1 ||
			(bs_static->tilex == 0 && bs_static->tiley == 0))
		{
			continue;
		}

		precache_static(*bs_static);
	}
}

void map_actor(
	const objtype& bs_actor)
{
	const auto bs_actor_index = get_actor_index(bs_actor);

	auto vertex_index = actors_base_vertex_index;
	vertex_index += (bs_actor_index * vertices_per_sprite);

	auto& sprite = actors_[bs_actor_index];

	map_sprite(SpriteKind::actor, vertex_index, sprite);

	sprite.x = bs_actor.x;
	sprite.y = bs_actor.y;
	sprite.tile_x = bs_actor.tilex;
	sprite.tile_y = bs_actor.tiley;
	sprite.bs_sprite_id = get_bs_actor_sprite_id(bs_actor);

	sprite.bs_object.actor = &bs_actor;
}

// Explosion.
void precache_explosion()
{
	cache_sprite(SPR_EXPLOSION_1);
	cache_sprite(SPR_EXPLOSION_2);
	cache_sprite(SPR_EXPLOSION_3);
	cache_sprite(SPR_EXPLOSION_4);
	cache_sprite(SPR_EXPLOSION_5);
}

// Clip Explosion.
void precache_clip_explosion()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		cache_sprite(SPR_CLIP_EXP1);
		cache_sprite(SPR_CLIP_EXP2);
		cache_sprite(SPR_CLIP_EXP3);
		cache_sprite(SPR_CLIP_EXP4);
		cache_sprite(SPR_CLIP_EXP5);
		cache_sprite(SPR_CLIP_EXP6);
		cache_sprite(SPR_CLIP_EXP7);
		cache_sprite(SPR_CLIP_EXP8);
	}
}

// Grenade explosion.
void precache_grenade_explosion()
{
	cache_sprite(SPR_GRENADE_EXPLODE1);
	cache_sprite(SPR_GRENADE_EXPLODE2);
	cache_sprite(SPR_GRENADE_EXPLODE3);
	cache_sprite(SPR_GRENADE_EXPLODE4);
	cache_sprite(SPR_GRENADE_EXPLODE5);
}

// Flying grenade.
void precache_flying_grenade()
{
	cache_sprite(SPR_GRENADE_FLY1);
	cache_sprite(SPR_GRENADE_FLY2);
	cache_sprite(SPR_GRENADE_FLY3);
	cache_sprite(SPR_GRENADE_FLY4);

	precache_grenade_explosion();
}

void precache_anti_plasma_cannon_explosion()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		cache_sprite(SPR_BFG_EXP1);
		cache_sprite(SPR_BFG_EXP2);
		cache_sprite(SPR_BFG_EXP3);
		cache_sprite(SPR_BFG_EXP4);
		cache_sprite(SPR_BFG_EXP5);
		cache_sprite(SPR_BFG_EXP6);
		cache_sprite(SPR_BFG_EXP7);
		cache_sprite(SPR_BFG_EXP8);
	}
}

void precache_anti_plasma_cannon_shot()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		cache_sprite(SPR_BFG_WEAPON_SHOT1);
		cache_sprite(SPR_BFG_WEAPON_SHOT2);
		cache_sprite(SPR_BFG_WEAPON_SHOT3);


		precache_anti_plasma_cannon_explosion();
	}
}

// A rubble.
void precache_rubble()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		cache_sprite(SPR_RUBBLE);
	}
}

// Toxic waste (green #1).
void precache_toxic_waste_green_1()
{
	cache_sprite(SPR_GREEN_OOZE1);
	cache_sprite(SPR_GREEN_OOZE2);
	cache_sprite(SPR_GREEN_OOZE3);
}

// Toxic waste (green #2).
void precache_toxic_waste_green_2()
{
	cache_sprite(SPR_GREEN2_OOZE1);
	cache_sprite(SPR_GREEN2_OOZE2);
	cache_sprite(SPR_GREEN2_OOZE3);
}

// Toxic waste (black #1).
void precache_toxic_waste_black_1()
{
	cache_sprite(SPR_BLACK_OOZE1);
	cache_sprite(SPR_BLACK_OOZE2);
	cache_sprite(SPR_BLACK_OOZE3);
}

// Toxic waste (black #2).
void precache_toxic_waste_black_2()
{
	cache_sprite(SPR_BLACK2_OOZE1);
	cache_sprite(SPR_BLACK2_OOZE2);
	cache_sprite(SPR_BLACK2_OOZE3);
}

// Coin (1).
void precache_coin_1()
{
	cache_sprite(SPR_STAT_77);
}

// Red Access Card.
void precache_red_access_card()
{
	cache_sprite(SPR_STAT_32);
}

// Yellow Access Card.
void precache_yellow_access_card()
{
	cache_sprite(SPR_STAT_33);
}

// Green Access Card (AOG).
void precache_green_access_card()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		cache_sprite(SPR_STAT_34);
	}
}

// Blue Access Card.
void precache_blue_access_card()
{
	cache_sprite(SPR_STAT_35);
}

// Golden Access Card (AOG).
void precache_golden_access_card()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		cache_sprite(SPR_STAT_36);
	}
}

// Small yellow box (PS).
void precache_small_yellow_box()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		cache_sprite(SPR_STAT_36);
	}
}

// Partial Charge Pack.
void precache_partial_charge_pack()
{
	cache_sprite(SPR_STAT_26);
}

// Charge Pack.
void precache_charge_pack()
{
	cache_sprite(SPR_STAT_31);
}

// Slow Fire Protector.
void precache_slow_fire_protector()
{
	cache_sprite(SPR_STAT_24);
}

// Rapid Assault Weapon.
void precache_rapid_assault_weapon()
{
	cache_sprite(SPR_STAT_27);
}

// Dual Neutron Disruptor.
void precache_dual_neutron_disruptor_weapon()
{
	cache_sprite(SPR_STAT_28);
}

// Plasma Discharge Unit.
void precache_plasma_discharge_unit_weapon()
{
	cache_sprite(SPR_STAT_46);
}

// Anti-Plasma Cannon.
void precache_anti_plasma_cannon_weapon()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		cache_sprite(SPR_STAT_34);
	}
}

// Generic alien spit (#1).
void precache_generic_alien_spit_1()
{
	cache_sprite(SPR_SPIT1_1);
	cache_sprite(SPR_SPIT1_2);
	cache_sprite(SPR_SPIT1_3);

	cache_sprite(SPR_SPIT_EXP1_1);
	cache_sprite(SPR_SPIT_EXP1_2);
	cache_sprite(SPR_SPIT_EXP1_3);
}

// Generic alien spit (#2).
void precache_generic_alien_spit_2()
{
	cache_sprite(SPR_SPIT2_1);
	cache_sprite(SPR_SPIT2_2);
	cache_sprite(SPR_SPIT2_3);

	cache_sprite(SPR_SPIT_EXP2_1);
	cache_sprite(SPR_SPIT_EXP2_2);
	cache_sprite(SPR_SPIT_EXP2_3);
}

// Generic alien spit (#3).
void precache_generic_alien_spit_3()
{
	cache_sprite(SPR_SPIT3_1);
	cache_sprite(SPR_SPIT3_2);
	cache_sprite(SPR_SPIT3_3);

	cache_sprite(SPR_SPIT_EXP3_1);
	cache_sprite(SPR_SPIT_EXP3_2);
	cache_sprite(SPR_SPIT_EXP3_3);
}

// Electrical Shot.
void precache_electrical_shot()
{
	cache_sprite(SPR_ELEC_SHOT1);
	cache_sprite(SPR_ELEC_SHOT2);
	cache_sprite(SPR_ELEC_SHOT_EXP1);
	cache_sprite(SPR_ELEC_SHOT_EXP2);
}

// Sector Patrol (AOG) / Sector Guard (PS).
void precache_sector_patrol_or_sector_guard()
{
	cache_sprite(SPR_RENT_S_1);
	cache_sprite(SPR_RENT_S_2);
	cache_sprite(SPR_RENT_S_3);
	cache_sprite(SPR_RENT_S_4);
	cache_sprite(SPR_RENT_S_5);
	cache_sprite(SPR_RENT_S_6);
	cache_sprite(SPR_RENT_S_7);
	cache_sprite(SPR_RENT_S_8);

	cache_sprite(SPR_RENT_W1_1);
	cache_sprite(SPR_RENT_W1_2);
	cache_sprite(SPR_RENT_W1_3);
	cache_sprite(SPR_RENT_W1_4);
	cache_sprite(SPR_RENT_W1_5);
	cache_sprite(SPR_RENT_W1_6);
	cache_sprite(SPR_RENT_W1_7);
	cache_sprite(SPR_RENT_W1_8);

	cache_sprite(SPR_RENT_W2_1);
	cache_sprite(SPR_RENT_W2_2);
	cache_sprite(SPR_RENT_W2_3);
	cache_sprite(SPR_RENT_W2_4);
	cache_sprite(SPR_RENT_W2_5);
	cache_sprite(SPR_RENT_W2_6);
	cache_sprite(SPR_RENT_W2_7);
	cache_sprite(SPR_RENT_W2_8);

	cache_sprite(SPR_RENT_W3_1);
	cache_sprite(SPR_RENT_W3_2);
	cache_sprite(SPR_RENT_W3_3);
	cache_sprite(SPR_RENT_W3_4);
	cache_sprite(SPR_RENT_W3_5);
	cache_sprite(SPR_RENT_W3_6);
	cache_sprite(SPR_RENT_W3_7);
	cache_sprite(SPR_RENT_W3_8);

	cache_sprite(SPR_RENT_W4_1);
	cache_sprite(SPR_RENT_W4_2);
	cache_sprite(SPR_RENT_W4_3);
	cache_sprite(SPR_RENT_W4_4);
	cache_sprite(SPR_RENT_W4_5);
	cache_sprite(SPR_RENT_W4_6);
	cache_sprite(SPR_RENT_W4_7);
	cache_sprite(SPR_RENT_W4_8);

	cache_sprite(SPR_RENT_DIE_1);
	cache_sprite(SPR_RENT_DIE_2);
	cache_sprite(SPR_RENT_DIE_3);
	cache_sprite(SPR_RENT_DIE_4);
	cache_sprite(SPR_RENT_PAIN_1);
	cache_sprite(SPR_RENT_DEAD);

	cache_sprite(SPR_RENT_SHOOT1);
	cache_sprite(SPR_RENT_SHOOT2);
	cache_sprite(SPR_RENT_SHOOT3);


	// Goodies.
	//
	precache_slow_fire_protector();
	precache_partial_charge_pack();
	precache_coin_1();
}

// Robot Turret.
void precache_robot_turret()
{
	cache_sprite(SPR_TERROT_1);
	cache_sprite(SPR_TERROT_2);
	cache_sprite(SPR_TERROT_3);
	cache_sprite(SPR_TERROT_4);
	cache_sprite(SPR_TERROT_5);
	cache_sprite(SPR_TERROT_6);
	cache_sprite(SPR_TERROT_7);
	cache_sprite(SPR_TERROT_8);

	cache_sprite(SPR_TERROT_FIRE_1);
	cache_sprite(SPR_TERROT_FIRE_2);
	cache_sprite(SPR_TERROT_DIE_1);
	cache_sprite(SPR_TERROT_DIE_2);
	cache_sprite(SPR_TERROT_DIE_3);
	cache_sprite(SPR_TERROT_DIE_4);
	cache_sprite(SPR_TERROT_DEAD);
}

// Bio-Technician.
void precache_bio_technician()
{
	cache_sprite(SPR_OFC_S_1);
	cache_sprite(SPR_OFC_S_2);
	cache_sprite(SPR_OFC_S_3);
	cache_sprite(SPR_OFC_S_4);
	cache_sprite(SPR_OFC_S_5);
	cache_sprite(SPR_OFC_S_6);
	cache_sprite(SPR_OFC_S_7);
	cache_sprite(SPR_OFC_S_8);

	cache_sprite(SPR_OFC_W1_1);
	cache_sprite(SPR_OFC_W1_2);
	cache_sprite(SPR_OFC_W1_3);
	cache_sprite(SPR_OFC_W1_4);
	cache_sprite(SPR_OFC_W1_5);
	cache_sprite(SPR_OFC_W1_6);
	cache_sprite(SPR_OFC_W1_7);
	cache_sprite(SPR_OFC_W1_8);

	cache_sprite(SPR_OFC_W2_1);
	cache_sprite(SPR_OFC_W2_2);
	cache_sprite(SPR_OFC_W2_3);
	cache_sprite(SPR_OFC_W2_4);
	cache_sprite(SPR_OFC_W2_5);
	cache_sprite(SPR_OFC_W2_6);
	cache_sprite(SPR_OFC_W2_7);
	cache_sprite(SPR_OFC_W2_8);

	cache_sprite(SPR_OFC_W3_1);
	cache_sprite(SPR_OFC_W3_2);
	cache_sprite(SPR_OFC_W3_3);
	cache_sprite(SPR_OFC_W3_4);
	cache_sprite(SPR_OFC_W3_5);
	cache_sprite(SPR_OFC_W3_6);
	cache_sprite(SPR_OFC_W3_7);
	cache_sprite(SPR_OFC_W3_8);

	cache_sprite(SPR_OFC_W4_1);
	cache_sprite(SPR_OFC_W4_2);
	cache_sprite(SPR_OFC_W4_3);
	cache_sprite(SPR_OFC_W4_4);
	cache_sprite(SPR_OFC_W4_5);
	cache_sprite(SPR_OFC_W4_6);
	cache_sprite(SPR_OFC_W4_7);
	cache_sprite(SPR_OFC_W4_8);

	cache_sprite(SPR_OFC_PAIN_1);
	cache_sprite(SPR_OFC_DIE_1);
	cache_sprite(SPR_OFC_DIE_2);
	cache_sprite(SPR_OFC_DIE_3);
	cache_sprite(SPR_OFC_PAIN_2);
	cache_sprite(SPR_OFC_DIE_4);
	cache_sprite(SPR_OFC_DEAD);

	cache_sprite(SPR_OFC_SHOOT1);
	cache_sprite(SPR_OFC_SHOOT2);
	cache_sprite(SPR_OFC_SHOOT3);


	// Goodies.
	//
	precache_partial_charge_pack();
	precache_coin_1();
}

// Pod Alien.
void precache_pod_alien()
{
	cache_sprite(SPR_POD_WALK1);
	cache_sprite(SPR_POD_WALK2);
	cache_sprite(SPR_POD_WALK3);
	cache_sprite(SPR_POD_WALK4);
	cache_sprite(SPR_POD_ATTACK1);
	cache_sprite(SPR_POD_ATTACK2);
	cache_sprite(SPR_POD_ATTACK3);
	cache_sprite(SPR_POD_OUCH);
	cache_sprite(SPR_POD_DIE1);
	cache_sprite(SPR_POD_DIE2);
	cache_sprite(SPR_POD_DIE3);
	cache_sprite(SPR_POD_SPIT1);
	cache_sprite(SPR_POD_SPIT2);
	cache_sprite(SPR_POD_SPIT3);


	precache_generic_alien_spit_3();
}

// Pod Alien Egg.
void precache_pod_alien_egg()
{
	cache_sprite(SPR_POD_EGG);
	cache_sprite(SPR_POD_HATCH1);
	cache_sprite(SPR_POD_HATCH2);
	cache_sprite(SPR_POD_HATCH3);


	precache_pod_alien();
}

// High Energy Plasma Alien.
void precache_high_energy_plasma_alien()
{
	cache_sprite(SPR_ELEC_APPEAR1);
	cache_sprite(SPR_ELEC_APPEAR2);
	cache_sprite(SPR_ELEC_APPEAR3);
	cache_sprite(SPR_ELEC_WALK1);
	cache_sprite(SPR_ELEC_WALK2);
	cache_sprite(SPR_ELEC_WALK3);
	cache_sprite(SPR_ELEC_WALK4);
	cache_sprite(SPR_ELEC_OUCH);
	cache_sprite(SPR_ELEC_SHOOT1);
	cache_sprite(SPR_ELEC_SHOOT2);
	cache_sprite(SPR_ELEC_SHOOT3);
	cache_sprite(SPR_ELEC_DIE1);
	cache_sprite(SPR_ELEC_DIE2);
	cache_sprite(SPR_ELEC_DIE3);


	precache_electrical_shot();
}

// Plasma Sphere.
void precache_plasma_sphere()
{
	cache_sprite(SPR_ELECTRO_SPHERE_ROAM1);
	cache_sprite(SPR_ELECTRO_SPHERE_ROAM2);
	cache_sprite(SPR_ELECTRO_SPHERE_ROAM3);
	cache_sprite(SPR_ELECTRO_SPHERE_OUCH);
	cache_sprite(SPR_ELECTRO_SPHERE_DIE1);
	cache_sprite(SPR_ELECTRO_SPHERE_DIE2);
	cache_sprite(SPR_ELECTRO_SPHERE_DIE3);
	cache_sprite(SPR_ELECTRO_SPHERE_DIE4);
}

// Star Sentinel (AOG) / Tech Warrior (PS).
void precache_star_sentinel_or_tech_warrior()
{
	cache_sprite(SPR_PRO_S_1);
	cache_sprite(SPR_PRO_S_2);
	cache_sprite(SPR_PRO_S_3);
	cache_sprite(SPR_PRO_S_4);
	cache_sprite(SPR_PRO_S_5);
	cache_sprite(SPR_PRO_S_6);
	cache_sprite(SPR_PRO_S_7);
	cache_sprite(SPR_PRO_S_8);

	cache_sprite(SPR_PRO_W1_1);
	cache_sprite(SPR_PRO_W1_2);
	cache_sprite(SPR_PRO_W1_3);
	cache_sprite(SPR_PRO_W1_4);
	cache_sprite(SPR_PRO_W1_5);
	cache_sprite(SPR_PRO_W1_6);
	cache_sprite(SPR_PRO_W1_7);
	cache_sprite(SPR_PRO_W1_8);

	cache_sprite(SPR_PRO_W2_1);
	cache_sprite(SPR_PRO_W2_2);
	cache_sprite(SPR_PRO_W2_3);
	cache_sprite(SPR_PRO_W2_4);
	cache_sprite(SPR_PRO_W2_5);
	cache_sprite(SPR_PRO_W2_6);
	cache_sprite(SPR_PRO_W2_7);
	cache_sprite(SPR_PRO_W2_8);

	cache_sprite(SPR_PRO_W3_1);
	cache_sprite(SPR_PRO_W3_2);
	cache_sprite(SPR_PRO_W3_3);
	cache_sprite(SPR_PRO_W3_4);
	cache_sprite(SPR_PRO_W3_5);
	cache_sprite(SPR_PRO_W3_6);
	cache_sprite(SPR_PRO_W3_7);
	cache_sprite(SPR_PRO_W3_8);

	cache_sprite(SPR_PRO_W4_1);
	cache_sprite(SPR_PRO_W4_2);
	cache_sprite(SPR_PRO_W4_3);
	cache_sprite(SPR_PRO_W4_4);
	cache_sprite(SPR_PRO_W4_5);
	cache_sprite(SPR_PRO_W4_6);
	cache_sprite(SPR_PRO_W4_7);
	cache_sprite(SPR_PRO_W4_8);

	cache_sprite(SPR_PRO_PAIN_1);
	cache_sprite(SPR_PRO_DIE_1);
	cache_sprite(SPR_PRO_DIE_2);
	cache_sprite(SPR_PRO_DIE_3);
	cache_sprite(SPR_PRO_PAIN_2);
	cache_sprite(SPR_PRO_DIE_4);
	cache_sprite(SPR_PRO_DEAD);

	cache_sprite(SPR_PRO_SHOOT1);
	cache_sprite(SPR_PRO_SHOOT2);
	cache_sprite(SPR_PRO_SHOOT3);


	// Goodies.
	//
	precache_rapid_assault_weapon();
	precache_partial_charge_pack();
	precache_coin_1();
}

// High-Security Genetic Guard.
void precache_high_security_genetic_guard()
{
	cache_sprite(SPR_GENETIC_W1);
	cache_sprite(SPR_GENETIC_W2);
	cache_sprite(SPR_GENETIC_W3);
	cache_sprite(SPR_GENETIC_W4);
	cache_sprite(SPR_GENETIC_SWING1);
	cache_sprite(SPR_GENETIC_SWING2);
	cache_sprite(SPR_GENETIC_SWING3);
	cache_sprite(SPR_GENETIC_DEAD);
	cache_sprite(SPR_GENETIC_DIE1);
	cache_sprite(SPR_GENETIC_DIE2);
	cache_sprite(SPR_GENETIC_DIE3);
	cache_sprite(SPR_GENETIC_DIE4);
	cache_sprite(SPR_GENETIC_OUCH);
	cache_sprite(SPR_GENETIC_SHOOT1);
	cache_sprite(SPR_GENETIC_SHOOT2);
	cache_sprite(SPR_GENETIC_SHOOT3);


	// Goodies.
	//
	precache_slow_fire_protector();
	precache_partial_charge_pack();
}

// Experimental Mech-Sentinel.
void precache_experimental_mech_sentinel()
{
	cache_sprite(SPR_MUTHUM1_W1);
	cache_sprite(SPR_MUTHUM1_W2);
	cache_sprite(SPR_MUTHUM1_W3);
	cache_sprite(SPR_MUTHUM1_W4);
	cache_sprite(SPR_MUTHUM1_SWING1);
	cache_sprite(SPR_MUTHUM1_SWING2);
	cache_sprite(SPR_MUTHUM1_SWING3);
	cache_sprite(SPR_MUTHUM1_DEAD);
	cache_sprite(SPR_MUTHUM1_DIE1);
	cache_sprite(SPR_MUTHUM1_DIE2);
	cache_sprite(SPR_MUTHUM1_DIE3);
	cache_sprite(SPR_MUTHUM1_DIE4);
	cache_sprite(SPR_MUTHUM1_OUCH);
	cache_sprite(SPR_MUTHUM1_SPIT1);
	cache_sprite(SPR_MUTHUM1_SPIT2);
	cache_sprite(SPR_MUTHUM1_SPIT3);

	precache_electrical_shot();


	// Goodies.
	//
	precache_partial_charge_pack();
}

// Experimental Mutant Human.
void precache_experimental_mutant_human()
{
	cache_sprite(SPR_MUTHUM2_W1);
	cache_sprite(SPR_MUTHUM2_W2);
	cache_sprite(SPR_MUTHUM2_W3);
	cache_sprite(SPR_MUTHUM2_W4);
	cache_sprite(SPR_MUTHUM2_SWING1);
	cache_sprite(SPR_MUTHUM2_SWING2);
	cache_sprite(SPR_MUTHUM2_SWING3);
	cache_sprite(SPR_MUTHUM2_DEAD);
	cache_sprite(SPR_MUTHUM2_DIE1);
	cache_sprite(SPR_MUTHUM2_DIE2);
	cache_sprite(SPR_MUTHUM2_DIE3);
	cache_sprite(SPR_MUTHUM2_DIE4);
	cache_sprite(SPR_MUTHUM2_OUCH);
	cache_sprite(SPR_MUTHUM2_SPIT1);
	cache_sprite(SPR_MUTHUM2_SPIT2);
	cache_sprite(SPR_MUTHUM2_SPIT3);


	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		precache_electrical_shot();
	}
	else
	{
		precache_generic_alien_spit_1();
	}
}

// Morphing Experimental Mutant Human.
void precache_experimental_mutant_human_morphing()
{
	cache_sprite(SPR_MUTHUM2_MORPH1);
	cache_sprite(SPR_MUTHUM2_MORPH2);
	cache_sprite(SPR_MUTHUM2_MORPH3);
	cache_sprite(SPR_MUTHUM2_MORPH4);
	cache_sprite(SPR_MUTHUM2_MORPH5);
	cache_sprite(SPR_MUTHUM2_MORPH6);
	cache_sprite(SPR_MUTHUM2_MORPH7);
	cache_sprite(SPR_MUTHUM2_MORPH8);
	cache_sprite(SPR_MUTHUM2_MORPH9);


	precache_experimental_mutant_human();
}

// Large Experimental Genetic Alien.
void precache_large_experimental_genetic_alien()
{
	cache_sprite(SPR_LCAN_ALIEN_W1);
	cache_sprite(SPR_LCAN_ALIEN_W2);
	cache_sprite(SPR_LCAN_ALIEN_W3);
	cache_sprite(SPR_LCAN_ALIEN_W4);
	cache_sprite(SPR_LCAN_ALIEN_SWING1);
	cache_sprite(SPR_LCAN_ALIEN_SWING2);
	cache_sprite(SPR_LCAN_ALIEN_SWING3);
	cache_sprite(SPR_LCAN_ALIEN_DEAD);
	cache_sprite(SPR_LCAN_ALIEN_DIE1);
	cache_sprite(SPR_LCAN_ALIEN_DIE2);
	cache_sprite(SPR_LCAN_ALIEN_DIE3);
	cache_sprite(SPR_LCAN_ALIEN_DIE4);
	cache_sprite(SPR_LCAN_ALIEN_OUCH);
	cache_sprite(SPR_LCAN_ALIEN_SPIT1);
	cache_sprite(SPR_LCAN_ALIEN_SPIT2);
	cache_sprite(SPR_LCAN_ALIEN_SPIT3);


	precache_generic_alien_spit_3();
}

// A canister with large Experimental Genetic Alien.
void precache_canister_with_large_experimental_genetic_alien()
{
	cache_sprite(SPR_LCAN_ALIEN_READY);
	cache_sprite(SPR_LCAN_ALIEN_B1);
	cache_sprite(SPR_LCAN_ALIEN_B2);
	cache_sprite(SPR_LCAN_ALIEN_B3);
	cache_sprite(SPR_LCAN_ALIEN_EMPTY);

	precache_large_experimental_genetic_alien();
}

// Small Experimental Genetic Alien.
void precache_experimental_genetic_alien_small()
{
	cache_sprite(SPR_SCAN_ALIEN_W1);
	cache_sprite(SPR_SCAN_ALIEN_W2);
	cache_sprite(SPR_SCAN_ALIEN_W3);
	cache_sprite(SPR_SCAN_ALIEN_W4);
	cache_sprite(SPR_SCAN_ALIEN_SWING1);
	cache_sprite(SPR_SCAN_ALIEN_SWING2);
	cache_sprite(SPR_SCAN_ALIEN_SWING3);
	cache_sprite(SPR_SCAN_ALIEN_DEAD);
	cache_sprite(SPR_SCAN_ALIEN_DIE1);
	cache_sprite(SPR_SCAN_ALIEN_DIE2);
	cache_sprite(SPR_SCAN_ALIEN_DIE3);
	cache_sprite(SPR_SCAN_ALIEN_DIE4);
	cache_sprite(SPR_SCAN_ALIEN_OUCH);
	cache_sprite(SPR_SCAN_ALIEN_SPIT1);
	cache_sprite(SPR_SCAN_ALIEN_SPIT2);
	cache_sprite(SPR_SCAN_ALIEN_SPIT3);


	precache_generic_alien_spit_1();
}

// A canister with small Experimental Genetic Alien.
void precache_canister_with_small_experimental_genetic_alien()
{
	cache_sprite(SPR_SCAN_ALIEN_READY);
	cache_sprite(SPR_SCAN_ALIEN_B1);
	cache_sprite(SPR_SCAN_ALIEN_B2);
	cache_sprite(SPR_SCAN_ALIEN_B3);
	cache_sprite(SPR_SCAN_ALIEN_EMPTY);


	precache_experimental_genetic_alien_small();
}

// Mutated Guard.
void precache_mutated_guard()
{
	cache_sprite(SPR_GURNEY_MUT_W1);
	cache_sprite(SPR_GURNEY_MUT_W2);
	cache_sprite(SPR_GURNEY_MUT_W3);
	cache_sprite(SPR_GURNEY_MUT_W4);
	cache_sprite(SPR_GURNEY_MUT_SWING1);
	cache_sprite(SPR_GURNEY_MUT_SWING2);
	cache_sprite(SPR_GURNEY_MUT_SWING3);
	cache_sprite(SPR_GURNEY_MUT_DEAD);
	cache_sprite(SPR_GURNEY_MUT_DIE1);
	cache_sprite(SPR_GURNEY_MUT_DIE2);
	cache_sprite(SPR_GURNEY_MUT_DIE3);
	cache_sprite(SPR_GURNEY_MUT_DIE4);
	cache_sprite(SPR_GURNEY_MUT_OUCH);


	// Goodies.
	//
	precache_slow_fire_protector();
	precache_partial_charge_pack();
}

// Mutated Guard (waiting).
void precache_mutated_guard_waiting()
{
	cache_sprite(SPR_GURNEY_MUT_READY);
	cache_sprite(SPR_GURNEY_MUT_B1);
	cache_sprite(SPR_GURNEY_MUT_B2);
	cache_sprite(SPR_GURNEY_MUT_B3);
	cache_sprite(SPR_GURNEY_MUT_EMPTY);


	precache_mutated_guard();
}

// Fluid Alien Shot.
void precache_fluid_alien_shot()
{
	cache_sprite(SPR_LIQUID_SHOT_FLY_1);
	cache_sprite(SPR_LIQUID_SHOT_FLY_2);
	cache_sprite(SPR_LIQUID_SHOT_FLY_3);
	cache_sprite(SPR_LIQUID_SHOT_BURST_1);
	cache_sprite(SPR_LIQUID_SHOT_BURST_2);
	cache_sprite(SPR_LIQUID_SHOT_BURST_3);
}

// Fluid Alien.
void precache_fluid_alien()
{
	cache_sprite(SPR_LIQUID_M1);
	cache_sprite(SPR_LIQUID_M2);
	cache_sprite(SPR_LIQUID_M3);
	cache_sprite(SPR_LIQUID_R1);
	cache_sprite(SPR_LIQUID_R2);
	cache_sprite(SPR_LIQUID_R3);
	cache_sprite(SPR_LIQUID_R4);
	cache_sprite(SPR_LIQUID_S1);
	cache_sprite(SPR_LIQUID_S2);
	cache_sprite(SPR_LIQUID_S3);
	cache_sprite(SPR_LIQUID_OUCH);
	cache_sprite(SPR_LIQUID_DIE_1);
	cache_sprite(SPR_LIQUID_DIE_2);
	cache_sprite(SPR_LIQUID_DIE_3);
	cache_sprite(SPR_LIQUID_DIE_4);
	cache_sprite(SPR_LIQUID_DEAD);


	precache_fluid_alien_shot();
}

// Star Trooper (AOG) / Alien Protector (PS).
void precache_star_trooper_or_alien_protector()
{
	cache_sprite(SPR_SWAT_S_1);
	cache_sprite(SPR_SWAT_S_2);
	cache_sprite(SPR_SWAT_S_3);
	cache_sprite(SPR_SWAT_S_4);
	cache_sprite(SPR_SWAT_S_5);
	cache_sprite(SPR_SWAT_S_6);
	cache_sprite(SPR_SWAT_S_7);
	cache_sprite(SPR_SWAT_S_8);

	cache_sprite(SPR_SWAT_W1_1);
	cache_sprite(SPR_SWAT_W1_2);
	cache_sprite(SPR_SWAT_W1_3);
	cache_sprite(SPR_SWAT_W1_4);
	cache_sprite(SPR_SWAT_W1_5);
	cache_sprite(SPR_SWAT_W1_6);
	cache_sprite(SPR_SWAT_W1_7);
	cache_sprite(SPR_SWAT_W1_8);

	cache_sprite(SPR_SWAT_W2_1);
	cache_sprite(SPR_SWAT_W2_2);
	cache_sprite(SPR_SWAT_W2_3);
	cache_sprite(SPR_SWAT_W2_4);
	cache_sprite(SPR_SWAT_W2_5);
	cache_sprite(SPR_SWAT_W2_6);
	cache_sprite(SPR_SWAT_W2_7);
	cache_sprite(SPR_SWAT_W2_8);

	cache_sprite(SPR_SWAT_W3_1);
	cache_sprite(SPR_SWAT_W3_2);
	cache_sprite(SPR_SWAT_W3_3);
	cache_sprite(SPR_SWAT_W3_4);
	cache_sprite(SPR_SWAT_W3_5);
	cache_sprite(SPR_SWAT_W3_6);
	cache_sprite(SPR_SWAT_W3_7);
	cache_sprite(SPR_SWAT_W3_8);

	cache_sprite(SPR_SWAT_W4_1);
	cache_sprite(SPR_SWAT_W4_2);
	cache_sprite(SPR_SWAT_W4_3);
	cache_sprite(SPR_SWAT_W4_4);
	cache_sprite(SPR_SWAT_W4_5);
	cache_sprite(SPR_SWAT_W4_6);
	cache_sprite(SPR_SWAT_W4_7);
	cache_sprite(SPR_SWAT_W4_8);

	cache_sprite(SPR_SWAT_PAIN_1);
	cache_sprite(SPR_SWAT_DIE_1);
	cache_sprite(SPR_SWAT_DIE_2);
	cache_sprite(SPR_SWAT_DIE_3);
	cache_sprite(SPR_SWAT_PAIN_2);
	cache_sprite(SPR_SWAT_DIE_4);
	cache_sprite(SPR_SWAT_DEAD);

	cache_sprite(SPR_SWAT_SHOOT1);
	cache_sprite(SPR_SWAT_SHOOT2);
	cache_sprite(SPR_SWAT_SHOOT3);

	cache_sprite(SPR_SWAT_WOUNDED1);
	cache_sprite(SPR_SWAT_WOUNDED2);
	cache_sprite(SPR_SWAT_WOUNDED3);
	cache_sprite(SPR_SWAT_WOUNDED4);


	// Goodies.
	//
	precache_rapid_assault_weapon();
	precache_partial_charge_pack();
	precache_coin_1();
}

// Dr. Goldfire.
void precache_dr_goldfire()
{
	cache_sprite(SPR_GOLD_S_1);
	cache_sprite(SPR_GOLD_S_2);
	cache_sprite(SPR_GOLD_S_3);
	cache_sprite(SPR_GOLD_S_4);
	cache_sprite(SPR_GOLD_S_5);
	cache_sprite(SPR_GOLD_S_6);
	cache_sprite(SPR_GOLD_S_7);
	cache_sprite(SPR_GOLD_S_8);

	cache_sprite(SPR_GOLD_W1_1);
	cache_sprite(SPR_GOLD_W1_2);
	cache_sprite(SPR_GOLD_W1_3);
	cache_sprite(SPR_GOLD_W1_4);
	cache_sprite(SPR_GOLD_W1_5);
	cache_sprite(SPR_GOLD_W1_6);
	cache_sprite(SPR_GOLD_W1_7);
	cache_sprite(SPR_GOLD_W1_8);

	cache_sprite(SPR_GOLD_W2_1);
	cache_sprite(SPR_GOLD_W2_2);
	cache_sprite(SPR_GOLD_W2_3);
	cache_sprite(SPR_GOLD_W2_4);
	cache_sprite(SPR_GOLD_W2_5);
	cache_sprite(SPR_GOLD_W2_6);
	cache_sprite(SPR_GOLD_W2_7);
	cache_sprite(SPR_GOLD_W2_8);

	cache_sprite(SPR_GOLD_W3_1);
	cache_sprite(SPR_GOLD_W3_2);
	cache_sprite(SPR_GOLD_W3_3);
	cache_sprite(SPR_GOLD_W3_4);
	cache_sprite(SPR_GOLD_W3_5);
	cache_sprite(SPR_GOLD_W3_6);
	cache_sprite(SPR_GOLD_W3_7);
	cache_sprite(SPR_GOLD_W3_8);

	cache_sprite(SPR_GOLD_W4_1);
	cache_sprite(SPR_GOLD_W4_2);
	cache_sprite(SPR_GOLD_W4_3);
	cache_sprite(SPR_GOLD_W4_4);
	cache_sprite(SPR_GOLD_W4_5);
	cache_sprite(SPR_GOLD_W4_6);
	cache_sprite(SPR_GOLD_W4_7);
	cache_sprite(SPR_GOLD_W4_8);

	cache_sprite(SPR_GOLD_PAIN_1);

	cache_sprite(SPR_GOLD_WRIST_1);
	cache_sprite(SPR_GOLD_WRIST_2);

	cache_sprite(SPR_GOLD_SHOOT1);
	cache_sprite(SPR_GOLD_SHOOT2);
	cache_sprite(SPR_GOLD_SHOOT3);

	cache_sprite(SPR_GOLD_WARP1);
	cache_sprite(SPR_GOLD_WARP2);
	cache_sprite(SPR_GOLD_WARP3);
	cache_sprite(SPR_GOLD_WARP4);
	cache_sprite(SPR_GOLD_WARP5);


	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		cache_sprite(SPR_GOLD_DEATH1);
		cache_sprite(SPR_GOLD_DEATH2);
		cache_sprite(SPR_GOLD_DEATH3);
		cache_sprite(SPR_GOLD_DEATH4);
		cache_sprite(SPR_GOLD_DEATH5);
	}


	// Goodies.
	//
	precache_golden_access_card();
}

// Morphed Dr. Goldfire Shot.
void precache_morphed_dr_goldfire_shot()
{
	cache_sprite(SPR_MGOLD_SHOT1);
	cache_sprite(SPR_MGOLD_SHOT2);
	cache_sprite(SPR_MGOLD_SHOT3);
	cache_sprite(SPR_MGOLD_SHOT_EXP1);
	cache_sprite(SPR_MGOLD_SHOT_EXP2);
	cache_sprite(SPR_MGOLD_SHOT_EXP3);
}

// Morphed Dr. Goldfire.
void precache_morphed_dr_goldfire()
{
	cache_sprite(SPR_MGOLD_OUCH);

	cache_sprite(SPR_GOLD_MORPH1);
	cache_sprite(SPR_GOLD_MORPH2);
	cache_sprite(SPR_GOLD_MORPH3);
	cache_sprite(SPR_GOLD_MORPH4);
	cache_sprite(SPR_GOLD_MORPH5);
	cache_sprite(SPR_GOLD_MORPH6);
	cache_sprite(SPR_GOLD_MORPH7);
	cache_sprite(SPR_GOLD_MORPH8);

	cache_sprite(SPR_MGOLD_WALK1);
	cache_sprite(SPR_MGOLD_WALK2);
	cache_sprite(SPR_MGOLD_WALK3);
	cache_sprite(SPR_MGOLD_WALK4);
	cache_sprite(SPR_MGOLD_ATTACK1);
	cache_sprite(SPR_MGOLD_ATTACK2);
	cache_sprite(SPR_MGOLD_ATTACK3);
	cache_sprite(SPR_MGOLD_ATTACK4);


	precache_morphed_dr_goldfire_shot();


	// Goodies.
	//
	precache_golden_access_card();
}

// Volatile Material Transport.
void precache_volatile_material_transport()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog_sw())
	{
		return;
	}

	cache_sprite(SPR_GSCOUT_W1_1);
	cache_sprite(SPR_GSCOUT_W1_2);
	cache_sprite(SPR_GSCOUT_W1_3);
	cache_sprite(SPR_GSCOUT_W1_4);
	cache_sprite(SPR_GSCOUT_W1_5);
	cache_sprite(SPR_GSCOUT_W1_6);
	cache_sprite(SPR_GSCOUT_W1_7);
	cache_sprite(SPR_GSCOUT_W1_8);

	cache_sprite(SPR_GSCOUT_W2_1);
	cache_sprite(SPR_GSCOUT_W2_2);
	cache_sprite(SPR_GSCOUT_W2_3);
	cache_sprite(SPR_GSCOUT_W2_4);
	cache_sprite(SPR_GSCOUT_W2_5);
	cache_sprite(SPR_GSCOUT_W2_6);
	cache_sprite(SPR_GSCOUT_W2_7);
	cache_sprite(SPR_GSCOUT_W2_8);

	cache_sprite(SPR_GSCOUT_W3_1);
	cache_sprite(SPR_GSCOUT_W3_2);
	cache_sprite(SPR_GSCOUT_W3_3);
	cache_sprite(SPR_GSCOUT_W3_4);
	cache_sprite(SPR_GSCOUT_W3_5);
	cache_sprite(SPR_GSCOUT_W3_6);
	cache_sprite(SPR_GSCOUT_W3_7);
	cache_sprite(SPR_GSCOUT_W3_8);

	cache_sprite(SPR_GSCOUT_W4_1);
	cache_sprite(SPR_GSCOUT_W4_2);
	cache_sprite(SPR_GSCOUT_W4_3);
	cache_sprite(SPR_GSCOUT_W4_4);
	cache_sprite(SPR_GSCOUT_W4_5);
	cache_sprite(SPR_GSCOUT_W4_6);
	cache_sprite(SPR_GSCOUT_W4_7);
	cache_sprite(SPR_GSCOUT_W4_8);

	cache_sprite(SPR_GSCOUT_DIE1);
	cache_sprite(SPR_GSCOUT_DIE2);
	cache_sprite(SPR_GSCOUT_DIE3);
	cache_sprite(SPR_GSCOUT_DIE4);
	cache_sprite(SPR_GSCOUT_DIE5);
	cache_sprite(SPR_GSCOUT_DIE6);
	cache_sprite(SPR_GSCOUT_DIE7);
	cache_sprite(SPR_GSCOUT_DIE8);

	cache_sprite(SPR_GSCOUT_DEAD);


	//
	precache_explosion();
	precache_toxic_waste_green_1();
}

void precache_perscan_drone()
{
	cache_sprite(SPR_FSCOUT_W1_1);
	cache_sprite(SPR_FSCOUT_W1_2);
	cache_sprite(SPR_FSCOUT_W1_3);
	cache_sprite(SPR_FSCOUT_W1_4);
	cache_sprite(SPR_FSCOUT_W1_5);
	cache_sprite(SPR_FSCOUT_W1_6);
	cache_sprite(SPR_FSCOUT_W1_7);
	cache_sprite(SPR_FSCOUT_W1_8);

	const auto& assets_info = get_assets_info();

	if (!assets_info.is_aog_sw())
	{
		cache_sprite(SPR_FSCOUT_W2_1);
		cache_sprite(SPR_FSCOUT_W2_2);
		cache_sprite(SPR_FSCOUT_W2_3);
		cache_sprite(SPR_FSCOUT_W2_4);
		cache_sprite(SPR_FSCOUT_W2_5);
		cache_sprite(SPR_FSCOUT_W2_6);
		cache_sprite(SPR_FSCOUT_W2_7);
		cache_sprite(SPR_FSCOUT_W2_8);

		cache_sprite(SPR_FSCOUT_W3_1);
		cache_sprite(SPR_FSCOUT_W3_2);
		cache_sprite(SPR_FSCOUT_W3_3);
		cache_sprite(SPR_FSCOUT_W3_4);
		cache_sprite(SPR_FSCOUT_W3_5);
		cache_sprite(SPR_FSCOUT_W3_6);
		cache_sprite(SPR_FSCOUT_W3_7);
		cache_sprite(SPR_FSCOUT_W3_8);

		cache_sprite(SPR_FSCOUT_W4_1);
		cache_sprite(SPR_FSCOUT_W4_2);
		cache_sprite(SPR_FSCOUT_W4_3);
		cache_sprite(SPR_FSCOUT_W4_4);
		cache_sprite(SPR_FSCOUT_W4_5);
		cache_sprite(SPR_FSCOUT_W4_6);
		cache_sprite(SPR_FSCOUT_W4_7);
		cache_sprite(SPR_FSCOUT_W4_8);
	}

	cache_sprite(SPR_FSCOUT_DIE1);
	cache_sprite(SPR_FSCOUT_DIE2);
	cache_sprite(SPR_FSCOUT_DIE3);
	cache_sprite(SPR_FSCOUT_DIE4);
	cache_sprite(SPR_FSCOUT_DIE5);
	cache_sprite(SPR_FSCOUT_DIE6);
	cache_sprite(SPR_FSCOUT_DIE7);
	cache_sprite(SPR_FSCOUT_DEAD);

	//
	precache_explosion();
}

// Security Cube Explosion.
void precache_security_cube_explosion()
{
	cache_sprite(SPR_CUBE_EXP1);
	cache_sprite(SPR_CUBE_EXP2);
	cache_sprite(SPR_CUBE_EXP3);
	cache_sprite(SPR_CUBE_EXP4);
	cache_sprite(SPR_CUBE_EXP5);
	cache_sprite(SPR_CUBE_EXP6);
	cache_sprite(SPR_CUBE_EXP7);
	cache_sprite(SPR_CUBE_EXP8);
}

// Security Cube.
void precache_security_cube_or_projection_generator()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		cache_sprite(SPR_VITAL_STAND);
		cache_sprite(SPR_VITAL_DIE_1);
		cache_sprite(SPR_VITAL_DIE_2);
		cache_sprite(SPR_VITAL_DIE_3);
		cache_sprite(SPR_VITAL_DIE_4);
		cache_sprite(SPR_VITAL_DIE_5);
		cache_sprite(SPR_VITAL_DIE_6);
		cache_sprite(SPR_VITAL_DIE_7);
		cache_sprite(SPR_VITAL_DIE_8);
		cache_sprite(SPR_VITAL_DEAD_1);
		cache_sprite(SPR_VITAL_DEAD_2);
		cache_sprite(SPR_VITAL_DEAD_3);
		cache_sprite(SPR_VITAL_OUCH);


		precache_explosion();
	}
	else
	{
		cache_sprite(SPR_CUBE1);
		cache_sprite(SPR_CUBE2);
		cache_sprite(SPR_CUBE3);
		cache_sprite(SPR_CUBE4);
		cache_sprite(SPR_CUBE5);
		cache_sprite(SPR_CUBE6);
		cache_sprite(SPR_CUBE7);
		cache_sprite(SPR_CUBE8);
		cache_sprite(SPR_CUBE9);
		cache_sprite(SPR_CUBE10);
		cache_sprite(SPR_DEAD_CUBE);


		precache_security_cube_explosion();
	}
}

// Spider Mutant Shot.
void precache_spider_mutant_shot()
{
	cache_sprite(SPR_BOSS1_PROJ1);
	cache_sprite(SPR_BOSS1_PROJ2);
	cache_sprite(SPR_BOSS1_PROJ3);
	cache_sprite(SPR_BOSS1_EXP1);
	cache_sprite(SPR_BOSS1_EXP2);
	cache_sprite(SPR_BOSS1_EXP3);
}

// Spider Mutant.
void precache_spider_mutant()
{
	cache_sprite(SPR_BOSS1_W1);
	cache_sprite(SPR_BOSS1_W2);
	cache_sprite(SPR_BOSS1_W3);
	cache_sprite(SPR_BOSS1_W4);
	cache_sprite(SPR_BOSS1_SWING1);
	cache_sprite(SPR_BOSS1_SWING2);
	cache_sprite(SPR_BOSS1_SWING3);
	cache_sprite(SPR_BOSS1_DEAD);
	cache_sprite(SPR_BOSS1_DIE1);
	cache_sprite(SPR_BOSS1_DIE2);
	cache_sprite(SPR_BOSS1_DIE3);
	cache_sprite(SPR_BOSS1_DIE4);
	cache_sprite(SPR_BOSS1_OUCH);
	cache_sprite(SPR_BOSS1_PROJ1);
	cache_sprite(SPR_BOSS1_PROJ2);
	cache_sprite(SPR_BOSS1_PROJ3);
	cache_sprite(SPR_BOSS1_EXP1);
	cache_sprite(SPR_BOSS1_EXP2);
	cache_sprite(SPR_BOSS1_EXP3);


	precache_spider_mutant_shot();
}

// Morphing Spider Mutant.
void precache_spider_mutant_morphing()
{
	cache_sprite(SPR_BOSS1_MORPH1);
	cache_sprite(SPR_BOSS1_MORPH2);
	cache_sprite(SPR_BOSS1_MORPH3);
	cache_sprite(SPR_BOSS1_MORPH4);
	cache_sprite(SPR_BOSS1_MORPH5);
	cache_sprite(SPR_BOSS1_MORPH6);
	cache_sprite(SPR_BOSS1_MORPH7);
	cache_sprite(SPR_BOSS1_MORPH8);
	cache_sprite(SPR_BOSS1_MORPH9);


	precache_spider_mutant();
}

// Breather Beast.
void precache_breather_beast()
{
	cache_sprite(SPR_BOSS2_W1);
	cache_sprite(SPR_BOSS2_W2);
	cache_sprite(SPR_BOSS2_W3);
	cache_sprite(SPR_BOSS2_W4);
	cache_sprite(SPR_BOSS2_SWING1);
	cache_sprite(SPR_BOSS2_SWING2);
	cache_sprite(SPR_BOSS2_SWING3);
	cache_sprite(SPR_BOSS2_DEAD);
	cache_sprite(SPR_BOSS2_DIE1);
	cache_sprite(SPR_BOSS2_DIE2);
	cache_sprite(SPR_BOSS2_DIE3);
	cache_sprite(SPR_BOSS2_DIE4);
	cache_sprite(SPR_BOSS2_OUCH);
}

// Cyborg Warrior.
void precache_cyborg_warrior()
{
	cache_sprite(SPR_BOSS3_W1);
	cache_sprite(SPR_BOSS3_W2);
	cache_sprite(SPR_BOSS3_W3);
	cache_sprite(SPR_BOSS3_W4);
	cache_sprite(SPR_BOSS3_SWING1);
	cache_sprite(SPR_BOSS3_SWING2);
	cache_sprite(SPR_BOSS3_SWING3);
	cache_sprite(SPR_BOSS3_DEAD);
	cache_sprite(SPR_BOSS3_DIE1);
	cache_sprite(SPR_BOSS3_DIE2);
	cache_sprite(SPR_BOSS3_DIE3);
	cache_sprite(SPR_BOSS3_DIE4);
	cache_sprite(SPR_BOSS3_OUCH);
}

// Reptilian Warrior.
void precache_reptilian_warrior()
{
	cache_sprite(SPR_BOSS4_W1);
	cache_sprite(SPR_BOSS4_W2);
	cache_sprite(SPR_BOSS4_W3);
	cache_sprite(SPR_BOSS4_W4);
	cache_sprite(SPR_BOSS4_SWING1);
	cache_sprite(SPR_BOSS4_SWING2);
	cache_sprite(SPR_BOSS4_SWING3);
	cache_sprite(SPR_BOSS4_DEAD);
	cache_sprite(SPR_BOSS4_DIE1);
	cache_sprite(SPR_BOSS4_DIE2);
	cache_sprite(SPR_BOSS4_DIE3);
	cache_sprite(SPR_BOSS4_DIE4);
	cache_sprite(SPR_BOSS4_OUCH);
}

// Reptilian Warrior (morphing).
void precache_reptilian_warrior_morphing()
{
	cache_sprite(SPR_BOSS4_MORPH1);
	cache_sprite(SPR_BOSS4_MORPH2);
	cache_sprite(SPR_BOSS4_MORPH3);
	cache_sprite(SPR_BOSS4_MORPH4);
	cache_sprite(SPR_BOSS4_MORPH5);
	cache_sprite(SPR_BOSS4_MORPH6);
	cache_sprite(SPR_BOSS4_MORPH7);
	cache_sprite(SPR_BOSS4_MORPH8);
	cache_sprite(SPR_BOSS4_MORPH9);


	precache_reptilian_warrior();
}

// Acid Dragon Shot.
void precache_acid_dragon_shot()
{
	cache_sprite(SPR_BOSS5_PROJ1);
	cache_sprite(SPR_BOSS5_PROJ2);
	cache_sprite(SPR_BOSS5_PROJ3);
	cache_sprite(SPR_BOSS5_EXP1);
	cache_sprite(SPR_BOSS5_EXP2);
	cache_sprite(SPR_BOSS5_EXP3);
}

// Acid Dragon.
void precache_acid_dragon()
{
	cache_sprite(SPR_BOSS5_W1);
	cache_sprite(SPR_BOSS5_W2);
	cache_sprite(SPR_BOSS5_W3);
	cache_sprite(SPR_BOSS5_W4);
	cache_sprite(SPR_BOSS5_SWING1);
	cache_sprite(SPR_BOSS5_SWING2);
	cache_sprite(SPR_BOSS5_SWING3);
	cache_sprite(SPR_BOSS5_DEAD);
	cache_sprite(SPR_BOSS5_DIE1);
	cache_sprite(SPR_BOSS5_DIE2);
	cache_sprite(SPR_BOSS5_DIE3);
	cache_sprite(SPR_BOSS5_DIE4);
	cache_sprite(SPR_BOSS5_OUCH);


	precache_acid_dragon_shot();
}

// Bio-Mech Guardian.
void precache_bio_mech_guardian()
{
	cache_sprite(SPR_BOSS6_W1);
	cache_sprite(SPR_BOSS6_W2);
	cache_sprite(SPR_BOSS6_W3);
	cache_sprite(SPR_BOSS6_W4);
	cache_sprite(SPR_BOSS6_SWING1);
	cache_sprite(SPR_BOSS6_SWING2);
	cache_sprite(SPR_BOSS6_SWING3);
	cache_sprite(SPR_BOSS6_DEAD);
	cache_sprite(SPR_BOSS6_DIE1);
	cache_sprite(SPR_BOSS6_DIE2);
	cache_sprite(SPR_BOSS6_DIE3);
	cache_sprite(SPR_BOSS6_DIE4);
	cache_sprite(SPR_BOSS6_OUCH);
}

// The Giant Stalker.
void precache_the_giant_stalker()
{
	cache_sprite(SPR_BOSS7_W1);
	cache_sprite(SPR_BOSS7_W2);
	cache_sprite(SPR_BOSS7_W3);
	cache_sprite(SPR_BOSS7_W4);
	cache_sprite(SPR_BOSS7_SHOOT1);
	cache_sprite(SPR_BOSS7_SHOOT2);
	cache_sprite(SPR_BOSS7_SHOOT3);
	cache_sprite(SPR_BOSS7_DEAD);
	cache_sprite(SPR_BOSS7_DIE1);
	cache_sprite(SPR_BOSS7_DIE2);
	cache_sprite(SPR_BOSS7_DIE3);
	cache_sprite(SPR_BOSS7_DIE4);
	cache_sprite(SPR_BOSS7_OUCH);
}

// The Spector Demon.
void precache_the_spector_demon()
{
	cache_sprite(SPR_BOSS8_W1);
	cache_sprite(SPR_BOSS8_W2);
	cache_sprite(SPR_BOSS8_W3);
	cache_sprite(SPR_BOSS8_W4);
	cache_sprite(SPR_BOSS8_SHOOT1);
	cache_sprite(SPR_BOSS8_SHOOT2);
	cache_sprite(SPR_BOSS8_SHOOT3);
	cache_sprite(SPR_BOSS8_DIE1);
	cache_sprite(SPR_BOSS8_DIE2);
	cache_sprite(SPR_BOSS8_DIE3);
	cache_sprite(SPR_BOSS8_DIE4);
	cache_sprite(SPR_BOSS8_DEAD);
	cache_sprite(SPR_BOSS8_OUCH);


	//
	precache_morphed_dr_goldfire_shot();
}

// The Armored Stalker.
void precache_the_armored_stalker()
{
	cache_sprite(SPR_BOSS9_W1);
	cache_sprite(SPR_BOSS9_W2);
	cache_sprite(SPR_BOSS9_W3);
	cache_sprite(SPR_BOSS9_W4);
	cache_sprite(SPR_BOSS9_SHOOT1);
	cache_sprite(SPR_BOSS9_SHOOT2);
	cache_sprite(SPR_BOSS9_SHOOT3);
	cache_sprite(SPR_BOSS9_DIE1);
	cache_sprite(SPR_BOSS9_DIE2);
	cache_sprite(SPR_BOSS9_DIE3);
	cache_sprite(SPR_BOSS9_DIE4);
	cache_sprite(SPR_BOSS9_DEAD);
	cache_sprite(SPR_BOSS9_OUCH);
}

// The Crawler Beast Shot.
void precache_the_crawler_beast_shot()
{
	cache_sprite(SPR_BOSS10_SPIT1);
	cache_sprite(SPR_BOSS10_SPIT2);
	cache_sprite(SPR_BOSS10_SPIT3);

	cache_sprite(SPR_BOSS10_SPIT_EXP1);
	cache_sprite(SPR_BOSS10_SPIT_EXP2);
	cache_sprite(SPR_BOSS10_SPIT_EXP3);
}

// The Crawler Beast.
void precache_the_crawler_beast()
{
	cache_sprite(SPR_BOSS10_W1);
	cache_sprite(SPR_BOSS10_W2);
	cache_sprite(SPR_BOSS10_W3);
	cache_sprite(SPR_BOSS10_W4);
	cache_sprite(SPR_BOSS10_SHOOT1);
	cache_sprite(SPR_BOSS10_SHOOT2);
	cache_sprite(SPR_BOSS10_SHOOT3);
	cache_sprite(SPR_BOSS10_DEAD);
	cache_sprite(SPR_BOSS10_DIE1);
	cache_sprite(SPR_BOSS10_DIE2);
	cache_sprite(SPR_BOSS10_DIE3);
	cache_sprite(SPR_BOSS10_DIE4);
	cache_sprite(SPR_BOSS10_OUCH);


	precache_the_crawler_beast_shot();
}

// Blake Stone.
void precache_blake_stone()
{
	cache_sprite(SPR_BLAKE_W1);
	cache_sprite(SPR_BLAKE_W2);
	cache_sprite(SPR_BLAKE_W3);
	cache_sprite(SPR_BLAKE_W4);
}

void precache_vent_and_dripping_blood()
{
	cache_sprite(SPR_BLOOD_DRIP1);
	cache_sprite(SPR_BLOOD_DRIP2);
	cache_sprite(SPR_BLOOD_DRIP3);
	cache_sprite(SPR_BLOOD_DRIP4);
}

void precache_vent_and_dripping_water()
{
	cache_sprite(SPR_WATER_DRIP1);
	cache_sprite(SPR_WATER_DRIP2);
	cache_sprite(SPR_WATER_DRIP3);
	cache_sprite(SPR_WATER_DRIP4);
}

void precache_flicker_light()
{
	cache_sprite(SPR_DECO_ARC_1);
	cache_sprite(SPR_DECO_ARC_2);
	cache_sprite(SPR_DECO_ARC_3);
}

void precache_crate_content()
{
	precache_chicken_leg();
	precache_ham();

	precache_charge_pack();

	precache_slow_fire_protector();
	precache_rapid_assault_weapon();
	precache_dual_neutron_disruptor_weapon();
	precache_plasma_discharge_unit_weapon();
	precache_anti_plasma_cannon_weapon();

	precache_small_yellow_box();

	cache_sprite(SPR_STAT_48); // money bag
	cache_sprite(SPR_STAT_49); // loot
	cache_sprite(SPR_STAT_50); // gold
	cache_sprite(SPR_STAT_51); // bonus
	cache_sprite(SPR_STAT_57); // Body Parts
}

void precache_crate_1()
{
	cache_sprite(SPR_CRATE_1);


	// Goodies.
	//
	precache_grenade_explosion();
	precache_crate_content();
}

void precache_crate_2()
{
	cache_sprite(SPR_CRATE_2);


	// Goodies.
	//
	precache_grenade_explosion();
	precache_crate_content();
}

void precache_crate_3()
{
	cache_sprite(SPR_CRATE_3);


	// Goodies.
	//
	precache_grenade_explosion();
	precache_crate_content();
}

void precache_electrical_post_barrier()
{
	cache_sprite(SPR_ELEC_POST1);
	cache_sprite(SPR_ELEC_POST2);
	cache_sprite(SPR_ELEC_POST3);
	cache_sprite(SPR_ELEC_POST4);
}

void precache_electrical_arc_barrier()
{
	cache_sprite(SPR_ELEC_ARC1);
	cache_sprite(SPR_ELEC_ARC2);
	cache_sprite(SPR_ELEC_ARC3);
	cache_sprite(SPR_ELEC_ARC4);
}

void precache_vertical_post_barrier()
{
	cache_sprite(SPR_VPOST1);
	cache_sprite(SPR_VPOST2);
	cache_sprite(SPR_VPOST3);
	cache_sprite(SPR_VPOST4);
	cache_sprite(SPR_VPOST5);
	cache_sprite(SPR_VPOST6);
	cache_sprite(SPR_VPOST7);
	cache_sprite(SPR_VPOST8);
}

void precache_vertical_spike_barrier()
{
	cache_sprite(SPR_VSPIKE1);
	cache_sprite(SPR_VSPIKE2);
	cache_sprite(SPR_VSPIKE3);
	cache_sprite(SPR_VSPIKE4);
	cache_sprite(SPR_VSPIKE5);
	cache_sprite(SPR_VSPIKE6);
	cache_sprite(SPR_VSPIKE7);
	cache_sprite(SPR_VSPIKE8);
}

void precache_security_light()
{
	cache_sprite(SPR_SECURITY_NORMAL);
	cache_sprite(SPR_SECURITY_ALERT);
}

void precache_grate_and_steam()
{
	cache_sprite(SPR_GRATE);
	cache_sprite(SPR_STEAM_1);
	cache_sprite(SPR_STEAM_2);
	cache_sprite(SPR_STEAM_3);
	cache_sprite(SPR_STEAM_4);
}

void precache_pipe_and_steam()
{
	cache_sprite(SPR_STEAM_PIPE);
	cache_sprite(SPR_PIPE_STEAM_1);
	cache_sprite(SPR_PIPE_STEAM_2);
	cache_sprite(SPR_PIPE_STEAM_3);
	cache_sprite(SPR_PIPE_STEAM_4);
}

void precache_special_stuff()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		if (gamestate.mapon == 9)
		{
			precache_blake_stone();
			precache_dr_goldfire();
		}
	}
	else if (assets_info.is_ps())
	{
		if (gamestate.mapon == 19)
		{
			precache_morphed_dr_goldfire();
		}
	}

	if (GoldsternInfo.SpawnCnt > 0)
	{
		precache_dr_goldfire();
	}

	if (NumEAWalls > 0)
	{
		precache_high_energy_plasma_alien();
	}
}

void precache_access_cards()
{
	precache_red_access_card();
	precache_yellow_access_card();
	precache_green_access_card();
	precache_blue_access_card();
	precache_golden_access_card();
}

void precache_player_weapon_auto_charge_pistol()
{
	cache_sprite(SPR_KNIFEREADY);
	cache_sprite(SPR_KNIFEATK1);
	cache_sprite(SPR_KNIFEATK2);
	cache_sprite(SPR_KNIFEATK3);
	cache_sprite(SPR_KNIFEATK4);
}

void precache_player_weapon_slow_fire_protector()
{
	cache_sprite(SPR_PISTOLREADY);
	cache_sprite(SPR_PISTOLATK1);
	cache_sprite(SPR_PISTOLATK2);
	cache_sprite(SPR_PISTOLATK3);
	cache_sprite(SPR_PISTOLATK4);
}

void precache_player_weapon_rapid_assault_weapon()
{
	cache_sprite(SPR_MACHINEGUNREADY);
	cache_sprite(SPR_MACHINEGUNATK1);
	cache_sprite(SPR_MACHINEGUNATK2);
	cache_sprite(SPR_MACHINEGUNATK3);
	cache_sprite(SPR_MACHINEGUNATK4);
}

void precache_player_weapon_dual_neutron_disruptor()
{
	cache_sprite(SPR_CHAINREADY);
	cache_sprite(SPR_CHAINATK1);
	cache_sprite(SPR_CHAINATK2);
	cache_sprite(SPR_CHAINATK3);
	cache_sprite(SPR_CHAINATK4);
}

void precache_player_weapon_plasma_discharge_unit()
{
	cache_sprite(SPR_GRENADEREADY);
	cache_sprite(SPR_GRENADEATK1);
	cache_sprite(SPR_GRENADEATK2);
	cache_sprite(SPR_GRENADEATK3);
	cache_sprite(SPR_GRENADEATK4);

	precache_flying_grenade();
	precache_grenade_explosion();
	precache_explosion();
}

void precache_player_weapon_anti_plasma_cannon()
{
	const auto& assets_info = get_assets_info();

	if (!assets_info.is_ps())
	{
		return;
	}

	cache_sprite(SPR_BFG_WEAPON1);
	cache_sprite(SPR_BFG_WEAPON1);
	cache_sprite(SPR_BFG_WEAPON2);
	cache_sprite(SPR_BFG_WEAPON3);
	cache_sprite(SPR_BFG_WEAPON4);
	cache_sprite(SPR_BFG_WEAPON5);

	precache_anti_plasma_cannon_shot();
	precache_anti_plasma_cannon_explosion();

	precache_rubble();

	precache_explosion();
	precache_clip_explosion();
}

void precache_player_weapons()
{
	precache_player_weapon_auto_charge_pistol();
	precache_player_weapon_slow_fire_protector();
	precache_player_weapon_rapid_assault_weapon();
	precache_player_weapon_dual_neutron_disruptor();
	precache_player_weapon_plasma_discharge_unit();
	precache_player_weapon_anti_plasma_cannon();
}

void precache_dead(
	const objtype& bs_actor)
{
	const auto sprite_id = get_bs_actor_sprite_id(bs_actor);

	cache_sprite(sprite_id);
}

void precache_actors()
{
	if (!player)
	{
		return;
	}

	for (auto bs_actor = player->next; bs_actor; bs_actor = bs_actor->next)
	{
		switch (bs_actor->obclass)
		{
		case nothing:
			break;

		case rentacopobj:
			precache_sector_patrol_or_sector_guard();
			break;

		case hang_terrotobj:
			precache_robot_turret();
			break;

		case gen_scientistobj:
			precache_bio_technician();
			break;

		case podobj:
			precache_pod_alien();
			break;

		case electroobj:
			precache_high_energy_plasma_alien();
			break;

		case electrosphereobj:
			precache_plasma_sphere();
			break;

		case proguardobj:
			precache_star_sentinel_or_tech_warrior();
			break;

		case genetic_guardobj:
			precache_high_security_genetic_guard();
			break;

		case mutant_human1obj:
			precache_experimental_mech_sentinel();
			break;

		case mutant_human2obj:
			precache_experimental_mutant_human();
			break;

		case lcan_wait_alienobj:
			precache_canister_with_large_experimental_genetic_alien();
			break;

		case lcan_alienobj:
			precache_large_experimental_genetic_alien();
			break;

		case scan_wait_alienobj:
			precache_canister_with_small_experimental_genetic_alien();
			break;

		case scan_alienobj:
			precache_experimental_genetic_alien_small();
			break;

		case gurney_waitobj:
			precache_mutated_guard_waiting();
			break;

		case gurneyobj:
			precache_mutated_guard();
			break;

		case liquidobj:
			precache_fluid_alien();
			break;

		case swatobj:
			precache_star_trooper_or_alien_protector();
			break;

		case goldsternobj:
			precache_dr_goldfire();
			break;

		case gold_morphobj:
			precache_morphed_dr_goldfire();
			break;

		case volatiletransportobj:
			precache_volatile_material_transport();
			break;

		case floatingbombobj:
			precache_perscan_drone();
			break;

		case rotating_cubeobj:
			precache_security_cube_or_projection_generator();
			break;

		case spider_mutantobj:
			precache_spider_mutant();
			break;

		case breather_beastobj:
			precache_breather_beast();
			break;

		case cyborg_warriorobj:
			precache_cyborg_warrior();
			break;

		case reptilian_warriorobj:
			precache_reptilian_warrior();
			break;

		case acid_dragonobj:
			precache_acid_dragon();
			break;

		case mech_guardianobj:
			precache_bio_mech_guardian();
			break;

		case final_boss1obj:
			precache_the_giant_stalker();
			break;

		case final_boss2obj:
			precache_the_spector_demon();
			break;

		case final_boss3obj:
			precache_the_armored_stalker();
			break;

		case final_boss4obj:
			precache_the_crawler_beast();
			break;

		case blakeobj:
			precache_blake_stone();
			break;

		case crate1obj:
			precache_crate_1();
			break;

		case crate2obj:
			precache_crate_2();
			break;

		case crate3obj:
			precache_crate_3();
			break;

		case green_oozeobj:
			precache_toxic_waste_green_1();
			break;

		case black_oozeobj:
			precache_toxic_waste_black_1();
			break;

		case green2_oozeobj:
			precache_toxic_waste_green_2();
			break;

		case black2_oozeobj:
			precache_toxic_waste_black_2();
			break;

		case podeggobj:
			precache_pod_alien_egg();
			break;

		case morphing_spider_mutantobj:
			precache_spider_mutant_morphing();
			break;

		case morphing_reptilian_warriorobj:
			precache_reptilian_warrior_morphing();
			break;

		case morphing_mutanthuman2obj:
			precache_experimental_mutant_human_morphing();
			break;

		case electroshotobj:
			precache_electrical_shot();
			break;

		case post_barrierobj:
			precache_electrical_post_barrier();
			break;

		case arc_barrierobj:
			precache_electrical_arc_barrier();
			break;

		case vpost_barrierobj:
			precache_vertical_post_barrier();
			break;

		case vspike_barrierobj:
			precache_vertical_spike_barrier();
			break;

		case goldmorphshotobj:
			precache_morphed_dr_goldfire_shot();
			break;

		case security_lightobj:
			precache_security_light();
			break;

		case explosionobj:
			precache_explosion();
			precache_clip_explosion();
			break;

		case steamgrateobj:
			precache_grate_and_steam();
			break;

		case steampipeobj:
			precache_pipe_and_steam();
			break;

		case liquidshotobj:
			precache_fluid_alien_shot();
			break;

		case lcanshotobj:
			precache_generic_alien_spit_3();
			break;

		case podshotobj:
			precache_generic_alien_spit_3();
			break;

		case scanshotobj:
			precache_generic_alien_spit_1();
			break;

		case dogshotobj:
			precache_generic_alien_spit_1();
			break;

		case mut_hum1shotobj:
			precache_electrical_shot();
			break;

		case ventdripobj:
			precache_vent_and_dripping_blood();
			precache_vent_and_dripping_water();
			break;

		case playerspshotobj:
			break;

		case flickerlightobj:
			precache_flicker_light();
			break;

		case plasma_detonatorobj:
		case plasma_detonator_reserveobj:
			precache_plasma_detonator();
			break;

		case grenadeobj:
			precache_flying_grenade();
			break;

		case bfg_shotobj:
			precache_anti_plasma_cannon_shot();
			break;

		case bfg_explosionobj:
			precache_anti_plasma_cannon_explosion();
			break;

		case pd_explosionobj:
			precache_plasma_detonator_explosion();
			break;

		case spider_mutantshotobj:
			precache_spider_mutant_shot();
			break;

		case breather_beastshotobj:
			break;

		case cyborg_warriorshotobj:
			break;

		case reptilian_warriorshotobj:
			break;

		case acid_dragonshotobj:
			precache_acid_dragon_shot();
			break;

		case mech_guardianshotobj:
			break;

		case final_boss2shotobj:
			precache_morphed_dr_goldfire_shot();
			break;

		case final_boss4shotobj:
			precache_the_crawler_beast_shot();
			break;

		case doorexplodeobj:
			precache_explosion();
			precache_rubble();
			break;

		case gr_explosionobj:
			precache_explosion();
			precache_grenade_explosion();
			break;

		case gold_morphingobj:
			precache_morphed_dr_goldfire();
			break;

		case deadobj:
			precache_dead(*bs_actor);
			break;

		default:
			break;
		}
	}

	precache_special_stuff();
	precache_access_cards();
	precache_player_weapons();
}

void precache_sprites()
{
	precache_statics();
	precache_actors();
}

void build_statics()
{
	uninitialize_statics();

	initialize_statics();

	if (!laststatobj)
	{
		return;
	}

	for (auto bs_static = statobjlist.data(); bs_static != laststatobj; ++bs_static)
	{
		if (bs_static->shapenum == -1 ||
			(bs_static->tilex == 0 && bs_static->tiley == 0))
		{
			continue;
		}

		map_static(*bs_static);
	}
}

void build_actors()
{
	uninitialize_actors();
	initialize_actors();

	if (!player)
	{
		return;
	}

	for (auto bs_actor = player->next; bs_actor; bs_actor = bs_actor->next)
	{
		map_actor(*bs_actor);
	}
}

void build_sprites()
{
	uninitialize_sprites();
	initialize_sprites();

	build_statics();
	build_actors();
}

void precache_resources()
{
	log("");
	log("Precaching resources.");

	texture_mgr_->begin_cache();

	precache_flooring();
	precache_ceiling();
	precache_walls();
	precache_pushwalls();
	precache_doors();
	precache_sprites();

	texture_mgr_->end_cache();

	texture_mgr_->purge_cache();
}

void destroy_texture_upscale_resources()
{
	ui_t2d_ = nullptr;
	flooring_textured_t2d_ = nullptr;
	ceiling_textured_t2d_ = nullptr;
}

void create_texture_upscale_resources()
{
	ui_t2d_ = texture_mgr_->get_ui();

	if (FloorTile > 0)
	{
		flooring_textured_t2d_ = texture_mgr_->get_wall(FloorTile);
	}

	if (CeilingTile > 0)
	{
		ceiling_textured_t2d_ = texture_mgr_->get_wall(CeilingTile);
	}
}

void destroy_external_textures_resources()
{
	flooring_textured_t2d_ = nullptr;
	ceiling_textured_t2d_ = nullptr;
}

void create_external_textures_resources()
{
	if (FloorTile > 0)
	{
		flooring_textured_t2d_ = texture_mgr_->get_wall(FloorTile);
	}

	if (CeilingTile > 0)
	{
		ceiling_textured_t2d_ = texture_mgr_->get_wall(CeilingTile);
	}
}

void uninitialize_video()
{
	uninitialize_command_buffers();

	uninitialize_program();

	uninitialize_walls();
	uninitialize_pushwalls();
	uninitialize_door_sides();
	uninitialize_sprites();

	uninitialize_flooring();
	uninitialize_ceiling();

	uninitialize_3d_fade();

	uninitialize_player_weapon();
	uninitialize_2d();

	destroy_texture_manager();

	uninitialize_samplers();

	renderer_ = nullptr;
	renderer_mgr_ = nullptr;
}

void initialize_video()
{
	log("");
	log("Initializing hardware accelerated video system.");

	log("Initializing task manager.");

	set_samplers_default_states();
	set_player_weapon_sampler_default_state();

	log("Creating renderer manager.");

	renderer_mgr_ = bstone::Ren3dMgrFactory::create();

	vid_initialize_common();
	calculate_dimensions();

	vid_initialize_vanilla_raycaster();

	initialize_renderer();
	log_device_features();
	initialize_program();
	create_texture_manager();
	initialize_ui_buffer();
	initialize_palette();
	initialize_2d();
	initialize_flooring();
	initialize_ceiling();
	initialize_3d_fade();
	initialize_player_weapon();
	initialize_command_buffers();
	build_matrices();
	initialize_samplers();

	device_features_ = renderer_->get_device_features();

	apply_external_textures();
	apply_texture_upscale();

	const auto window_title = vid_get_window_title_for_renderer(renderer_->get_name());
	renderer_->set_window_title(window_title);
	renderer_->show_window(true);

	in_grab_mouse(true);
}
}; // HwVideo


// ==========================================================================


VideoUPtr make_hw_video()
{
	return std::make_unique<HwVideo>();
}


} // bstone
