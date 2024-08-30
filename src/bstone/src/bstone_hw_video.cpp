/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Hardware accelerated video (HW).

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "3d_def.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_vh.h"
#include "id_vl.h"

#include "bstone_algorithm.h"
#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_exception_utils.h"
#include "bstone_single_pool_resource.h"

#include "bstone_cgm_mat.h"
#include "bstone_cgm_clip_space.h"
#include "bstone_cgm_transform.h"
#include "bstone_cgm_vec.h"

#include "bstone_door.h"
#include "bstone_globals.h"
#include "bstone_hw_shader_registry.h"
#include "bstone_hw_video.h"
#include "bstone_logger.h"
#include "bstone_mod_value.h"
#include "bstone_video.h"

#include "bstone_r3r_cmd_buffer.h"
#include "bstone_r3r_limits.h"
#include "bstone_r3r_mgr.h"
#include "bstone_r3r_utils.h"

namespace bstone {

namespace {

class HwVideo final : public Video
{
public:
	HwVideo();
	~HwVideo() override;

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

	void fade_out(int start, int end, int red, int green, int blue, int step_count) override;
	void fade_in(int start, int end, const std::uint8_t* palette, int step_count) override;

	void apply_widescreen() override;
	void apply_window_mode() override;
	void apply_filler_color_index() override;

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

private:
	static constexpr auto log_prefix = "[VIDHW] ";

	VgaPalette vga_palette_{};

private:
	void door_get_page_numbers(const doorobj_t& door, int& front_face_page_number, int& back_face_page_number);

	static void validate_tile_xy(int tile_x, int tile_y);

	static void log(LoggerMessageType message_type, const std::string& message);
	static void log(const std::string& message);
	static void log_error(const std::string& message);

	enum class VertexAttribLocationId : unsigned char
	{
		none,
		position,
		color,
		texture_coordinates,
	};

	template<typename T>
	static constexpr T map_dimension = static_cast<T>(MAPSIZE);

	static constexpr auto map_dimension_f = map_dimension<float>;
	static constexpr auto map_dimension_d = map_dimension<double>;
	static constexpr auto map_height_f = 1.0F;

	template<typename T>
	static constexpr auto tile_dimension = static_cast<T>(1);

	static constexpr auto tile_dimension_f = tile_dimension<float>;

	template<typename T>
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
		typename T, VertexAttribLocationId TLocationId, typename = int>
	struct VertexHasAttribute : std::false_type
	{};

	template<typename T>
	struct VertexHasAttribute<
		T, VertexAttribLocationId::position, decltype(static_cast<void>(T::xyz), 0)> : std::true_type
	{};

	template<typename T>
	struct VertexHasAttribute<
		T, VertexAttribLocationId::color, decltype(static_cast<void>(T::rgba8), 0)> : std::true_type
	{};

	template<typename T>
	struct VertexHasAttribute<
		T, VertexAttribLocationId::texture_coordinates, decltype(static_cast<void>(T::uv), 0)> : std::true_type
	{};

	template<typename T, VertexAttribLocationId TLocationId, typename = int>
	struct VertexAttributeTraits
	{
		static constexpr auto is_valid = false;
		static constexpr auto offset = -1;
		static constexpr auto stride = -1;
	};

	template<typename T>
	struct VertexAttributeTraits<
		T, VertexAttribLocationId::position, decltype(static_cast<void>(T::xyz), 0)>
	{
		static constexpr auto is_valid = true;
		static constexpr auto offset = static_cast<int>(offsetof(T, xyz));
		static constexpr auto stride = static_cast<int>(sizeof(T));
	};

	template<typename T>
	struct VertexAttributeTraits<
		T, VertexAttribLocationId::color, decltype(static_cast<void>(T::rgba8), 0)>
	{
		static constexpr auto is_valid = true;
		static constexpr auto offset = static_cast<int>(offsetof(T, rgba8));
		static constexpr auto stride = static_cast<int>(sizeof(T));
	};

	template<typename T>
	struct VertexAttributeTraits<
		T, VertexAttribLocationId::texture_coordinates, decltype(static_cast<void>(T::uv), 0)>
	{
		static constexpr auto is_valid = true;
		static constexpr auto offset = static_cast<int>(offsetof(T, uv));
		static constexpr auto stride = static_cast<int>(sizeof(T));
	};

	using VertexPosition = cgm::Vec3F;
	using VertexColor = Rgba8;
	using VertexTextureCoordinates = cgm::Vec2F;

	struct VertexXyzUv
	{
		VertexPosition xyz;
		VertexTextureCoordinates uv;
	};

	struct VertexXyzRgbaUv
	{
		VertexPosition xyz;
		VertexColor rgba8;
		VertexTextureCoordinates uv;
	};

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
	};

	struct Wall;
	using WallCPtr = const Wall*;

	struct WallSideFlags
	{
		using Type = unsigned char;

		Type is_active : 1;
		Type is_vertical : 1;
		Type is_door_track : 1;
	};

	struct WallSide
	{
		WallCPtr wall;

		WallSideFlags flags;
		int vertex_index;
		int texture_id;
	};

	using WallSideCPtr = const WallSide*;

	struct Wall
	{
		static constexpr auto max_sides = 4;

		using Sides = std::array<WallSide, max_sides>;

		int x;
		int y;

		Sides sides;
	};

	using XyWallMap = std::unordered_map<int, Wall>;
	using WallsToRenderList = std::unordered_set<int>;

	enum XyWallType
	{
		solid,
		push,
	};

	struct WallSideDrawItem
	{
		int texture_id;
		WallSideCPtr wall_side;
	};

	struct Door;
	using DoorPtr = Door*;

	struct DoorSide
	{
		DoorPtr door;

		bool is_back_face;
		int texture_id;
	};

	using DoorSideCPtr = const DoorSide*;

	struct Door
	{
		using Sides = std::array<DoorSide, sides_per_door>;

		int bs_door_index;
		int vertex_index;
		Sides sides;
	};

	struct DoorDrawItem
	{
		int texture_id;
		DoorSideCPtr door_side;
	};

	using XyDoorMap = std::unordered_map<int, Door>;
	using DoorsToRenderList = std::unordered_set<int>;
	using DoorDrawItems = std::vector<DoorDrawItem>;

	using WallSideDrawItems = std::vector<WallSideDrawItem>;

	using WallSideIndexBuffer = std::vector<std::uint16_t>;
	using DoorIndexBuffer = std::vector<std::uint16_t>;

	enum class SpriteType
	{
		none,
		stat,
		actor,
	};

	struct SpriteFlags
	{
		using Value = unsigned char;

		Value is_visible : 1;
	};

	struct Sprite
	{
		union BsObject
		{
			const statobj_t* stat;
			const objtype* actor;
		};

		double x;
		double y;
		int tile_x;
		int tile_y;
		int bs_sprite_id;
		double square_distance;

		SpriteType type;
		SpriteFlags flags;
		int vertex_index;
		BsObject bs_object;
	};

	using SpriteCPtr = const Sprite*;

	struct SpriteDrawItem
	{
		int texture_id;
		SpriteCPtr sprite;
	};

	using Sprites = std::vector<Sprite>;

	using StaticsToRenderList = std::unordered_set<int>;
	using ActorsToRenderList = std::unordered_set<int>;
	using SpritesDrawList = std::vector<SpriteDrawItem>;

	using SpritesIndexBuffer = std::vector<std::uint16_t>;

	template<typename TVertex>
	using VertexBufferImageT = std::vector<TVertex>;

	using Vbi = VertexBufferImageT<Vertex>;
	using WallsVbi = VertexBufferImageT<WallVertex>;
	using PushwallsVbi = VertexBufferImageT<PushwallVertex>;
	using DoorsVbi = VertexBufferImageT<DoorVertex>;
	using PlayerWeaponVbi = VertexBufferImageT<PlayerWeaponVertex>;
	using FadeVbi = VertexBufferImageT<FadeVertex>;

	using SamplerVarMod = ModValue<int>;
	SamplerVarMod sampler_var_;

	using ShadingModeMod = ModValue<int>;
	ShadingModeMod shading_mode_;

	using BsShadeMaxMod = ModValue<int>;
	BsShadeMaxMod bs_shade_max_;

	using BsNormalShadeMod = ModValue<int>;
	BsNormalShadeMod bs_normal_shade_;

	using BsHeightNumeratorMod = ModValue<double>;
	BsHeightNumeratorMod bs_height_numerator_;

	using BsLightingMod = ModValue<int>;
	BsLightingMod bs_lighting_;

	using BsViewDirectionMod = ModValue<cgm::Vec2D>;
	BsViewDirectionMod bs_view_direction_;

	using BsViewPositionMod = ModValue<cgm::Vec2D>;
	BsViewPositionMod bs_view_position_;

	cgm::Mat4D r2_matrix_model_{};
	cgm::Mat4D r2_matrix_view_{};
	cgm::Mat4D r2_matrix_projection_{};


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
	double camera_vfov_rad_{};

	// Vertical FOV in degrees of the camera.
	double camera_vfov_deg_{};

	// Distance to the near plane of the camera.
	double camera_near_distance_{};

	// Distance to the far plane of the camera.
	double camera_far_distance_{};

	bool is_draw_3d_{};

	R2Vbi r2_vertices_{};

	R3rDeviceFeatures device_features_{};
	R3rMgrUPtr renderer_mgr_{};
	R3r* renderer_{};

	R3rSamplerState ui_sampler_state_{};
	R3rSamplerUPtr ui_sampler_{};

	R3rSamplerState wall_sampler_state_{};
	R3rSamplerUPtr wall_sampler_{};

	R3rSamplerState sprite_sampler_state_{};
	R3rSamplerUPtr sprite_sampler_{};

	R3rSamplerState player_weapon_sampler_state_{};
	R3rSamplerUPtr player_weapon_sampler_{};

	R3rSamplerUPtr fade_sampler_{};

	HwTextureMgrUPtr texture_mgr_{};

	Rgba8Palette palette_{};
	Rgba8Palette default_palette_{};

	R3rCmdBufferUPtr vsync_command_buffer_{};
	R3rCmdBufferUPtr common_command_buffer_{};
	R3rCmdBufferUPtr r2_command_buffer_{};
	R3rCmdBufferUPtr r3_command_buffer_{};

	using R3rCmdBuffers = std::array<R3rCmdBuffer*, 3>;
	R3rCmdBuffers command_buffers_{};

	R3rR2Texture* ui_t2d_{};
	R3rBufferUPtr ui_ib_{};
	R3rBufferUPtr ui_vb_{};
	R3rVertexInputUPtr ui_vi_{};

	R3rR2Texture* r2_black_t2d_1x1_{};
	R3rR2Texture* r2_white_t2d_1x1_{};

	R3rBufferUPtr r2_fillers_ib_{};
	R3rBufferUPtr r2_fillers_vb_{};
	R3rVertexInputUPtr r2_fillers_vi_{};

	bool r2_fade_is_enabled_{};
	VertexColor r2_fade_color_{};
	R3rR2Texture* r2_fade_t2d_{};

	cgm::Mat4D r3_matrix_bs_to_r_{};
	cgm::Mat4D r3_matrix_model_{};
	cgm::Mat4D r3_matrix_view_{};
	cgm::Mat4D r3_matrix_projection_{};

	R3rBufferUPtr flooring_ib_{};
	R3rBufferUPtr flooring_vb_{};
	R3rVertexInputUPtr flooring_vi_{};
	R3rR2Texture* flooring_solid_t2d_{};
	R3rR2Texture* flooring_textured_t2d_{};

	R3rBufferUPtr ceiling_ib_{};
	R3rBufferUPtr ceiling_vb_{};
	R3rVertexInputUPtr ceiling_vi_{};
	R3rR2Texture* ceiling_solid_t2d_{};
	R3rR2Texture* ceiling_textured_t2d_{};

	double player_angle_rad_{};

	cgm::Vec2D view_direction_{};
	cgm::Vec2D player_position_{};
	cgm::Vec3D view_position_{};

	bool has_active_pushwall_{};
	int active_pushwall_next_x_{};
	int active_pushwall_next_y_{};

	int wall_count_{};
	int wall_side_count_{};
	int wall_vertex_count_{};
	int wall_last_xy_to_render_at_{};
	XyWallMap xy_wall_map_{};
	WallsToRenderList walls_to_render_{};

	int wall_side_draw_item_count_{};
	WallSideDrawItems wall_side_draw_items_{};

	R3rBufferUPtr wall_sides_ib_{};
	R3rBufferUPtr wall_sides_vb_{};
	R3rVertexInputUPtr wall_sides_vi_{};

	WallSideIndexBuffer wall_sides_ibi_{};

	int pushwall_count_{};
	int pushwall_side_count_{};
	int pushwall_last_xy_to_render_at_{};
	XyWallMap xy_pushwall_map_{};
	WallsVbi pushwall_to_wall_vbi_{};
	WallsToRenderList pushwalls_to_render_{};

	int pushwall_side_draw_item_count_{};
	WallSideDrawItems pushwall_side_draw_items_{};

	R3rBufferUPtr pushwall_sides_ib_{};
	R3rBufferUPtr pushwall_sides_vb_{};
	R3rVertexInputUPtr pushwall_sides_vi_{};

	WallSideIndexBuffer pushwall_sides_ibi_{};
	PushwallsVbi pushwalls_vbi_{};

	int door_count_{};

	XyDoorMap xy_door_map_{};

	int door_draw_item_count_{};
	int door_last_xy_to_render_at_{};
	DoorsToRenderList doors_to_render_{};
	DoorDrawItems door_draw_items_{};

	R3rBufferUPtr door_sides_ib_{};
	R3rBufferUPtr door_sides_vb_{};
	R3rVertexInputUPtr door_sides_vi_{};

	DoorIndexBuffer door_sides_ibi_{};
	DoorsVbi doors_vbi_{};

	Sprites statics_{};
	StaticsToRenderList statics_to_render_{};

	using ActorsToReposition = std::vector<Sprite>;
	Sprites actors_{};

	int sprites_draw_count_{};
	ActorsToRenderList actors_to_render_{};
	SpritesDrawList sprites_draw_list_{};

	R3rBufferUPtr sprites_ib_{};
	R3rBufferUPtr sprites_vb_{};
	R3rVertexInputUPtr sprites_vi_{};

	SpritesIndexBuffer sprites_ibi_{};
	Vbi sprites_vbi_{};

	R3rBufferUPtr player_weapon_ib_{};
	R3rBufferUPtr player_weapon_vb_{};
	R3rVertexInputUPtr player_weapon_vi_{};

	cgm::Mat4D player_weapon_model_matrix_{};
	cgm::Mat4D player_weapon_view_matrix_{};
	cgm::Mat4D player_weapon_projection_matrix_{};

	bool r3_fade_is_enabled_{};
	R3rBufferUPtr r3_fade_ib_{};
	R3rBufferUPtr r3_fade_vb_{};
	R3rVertexInputUPtr r3_fade_vi_{};
	R3rR2Texture* r3_fade_t2d_{};

	bool fizzle_fx_is_enabled_{};
	bool fizzle_fx_is_fading_{};
	int fizzle_fx_color_index_{};
	float fizzle_fx_ratio_{};

	R3rShaderUPtr fragment_shader_{};
	R3rShaderUPtr vertex_shader_{};
	R3rShaderStageUPtr shader_stage_{};

	R3rShaderMat4Var* model_mat_uniform_{};
	R3rShaderMat4Var* view_mat_uniform_{};
	R3rShaderMat4Var* projection_mat_uniform_{};

	R3rShaderR2SamplerVar* sampler_uniform_{};
	R3rShaderInt32Var* shading_mode_uniform_{};
	R3rShaderFloat32Var* shade_max_uniform_{};
	R3rShaderFloat32Var* normal_shade_uniform_{};
	R3rShaderFloat32Var* height_numerator_uniform_{};
	R3rShaderFloat32Var* extra_lighting_uniform_{};
	R3rShaderVec2Var* view_direction_uniform_{};
	R3rShaderVec2Var* view_position_uniform_{};

	static void convert(const cgm::Vec4F& src, R3rVec4& dst);
	static void convert(const cgm::Vec2D& src, R3rVec2& dst);
	static void convert(const cgm::Mat4D& src, R3rMat4& dst);

	static R3rMipmapMode cfg_texture_mipmap_filter_to_renderer(R3rFilterType filter_type);
	static int cfg_texture_anisotropy_to_renderer(int value) noexcept;

	static int get_static_index(const statobj_t& bs_static) noexcept;
	static int get_actor_index(const objtype& bs_actor) noexcept;

	static constexpr int encode_xy(int x, int y) noexcept;

	static VertexColor vga_color_to_rgba_8(int vga_red, int vga_green, int vga_blue) noexcept;

	static void destroy_sampler(R3rSamplerUPtr& sampler) noexcept;
	static void destroy_index_buffer(R3rBufferUPtr& index_buffer) noexcept;

	R3rBufferUPtr create_index_buffer(R3rBufferUsageType usage_type, int byte_depth, int index_count);

	template<typename TIndex>
	static void update_index_buffer(
		const R3rBufferUPtr& index_buffer,
		int index_offset,
		int index_count,
		const TIndex* indices);

	static void destroy_vertex_buffer(R3rBufferUPtr& vertex_buffer) noexcept;

	template<typename TVertex>
	R3rBufferUPtr create_vertex_buffer(R3rBufferUsageType usage_type, int vertex_count);

	template<typename TVertex>
	static void update_vertex_buffer(
		const R3rBufferUPtr& vertex_buffer,
		int vertex_offset,
		int vertex_count,
		const TVertex* vertices);

	static void destroy_vertex_input(R3rVertexInputUPtr& vertex_input) noexcept;

	template<typename TVertex, bool TIsExist = false>
	struct VertexInputAddAttribDescr
	{
		void operator()(
			int location,
			R3rVertexAttribFormat format,
			int offset,
			int stride,
			const cgm::Vec4F& default_value,
			const R3rBufferUPtr& vertex_buffer,
			R3rVertexAttribDescrs& attribute_descriptions) const
		{
			static_cast<void>(format);
			static_cast<void>(offset);
			static_cast<void>(stride);
			static_cast<void>(vertex_buffer);

			attribute_descriptions.emplace_back();

			auto& description = attribute_descriptions.back();
			description.is_default = true;
			description.location = location;
			description.format = R3rVertexAttribFormat::none;
			description.vertex_buffer = nullptr;
			description.offset = -1;
			description.stride = -1;
			convert(default_value, description.default_value);
		}
	};

	template<typename TVertex>
	struct VertexInputAddAttribDescr<TVertex, true>
	{
		void operator()(
			int location,
			R3rVertexAttribFormat format,
			int offset,
			int stride,
			const cgm::Vec4F& default_value,
			const R3rBufferUPtr& vertex_buffer,
			R3rVertexAttribDescrs& attribute_descriptions) const
		{
			attribute_descriptions.emplace_back();

			auto& description = attribute_descriptions.back();
			description.is_default = false;
			description.location = location;
			description.format = format;
			description.vertex_buffer = vertex_buffer.get();
			description.offset = offset;
			description.stride = stride;
			convert(default_value, description.default_value);
		}
	};

	template<typename TVertex, VertexAttribLocationId TLocationId>
	void add_vertex_input_attrib_descr(
		R3rVertexAttribFormat format,
		const cgm::Vec4F& default_value,
		const R3rBufferUPtr& vertex_buffer,
		R3rVertexAttribDescrs& attribute_descriptions);

	template<typename TVertex>
	void create_vertex_input(
		const R3rBufferUPtr& index_buffer,
		const R3rBufferUPtr& vertex_buffer,
		R3rVertexInputUPtr& vertex_input);

	void update_player_direction() noexcept;
	void update_player_position() noexcept;
	void update_player_view_position() noexcept;
	void update_player() noexcept;

	void initialize_ui_buffer();

	static void destroy_shader(R3rShaderUPtr& shader) noexcept;
	void create_shader(R3rShaderType type, R3rShaderUPtr& shader);
	void destroy_fragment_shader() noexcept;
	void create_fragment_shader();
	void destroy_vertex_shader() noexcept;
	void create_vertex_shader();

	void destroy_shader_stage() noexcept;
	void create_shader_stage();

	template<typename T>
	static void uninitialize_uniform(T*& var) noexcept;

	void uninitialize_model_mat_uniform() noexcept;

	struct InitializeInt32UniformTag {};
	struct InitializeFloat32UniformTag {};
	struct InitializeVec2UniformTag {};
	struct InitializeMat4UniformTag {};
	struct InitializeSampler2dUniformTag {};

	void initialize_uniform(
		const char* name,
		R3rShaderInt32Var*& var,
		InitializeInt32UniformTag) noexcept;

	void initialize_uniform(
		const char* name,
		R3rShaderFloat32Var*& var,
		InitializeFloat32UniformTag) noexcept;

	void initialize_uniform(
		const char* name,
		R3rShaderVec2Var*& var,
		InitializeVec2UniformTag) noexcept;

	void initialize_uniform(
		const char* name,
		R3rShaderMat4Var*& var,
		InitializeMat4UniformTag) noexcept;

	void initialize_uniform(
		const char* name,
		R3rShaderR2SamplerVar*& var,
		InitializeSampler2dUniformTag) noexcept;

	template<typename T>
	void initialize_uniform(const char* name, T*& var);

	void initialize_model_mat_uniform();
	void uninitialize_view_mat_uniform() noexcept;
	void initialize_view_mat_uniform();
	void uninitialize_projection_mat_uniform() noexcept;
	void initialize_projection_mat_uniform();

	void uninitialize_sampler_uniform() noexcept;
	void initialize_sampler_uniform();

	void uninitialize_shading_mode_uniform() noexcept;
	void initialize_shading_mode_uniform();

	void uninitialize_shade_max_uniform() noexcept;
	void initialize_shade_max_uniform();

	void uninitialize_normal_shade_uniform() noexcept;
	void initialize_normal_shade_uniform();

	void uninitialize_height_numerator_uniform() noexcept;
	void initialize_height_numerator_uniform();

	void uninitialize_extra_lighting_uniform() noexcept;
	void initialize_extra_lighting_uniform();

	void uninitialize_view_direction_uniform() noexcept;
	void initialize_view_direction_uniform();

	void uninitialize_view_position_uniform() noexcept;
	void initialize_view_position_uniform();

	void uninitialize_shading_uniforms() noexcept;
	void initialize_shading_uniforms();

	void uninitialize_uniforms() noexcept;
	void initialize_uniforms();

	void uninitialize_program() noexcept;
	void initialize_program();

	static R3rType get_renderer_type(RendererType renderer_type);

	void log_device_features();

	void initialize_renderer();

	void destroy_ui_ib() noexcept;
	void create_ui_ib();
	void destroy_ui_vi() noexcept;
	void create_ui_vi();

	void fill_2d_vb(float left_f, float right_f, float width_f, int vertex_offset) noexcept;
	void fill_2d_vb_stretched() noexcept;
	void fill_2d_vb_non_stretched() noexcept;

	void destroy_ui_vb() noexcept;
	void create_ui_vb();

	void destroy_2d_fillers_ib() noexcept;
	void create_2d_fillers_ib();
	void destroy_2d_fillers_vb() noexcept;
	void create_2d_fillers_vb();
	void destroy_2d_fillers_vi() noexcept;
	void create_2d_fillers_vi();

	void destroy_black_1x1_2d_texture() noexcept;
	void create_black_1x1_2d_texture();

	void destroy_white_1x1_2d_texture() noexcept;
	void create_white_1x1_2d_texture();

	void destroy_fade_1x1_2d_texture() noexcept;
	void create_fade_1x1_2d_texture();

	void destroy_ui_texture() noexcept;
	void create_ui_texture();

	void uninitialize_2d() noexcept;
	void initialize_2d();

	void destroy_flooring_ib() noexcept;
	void create_flooring_ib();
	void destroy_flooring_vb() noexcept;
	void create_flooring_vb();
	void destroy_flooring_vi() noexcept;
	void create_flooring_vi();

	void destroy_solid_flooring_r2_texture() noexcept;
	void create_solid_flooring_r2_texture();

	void uninitialize_flooring() noexcept;
	void initialize_flooring();

	void destroy_ceiling_ib() noexcept;
	void create_ceiling_ib();
	void destroy_ceiling_vb() noexcept;
	void create_ceiling_vb();
	void destroy_ceiling_vi() noexcept;
	void create_ceiling_vi();

	void destroy_solid_ceiling_r2_texture() noexcept;
	void create_solid_ceiling_r2_texture();

	void initialize_ceiling();
	void uninitialize_ceiling() noexcept;

	void create_walls_ib();
	void destroy_walls_ib() noexcept;
	void create_walls_vb();
	void destroy_walls_vi() noexcept;
	void create_walls_vi();
	void destroy_walls_vb() noexcept;
	void initialize_walls();
	void uninitialize_walls() noexcept;

	void create_pushwalls_ibi();
	void create_pushwalls_ib();
	void destroy_pushwalls_ibi() noexcept;
	void destroy_pushwalls_ib() noexcept;
	void create_pushwalls_vb();
	void destroy_pushwalls_vi() noexcept;
	void create_pushwalls_vi();
	void destroy_pushwalls_vb() noexcept;
	void initialize_pushwalls();
	void uninitialize_pushwalls() noexcept;

	void create_door_sides_ibi();
	void create_door_sides_ib();
	void destroy_door_sides_ibi() noexcept;
	void destroy_door_sides_ib() noexcept;
	void create_door_sides_vb();
	void destroy_door_sides_vb() noexcept;
	void destroy_door_sides_vi() noexcept;
	void create_door_sides_vi();
	void initialize_door_sides();
	void uninitialize_door_sides() noexcept;

	void update_palette_from_vga(int offset, int count) noexcept;
	void initialize_palette() noexcept;

	void calculate_dimensions() noexcept;

	void build_2d_model_matrix() noexcept;
	void build_2d_view_matrix() noexcept;
	void build_2d_projection_matrix() noexcept;
	void build_2d_matrices() noexcept;

	void calculate_camera_parameters() noexcept;

	void build_bs_to_ren_matrix() noexcept;
	void build_model_matrix() noexcept;
	void build_view_matrix() noexcept;
	void build_projection_matrix() noexcept;
	void build_3d_matrices() noexcept;
	void build_matrices() noexcept;

	void set_ui_sampler_default_state() noexcept;
	void update_ui_sampler_state() noexcept;
	void update_ui_sampler();
	void destroy_ui_sampler() noexcept;
	void create_ui_sampler();

	void set_sprite_sampler_default_state() noexcept;
	void update_sprite_sampler_state();
	void update_sprite_sampler();
	void destroy_sprite_sampler() noexcept;
	void create_sprite_sampler();

	void set_wall_sampler_default_state() noexcept;
	void update_wall_sampler_state();
	void update_wall_sampler();
	void destroy_wall_sampler() noexcept;
	void create_wall_sampler();

	void update_player_weapon_vb();
	void destroy_player_weapon_ib() noexcept;
	void create_player_weapon_ib();
	void update_player_weapon_ib();
	void destroy_player_weapon_vb() noexcept;
	void create_player_weapon_vb();
	void destroy_player_weapon_vi() noexcept;
	void create_player_weapon_vi();

	void update_player_weapon_model_matrix();
	void update_player_weapon_view_matrix() noexcept;
	void build_player_weapon_projection_matrix() noexcept;

	void set_player_weapon_sampler_default_state() noexcept;
	void update_player_weapon_sampler_state() noexcept;
	void update_player_weapon_sampler();
	void destroy_player_weapon_sampler() noexcept;
	void create_player_weapon_sampler();

	void uninitialize_player_weapon() noexcept;
	void initialize_player_weapon();

	void destroy_fade_sampler() noexcept;
	void create_fade_sampler();

	void set_samplers_default_states() noexcept;

	void uninitialize_samplers() noexcept;
	void initialize_samplers();

	void destroy_command_buffers() noexcept;
	void create_command_buffers() noexcept;
	void destroy_common_command_buffer() noexcept;

	void create_vsync_command_buffer();
	void destroy_vsync_command_buffer() noexcept;
	void create_common_command_buffer();

	void destroy_2d_command_buffer() noexcept;
	void create_2d_command_buffer();
	void destroy_3d_command_buffer() noexcept;
	void create_3d_command_buffer();

	void uninitialize_command_buffers() noexcept;
	void initialize_command_buffers();

	void destroy_3d_fade_ib() noexcept;
	void create_3d_fade_ib();
	void destroy_3d_fade_vb() noexcept;
	void create_3d_fade_vb();
	void destroy_3d_fade_vi() noexcept;
	void create_3d_fade_vi();
	void update_3d_fade_ib();
	void update_3d_fade_vb();

	void destroy_3d_fade_r2_texture() noexcept;
	void create_r3_fade_r2_texture();

	void uninitialize_3d_fade() noexcept;

	void destroy_texture_manager() noexcept;
	void create_texture_manager();

	void initialize_3d_fade();

	void present_common();
	void present_2d();

	void render_walls();
	void render_pushwalls();
	void render_doors();

	void update_cloaked_actor(const Sprite& sprite);
	static int calculate_actor_anim_rotation(const objtype& bs_actor);
	int get_bs_actor_sprite_id(const objtype& bs_actor);
	void update_actor(std::intptr_t bs_actor_index);
	void orient_sprite(Sprite& sprite);
	void update_static(int bs_static_index);
	void render_sprites();
	void update_3d_fade();
	void present_3d();

	static bool bs_is_activated_pushwall_tile(int tile) noexcept;
	static bool bs_is_door(int tile) noexcept;
	static bool bs_is_door_track(int tile) noexcept;
	static bool bs_is_solid_wall(int tile) noexcept;
	static bool bs_is_pushwall(int x, int y) noexcept;
	bool bs_is_solid_wall(int x, int y) const noexcept;
	int get_solid_wall_side_count(int x, int y) const;
	int get_door_track_wall_id(int x, int y, controldir_t direction) const;

	void precache_flooring();
	void precache_ceiling();
	void precache_wall(int wall_id);
	void precache_horizontal_wall(int tile_wall);
	void precache_vertical_wall(int tile_wall);
	void precache_switches();
	void precache_door_track(int x, int y);
	void precache_walls();
	void precache_pushwalls();
	void precache_door_side(int page_number);
	void precache_door(const doorobj_t& door);
	void precache_doors();

	template<typename TVertex, bool TIsExists = false>
	struct UpdateVertexXyz
	{
		void operator()(TVertex&, const VertexPosition&) const noexcept {}
	};

	template<typename TVertex>
	struct UpdateVertexXyz<TVertex, true>
	{
		void operator()(TVertex& vertex, const VertexPosition& xyz) const noexcept
		{
			vertex.xyz = xyz;
		}
	};

	template<typename TVertex>
	static void update_vertex_xyz(TVertex& vertex, const VertexPosition& xyz) noexcept;

	template<typename TVertex, bool TIsExists = false>
	struct UpdateVertexRgba
	{
		void operator()(TVertex&, const VertexColor&) const noexcept {}
	};

	template<typename TVertex>
	struct UpdateVertexRgba<TVertex, true>
	{
		void operator()(TVertex& vertex, const VertexColor& rgba_8) const noexcept
		{
			vertex.rgba8 = rgba_8;
		}
	};

	template<typename TVertex>
	static void update_vertex_rgba(TVertex& vertex, const VertexColor& rgba_8) noexcept;

	template<typename TVertex, bool TIsExists = false>
	struct UpdateVertexUv
	{
		void operator()(TVertex&, const VertexTextureCoordinates&) const noexcept {}
	};

	template<typename TVertex>
	struct UpdateVertexUv<TVertex, true>
	{
		void operator()(TVertex& vertex, const VertexTextureCoordinates& uv) const noexcept
		{
			vertex.uv = uv;
		}
	};

	template<typename TVertex>
	static void update_vertex_uv(TVertex& vertex, const VertexTextureCoordinates& uv) noexcept;

	template<typename TVertex>
	void map_wall_side(
		controldir_t side_direction,
		Wall& wall,
		int& vertex_index,
		VertexBufferImageT<TVertex>& vb_buffer);

	template<typename TVertex>
	void map_xy_to_xwall(
		XyWallType wall_type,
		int x,
		int y,
		XyWallMap& map,
		int& vertex_index,
		VertexBufferImageT<TVertex>& vb_buffer);

	void build_walls();

	void translate_pushwall_side(
		float translate_x,
		float translate_y,
		controldir_t side_direction,
		const Wall& wall,
		int& vertex_index,
		PushwallsVbi& vb_buffer);

	void translate_pushwall(const Wall& wall, int& vertex_index, PushwallsVbi& vb_buffer);
	void translate_pushwall();
	void step_pushwall(int old_x, int old_y);
	void build_pushwalls();

	template<typename TVertex>
	static void update_quad_vertices(
		QuadFlags flags,
		const VertexPosition& origin,
		const cgm::Vec2F& size,
		int& vertex_index,
		VertexBufferImageT<TVertex>& vb_buffer) noexcept;

	static void map_door_side(DoorSide& door_side, int& vertex_index, DoorsVbi& vb_buffer) noexcept;
	void map_xy_to_door(const doorobj_t& bs_door, int& vertex_index, DoorsVbi& vb_buffer);

	void build_doors();

	void initialize_sprites_ibi();
	void uninitialize_sprites_ib() noexcept;
	void initialize_sprites_vb();
	void uninitialize_sprites_vb() noexcept;
	void uninitialize_sprites_vi() noexcept;
	void initialize_sprites_vi();

	void initialize_statics();
	void initialize_actors();
	void initialize_sprites();
	void uninitialize_statics() noexcept;
	void uninitialize_actors() noexcept;
	void uninitialize_sprites() noexcept;

	void map_sprite(SpriteType sprite_type, int vertex_index, Sprite& sprite) noexcept;
	void map_static(const statobj_t& bs_static) noexcept;

	void cache_sprite(int bs_sprite_id);

	void precache_water_bowl();
	void precache_chicken_leg();
	void precache_ham();
	void precache_candy_bar();
	void precache_sandwich();
	void precache_plasma_detonator_explosion();
	void precache_plasma_detonator();
	void precache_static(const statobj_t& bs_static);
	void precache_statics();

	void map_actor(const objtype& bs_actor);

	// Explosion.
	void precache_explosion();

	// Clip Explosion.
	void precache_clip_explosion();

	// Grenade explosion.
	void precache_grenade_explosion();

	// Flying grenade.
	void precache_flying_grenade();

	void precache_anti_plasma_cannon_explosion();
	void precache_anti_plasma_cannon_shot();

	// A rubble.
	void precache_rubble();

	// Toxic waste (green #1).
	void precache_toxic_waste_green_1();

	// Toxic waste (green #2).
	void precache_toxic_waste_green_2();

	// Toxic waste (black #1).
	void precache_toxic_waste_black_1();

	// Toxic waste (black #2).
	void precache_toxic_waste_black_2();

	// Coin (1).
	void precache_coin_1();

	// Red Access Card.
	void precache_red_access_card();

	// Yellow Access Card.
	void precache_yellow_access_card();

	// Green Access Card (AOG).
	void precache_green_access_card();

	// Blue Access Card.
	void precache_blue_access_card();

	// Golden Access Card (AOG).
	void precache_golden_access_card();

	// Small yellow box (PS).
	void precache_small_yellow_box();

	// Partial / Full Charge Pack.
	void precache_charge_packs();

	// Slow Fire Protector.
	void precache_slow_fire_protector();

	// Rapid Assault Weapon.
	void precache_rapid_assault_weapon();

	// Dual Neutron Disruptor.
	void precache_dual_neutron_disruptor_weapon();

	// Plasma Discharge Unit.
	void precache_plasma_discharge_unit_weapon();

	// Anti-Plasma Cannon.
	void precache_anti_plasma_cannon_weapon();

	// Generic alien spit (#1).
	void precache_generic_alien_spit_1();

	// Generic alien spit (#2).
	void precache_generic_alien_spit_2();

	// Generic alien spit (#3).
	void precache_generic_alien_spit_3();

	// Electrical Shot.
	void precache_electrical_shot();

	// Sector Patrol (AOG) / Sector Guard (PS).
	void precache_sector_patrol_or_sector_guard();

	// Robot Turret.
	void precache_robot_turret();

	// Bio-Technician.
	void precache_bio_technician();

	// Pod Alien.
	void precache_pod_alien();

	// Pod Alien Egg.
	void precache_pod_alien_egg();

	// High Energy Plasma Alien.
	void precache_high_energy_plasma_alien();

	// Plasma Sphere.
	void precache_plasma_sphere();

	// Star Sentinel (AOG) / Tech Warrior (PS).
	void precache_star_sentinel_or_tech_warrior();

	// High-Security Genetic Guard.
	void precache_high_security_genetic_guard();

	// Experimental Mech-Sentinel.
	void precache_experimental_mech_sentinel();

	// Experimental Mutant Human.
	void precache_experimental_mutant_human();

	// Morphing Experimental Mutant Human.
	void precache_experimental_mutant_human_morphing();

	// Large Experimental Genetic Alien.
	void precache_large_experimental_genetic_alien();

	// A canister with large Experimental Genetic Alien.
	void precache_canister_with_large_experimental_genetic_alien();

	// Small Experimental Genetic Alien.
	void precache_experimental_genetic_alien_small();

	// A canister with small Experimental Genetic Alien.
	void precache_canister_with_small_experimental_genetic_alien();

	// Mutated Guard.
	void precache_mutated_guard();

	// Mutated Guard (waiting).
	void precache_mutated_guard_waiting();

	// Fluid Alien Shot.
	void precache_fluid_alien_shot();

	// Fluid Alien.
	void precache_fluid_alien();

	// Star Trooper (AOG) / Alien Protector (PS).
	void precache_star_trooper_or_alien_protector();

	// Dr. Goldfire.
	void precache_dr_goldfire();

	// Morphed Dr. Goldfire Shot.
	void precache_morphed_dr_goldfire_shot();

	// Morphed Dr. Goldfire.
	void precache_morphed_dr_goldfire();

	// Volatile Material Transport.
	void precache_volatile_material_transport();

	void precache_perscan_drone();

	// Security Cube Explosion.
	void precache_security_cube_explosion();

	// Security Cube.
	void precache_security_cube_or_projection_generator();

	// Spider Mutant Shot.
	void precache_spider_mutant_shot();

	// Spider Mutant.
	void precache_spider_mutant();

	// Morphing Spider Mutant.
	void precache_spider_mutant_morphing();

	// Breather Beast.
	void precache_breather_beast();

	// Cyborg Warrior.
	void precache_cyborg_warrior();

	// Reptilian Warrior.
	void precache_reptilian_warrior();

	// Reptilian Warrior (morphing).
	void precache_reptilian_warrior_morphing();

	// Acid Dragon Shot.
	void precache_acid_dragon_shot();

	// Acid Dragon.
	void precache_acid_dragon();

	// Bio-Mech Guardian.
	void precache_bio_mech_guardian();

	// The Giant Stalker.
	void precache_the_giant_stalker();

	// The Spector Demon.
	void precache_the_spector_demon();

	// The Armored Stalker.
	void precache_the_armored_stalker();

	// The Crawler Beast Shot.
	void precache_the_crawler_beast_shot();

	// The Crawler Beast.
	void precache_the_crawler_beast();

	// Blake Stone.
	void precache_blake_stone();

	void precache_vent_and_dripping_blood();
	void precache_vent_and_dripping_water();
	void precache_flicker_light();
	void precache_crate_content();
	void precache_crate_1();
	void precache_crate_2();
	void precache_crate_3();
	void precache_electrical_post_barrier();
	void precache_electrical_arc_barrier();
	void precache_vertical_post_barrier();
	void precache_vertical_spike_barrier();
	void precache_security_light();
	void precache_grate_and_steam();
	void precache_pipe_and_steam();
	void precache_special_stuff();
	void precache_access_cards();
	void precache_player_weapon_auto_charge_pistol();
	void precache_player_weapon_slow_fire_protector();
	void precache_player_weapon_rapid_assault_weapon();
	void precache_player_weapon_dual_neutron_disruptor();
	void precache_player_weapon_plasma_discharge_unit();
	void precache_player_weapon_anti_plasma_cannon();
	void precache_player_weapons();
	void precache_dead(const objtype& bs_actor);
	void precache_actors();
	void precache_sprites();

	void build_statics();
	void build_actors();
	void build_sprites();
	void precache_resources();
	void destroy_texture_upscale_resources() noexcept;
	void create_texture_upscale_resources();
	void destroy_external_textures_resources() noexcept;
	void create_external_textures_resources();

	void uninitialize_video() noexcept;
	void initialize_video();
};

// ==========================================================================

using HwVideoPool = SinglePoolResource<HwVideo>;
HwVideoPool hw_video_pool{};

// ==========================================================================

HwVideo::HwVideo()
try {
	initialize_video();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

HwVideo::~HwVideo()
{
	uninitialize_video();
}

void* HwVideo::operator new(std::size_t size)
try {
	return hw_video_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::operator delete(void* ptr)
{
	hw_video_pool.deallocate(ptr);
}

bool HwVideo::is_hardware() const noexcept
{
	return true;
}

StringView HwVideo::get_renderer_name()
{
	return renderer_->get_name();
}

void HwVideo::clear_vga_buffer()
{
}

void HwVideo::take_screenshot(
	int width,
	int height,
	int stride_rgb_888,
	ScreenshotBuffer&& src_pixels_rgb_888)
try {
	auto is_flipped_vertically = false;

	renderer_->read_pixels(
		sys::PixelFormat::r8g8b8,
		src_pixels_rgb_888.get(),
		is_flipped_vertically);

	vid_schedule_save_screenshot_task(
		width,
		height,
		stride_rgb_888,
		std::move(src_pixels_rgb_888),
		is_flipped_vertically
	);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::vsync_present()
try {
	if (renderer_ == nullptr)
	{
		return;
	}

	auto command_buffers = vsync_command_buffer_.get();
	renderer_->submit_commands(make_span(&command_buffers, 1));
	renderer_->present();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::present()
try {
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

	renderer_->submit_commands(make_span(
		command_buffers_.data(),
		static_cast<std::intptr_t>(command_buffers_.size())));

	if (vid_is_take_screenshot_scheduled)
	{
		vid_take_screenshot();
	}

	renderer_->present();

	is_draw_3d_ = false;

	renderer_->enable_checking_api_calls_for_errors(vid_check_r3_api_call_for_errors());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::get_palette(int offset, int count, std::uint8_t* vga_palette) const
try {
	if (offset < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid offset.");
	}

	if (count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid count.");
	}

	if ((offset + count) > 256)
	{
		BSTONE_THROW_STATIC_SOURCE("The range outside of bounds.");
	}

	if (!vga_palette)
	{
		BSTONE_THROW_STATIC_SOURCE("Null palette.");
	}

	auto& dst_vga_palette = *reinterpret_cast<VgaPalette*>(vga_palette);

	std::copy_n(
		vga_palette_.cbegin() + offset,
		count,
		dst_vga_palette.begin()
	);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::fill_palette(int r, int g, int b) noexcept
{
	for (auto& vga_color : vga_palette_)
	{
		vga_color[0] = static_cast<std::uint8_t>(r);
		vga_color[1] = static_cast<std::uint8_t>(g);
		vga_color[2] = static_cast<std::uint8_t>(b);
	}

	update_palette_from_vga(0, 256);
}

void HwVideo::set_palette(int offset, int count, const std::uint8_t* vga_palette)
try {
	if (offset < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid offset.");
	}

	if (count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid count.");
	}

	if ((offset + count) > 256)
	{
		BSTONE_THROW_STATIC_SOURCE("The range outside of bounds.");
	}

	if (!vga_palette)
	{
		BSTONE_THROW_STATIC_SOURCE("Null palette.");
	}

	const auto& src_vga_palette = *reinterpret_cast<const VgaPalette*>(vga_palette);

	std::copy_n(
		src_vga_palette.cbegin(),
		count,
		vga_palette_.begin() + offset
	);

	update_palette_from_vga(offset, count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::fade_out(int start, int end, int red, int green, int blue, int step_count)
try {
	static_cast<void>(start);
	static_cast<void>(end);

	r2_fade_is_enabled_ = true;
	r2_fade_color_ = vga_color_to_rgba_8(red, green, blue);

	if (!gp_no_fade_in_or_out())
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::fade_in(int start, int end, const std::uint8_t* palette, int step_count)
try {
	static_cast<void>(start);
	static_cast<void>(end);

	r2_fade_is_enabled_ = true;
	screenfaded = false;
	
	set_palette(0, 256, palette);

	r2_fade_color_.a_ = 0xFF;

	if (!gp_no_fade_in_or_out())
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::apply_widescreen()
try {
	calculate_dimensions();
	SetViewSize();
	build_projection_matrix();
	build_player_weapon_projection_matrix();
	update_3d_fade_vb();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::apply_window_mode()
try {
	calculate_dimensions();
	vid_initialize_vanilla_raycaster();

	auto param = R3rUtilsSetWindowModeParam{};
	param.is_native = vid_is_native_mode();
	param.size.width = vid_layout_.window_width;
	param.size.height = vid_layout_.window_height;
	R3rUtils::set_window_mode(renderer_->get_window(), param);
	renderer_->handle_resize(sys::WindowSize{vid_layout_.window_width, vid_layout_.window_height});

	vid_initialize_common();

	uninitialize_2d();
	initialize_2d();

	uninitialize_3d_fade();
	initialize_3d_fade();

	uninitialize_player_weapon();
	initialize_player_weapon();

	build_matrices();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::apply_filler_color_index()
try {
	destroy_2d_fillers_vi();
	destroy_2d_fillers_ib();
	destroy_2d_fillers_vb();

	create_2d_fillers_ib();
	create_2d_fillers_vb();
	create_2d_fillers_vi();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const Rgba8Palette& HwVideo::get_default_palette() const noexcept
{
	return default_palette_;
}

void HwVideo::enable_fizzle_fx(bool is_enabled)
{
	fizzle_fx_is_enabled_ = is_enabled;
}

void HwVideo::enable_fizzle_fx_fading(bool is_fading)
{
	fizzle_fx_is_fading_ = is_fading;
}

void HwVideo::set_fizzle_fx_color_index(int color_index)
{
	fizzle_fx_color_index_ = color_index;
}

void HwVideo::set_fizzle_fx_ratio(float ratio)
{
	fizzle_fx_ratio_ = ratio;
}

void HwVideo::clear_wall_render_list() noexcept
{
	wall_last_xy_to_render_at_ = -1;
	walls_to_render_.clear();
}

void HwVideo::add_wall_render_item(int tile_x, int tile_y)
try {
	validate_tile_xy(tile_x, tile_y);

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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::clear_pushwall_render_list() noexcept
{
	pushwall_last_xy_to_render_at_ = -1;
	pushwalls_to_render_.clear();
}

void HwVideo::add_pushwall_render_item(int tile_x, int tile_y)
try {
	validate_tile_xy(tile_x, tile_y);

	const auto xy = encode_xy(tile_x, tile_y);

	if (pushwall_last_xy_to_render_at_ == xy)
	{
		return;
	}

	pushwall_last_xy_to_render_at_ = xy;
	pushwalls_to_render_.insert(xy);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::clear_door_render_list() noexcept
{
	door_last_xy_to_render_at_ = -1;
	doors_to_render_.clear();
}

void HwVideo::add_door_render_item(int tile_x, int tile_y)
try {
	validate_tile_xy(tile_x, tile_y);

	const auto xy = encode_xy(tile_x, tile_y);

	if (door_last_xy_to_render_at_ == xy)
	{
		return;
	}

	door_last_xy_to_render_at_ = xy;

	doors_to_render_.insert(xy);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::clear_static_render_list() noexcept
{
	statics_to_render_.clear();
}

void HwVideo::add_static_render_item(int bs_static_index)
try {
	statics_to_render_.insert(bs_static_index);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::clear_actor_render_list() noexcept
{
	actors_to_render_.clear();
}

void HwVideo::add_actor_render_item(int bs_actor_index)
try {
	actors_to_render_.emplace(bs_actor_index);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::on_load_level()
try {
	precache_resources();

	build_pushwalls();
	build_walls();
	build_doors();
	build_sprites();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::on_update_wall_switch(int x, int y)
try {
	validate_tile_xy(x, y);

	const auto xy = encode_xy(x, y);

	auto wall_it = xy_wall_map_.find(xy);

	if (wall_it == xy_wall_map_.cend())
	{
		const auto& assets_info = get_assets_info();

		if (assets_info.is_aog())
		{
			BSTONE_THROW_DYNAMIC_SOURCE(
				("Expected wall at (" + std::to_string(x) + ", " + std::to_string(y) + ").").c_str());
		}
		else
		{
			// Operable non-directly.
			return;
		}
	}

	const auto tile_wall = tilemap[x][y] & tilemap_wall_mask;


	if (tile_wall != OFF_SWITCH && tile_wall != ON_SWITCH)
	{
		BSTONE_THROW_STATIC_SOURCE("Not a wall switch.");
	}

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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::on_move_pushwall()
try {
	translate_pushwall();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::on_step_pushwall(int old_x, int old_y)
try {
	validate_tile_xy(old_x, old_y);

	step_pushwall(old_x, old_y);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::on_pushwall_to_wall(int old_x, int old_y, int new_x, int new_y)
try {
	validate_tile_xy(old_x, old_y);
	validate_tile_xy(new_x, new_y);

	const auto old_xy = encode_xy(old_x, old_y);
	const auto old_pushwall_it = xy_pushwall_map_.find(old_xy);

	if (old_pushwall_it == xy_pushwall_map_.cend())
	{
		BSTONE_THROW_STATIC_SOURCE("Pushwall not found.");
	}

	xy_pushwall_map_.erase(old_pushwall_it);

	auto vertex_index = 0;

	map_xy_to_xwall(
		XyWallType::solid,
		new_x,
		new_y,
		xy_wall_map_,
		vertex_index,
		pushwall_to_wall_vbi_);

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
		pushwall_to_wall_vbi_.data());

	wall_vertex_count_ += vertex_index;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::on_move_door(int door_index)
try {
	const auto& bs_door = doorobjlist[door_index];
	const auto xy = encode_xy(bs_door.tilex, bs_door.tiley);
	const auto map_it = xy_door_map_.find(xy);

	if (map_it == xy_door_map_.cend())
	{
		BSTONE_THROW_STATIC_SOURCE("Door mapping not found.");
	}

	auto& door = xy_door_map_[xy];

	auto vertex_index = door.vertex_index;
	const auto old_vertex_index = vertex_index;

	map_door_side(door.sides.front(), vertex_index, doors_vbi_);

	update_vertex_buffer(
		door_sides_vb_,
		old_vertex_index,
		vertices_per_door,
		&doors_vbi_[old_vertex_index]);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::on_update_door_lock(int bs_door_index)
try {
	const auto& bs_door = doorobjlist[bs_door_index];

	const auto xy = encode_xy(bs_door.tilex, bs_door.tiley);

	const auto map_it = xy_door_map_.find(xy);

	if (map_it == xy_door_map_.cend())
	{
		BSTONE_THROW_STATIC_SOURCE("Door mapping not found.");
	}

	auto& door = xy_door_map_[xy];
	door_get_page_numbers(bs_door, door.sides[0].texture_id, door.sides[1].texture_id);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::on_remove_static(const statobj_t& bs_static)
try {
	const auto bs_static_index = get_static_index(bs_static);
	auto& hw_static = statics_[bs_static_index];
	hw_static = {};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::on_remove_actor(const objtype& bs_actor)
try {
	const auto bs_actor_index = get_actor_index(bs_actor);
	auto& hw_actor = actors_[bs_actor_index];
	hw_actor = {};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::apply_vsync()
try {
	renderer_->enable_vsync(vid_cfg_is_vsync());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::apply_msaa()
try {
	renderer_->set_anti_aliasing(vid_cfg_get_aa_type(), vid_cfg_get_aa_degree());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::apply_texture_upscale()
try {
	destroy_texture_upscale_resources();

	texture_mgr_->set_upscale_filter(
		vid_cfg_get_texture_upscale_type(),
		vid_cfg_get_texture_upscale_xbrz_degree()
	);

	create_texture_upscale_resources();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::apply_external_textures()
try {
	destroy_external_textures_resources();
	texture_mgr_->enable_external_textures(vid_cfg_is_external_textures_enabled());
	create_external_textures_resources();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_samplers()
try {
	update_ui_sampler();
	update_sprite_sampler();
	update_wall_sampler();
	update_player_weapon_sampler();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rDeviceFeatures& HwVideo::get_device_features() const noexcept
{
	return device_features_;
}

void HwVideo::door_get_page_numbers(const doorobj_t& door, int& front_face_page_number, int& back_face_page_number)
{
	front_face_page_number = DOORWALL + door.vertical;
	back_face_page_number = DOORWALL + door.vertical;

	auto is_one_way = false;
	const auto is_unlocked = (door.lock == kt_none);

	switch (door.type)
	{
		case dr_normal:
			front_face_page_number += L_METAL;
			break;

		case dr_elevator:
			front_face_page_number += L_ELEVATOR;
			break;

		case dr_prison:
			front_face_page_number += L_PRISON;
			break;

		case dr_space:
			front_face_page_number += L_SPACE;
			break;

		case dr_bio:
			front_face_page_number += L_BIO;
			break;

		case dr_high_security:
			front_face_page_number += L_HIGH_SECURITY;
			break;

		case dr_office:
			front_face_page_number += L_HIGH_TECH;
			break;

		case dr_oneway_up:
			is_one_way = true;
			front_face_page_number += L_ENTER_ONLY + (is_unlocked ? UL_METAL : 0);
			back_face_page_number += NOEXIT;
			break;

		case dr_oneway_left:
			is_one_way = true;
			front_face_page_number += NOEXIT;
			back_face_page_number += L_ENTER_ONLY + (is_unlocked ? UL_METAL : 0);
			break;

		case dr_oneway_right:
			is_one_way = true;
			front_face_page_number += L_ENTER_ONLY + (is_unlocked ? UL_METAL : 0);
			back_face_page_number += NOEXIT;
			break;

		case dr_oneway_down:
			is_one_way = true;
			front_face_page_number += NOEXIT;
			back_face_page_number += L_ENTER_ONLY + (is_unlocked ? UL_METAL : 0);
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Invalid door type.");
	}

	if (!is_one_way)
	{
		front_face_page_number += (is_unlocked ? UL_METAL : 0);
		back_face_page_number = front_face_page_number;
	}
}

void HwVideo::validate_tile_xy(int tile_x, int tile_y)
try {
	if (tile_x < 0 || tile_x >= MAPSIZE ||
		tile_y < 0 || tile_y >= MAPSIZE)
	{
		BSTONE_THROW_STATIC_SOURCE("Tile coordinates out of range.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::log(LoggerMessageType message_type, const std::string& message)
{
	globals::logger->log(message_type, (log_prefix + message).c_str());
}

void HwVideo::log(const std::string& message)
{
	log(LoggerMessageType::information, message);
}

void HwVideo::log_error(const std::string& message)
{
	log(LoggerMessageType::error, message);
}

void HwVideo::convert(const cgm::Vec4F& src, R3rVec4& dst)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

void HwVideo::convert(const cgm::Vec2D& src, R3rVec2& dst)
{
	dst[0] = static_cast<float>(src[0]);
	dst[1] = static_cast<float>(src[1]);
}

void HwVideo::convert(const cgm::Mat4D& src, R3rMat4& dst)
{
	for (auto i = decltype(cgm::Mat4D::item_count){}; i < cgm::Mat4D::item_count; ++i)
	{
		dst[i] = static_cast<float>(src[i]);
	}
}

R3rMipmapMode HwVideo::cfg_texture_mipmap_filter_to_renderer(R3rFilterType filter_type)
try {
	switch (filter_type)
	{
		case R3rFilterType::nearest: return R3rMipmapMode::nearest;
		case R3rFilterType::linear: return R3rMipmapMode::linear;
		default: BSTONE_THROW_STATIC_SOURCE("Invalid mipmap mode.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int HwVideo::cfg_texture_anisotropy_to_renderer(int value) noexcept
{
	if (value < R3rLimits::min_anisotropy_off())
	{
		return R3rLimits::min_anisotropy_off();
	}
	else if (value > R3rLimits::max_anisotropy())
	{
		return R3rLimits::max_anisotropy();
	}
	else
	{
		return value;
	}
}

int HwVideo::get_static_index(const statobj_t& bs_static) noexcept
{
	return static_cast<int>(&bs_static - statobjlist.data());
}

int HwVideo::get_actor_index(const objtype& bs_actor) noexcept
{
	return static_cast<int>(&bs_actor - objlist);
}

constexpr int HwVideo::encode_xy(int x, int y) noexcept
{
	return (x << 8) | y;
}

auto HwVideo::vga_color_to_rgba_8(int vga_red, int vga_green, int vga_blue) noexcept -> VertexColor
{
	return VertexColor
	{
		static_cast<std::uint8_t>((255 * vga_red) / 63),
		static_cast<std::uint8_t>((255 * vga_green) / 63),
		static_cast<std::uint8_t>((255 * vga_blue) / 63),
		255
	};
}

void HwVideo::destroy_sampler(R3rSamplerUPtr& sampler) noexcept
{
	sampler = nullptr;
}

void HwVideo::destroy_index_buffer(R3rBufferUPtr& index_buffer) noexcept
{
	index_buffer = nullptr;
}

R3rBufferUPtr HwVideo::create_index_buffer(R3rBufferUsageType usage_type, int byte_depth, int index_count)
try {
	const auto index_buffer_size = index_count * byte_depth;

	auto param = R3rBufferInitParam{};
	param.type = R3rBufferType::index;
	param.usage_type = usage_type;
	param.size = index_buffer_size;

	return renderer_->create_buffer(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<typename TIndex>
void HwVideo::update_index_buffer(
	const R3rBufferUPtr& index_buffer,
	int index_offset,
	int index_count,
	const TIndex* indices)
try {
	const auto byte_depth = static_cast<int>(sizeof(TIndex));
	const auto offset = index_offset * byte_depth;
	const auto size = index_count * byte_depth;

	auto param = R3rUpdateBufferParam{};
	param.offset = offset;
	param.size = size;
	param.data = indices;

	index_buffer->update(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_vertex_buffer(R3rBufferUPtr& vertex_buffer) noexcept
{
	vertex_buffer = nullptr;
}

template<typename TVertex>
R3rBufferUPtr HwVideo::create_vertex_buffer(R3rBufferUsageType usage_type, int vertex_count)
try {
	const auto vertex_size = static_cast<int>(sizeof(TVertex));
	const auto vertex_buffer_size = vertex_count * vertex_size;

	auto param = R3rBufferInitParam{};
	param.type = R3rBufferType::vertex;
	param.usage_type = usage_type;
	param.size = vertex_buffer_size;

	return renderer_->create_buffer(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<typename TVertex>
void HwVideo::update_vertex_buffer(
	const R3rBufferUPtr& vertex_buffer,
	int vertex_offset,
	int vertex_count,
	const TVertex* vertices)
try {
	const auto vertex_size = static_cast<int>(sizeof(TVertex));
	const auto offset = vertex_offset * vertex_size;
	const auto size = vertex_count * vertex_size;

	auto param = R3rUpdateBufferParam{};
	param.offset = offset;
	param.size = size;
	param.data = vertices;

	vertex_buffer->update(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_vertex_input(R3rVertexInputUPtr& vertex_input) noexcept
{
	vertex_input = nullptr;
}

template<typename TVertex, HwVideo::VertexAttribLocationId TLocationId>
void HwVideo::add_vertex_input_attrib_descr(
	R3rVertexAttribFormat format,
	const cgm::Vec4F& default_value,
	const R3rBufferUPtr& vertex_buffer,
	R3rVertexAttribDescrs& attribute_descriptions)
{
	const auto traits = VertexAttributeTraits<TVertex, TLocationId>{};
	const auto add_attribute = VertexInputAddAttribDescr<TVertex, traits.is_valid>{};

	auto location = 0;

	switch (TLocationId)
	{
		case VertexAttribLocationId::position:
			location = HwShaderRegistry::get_a_position_location();
			break;

		case VertexAttribLocationId::color:
			location = HwShaderRegistry::get_a_color_location();
			break;

		case VertexAttribLocationId::texture_coordinates:
			location = HwShaderRegistry::get_a_tx_coords_location();
			break;
	}

	add_attribute(
		location,
		format,
		traits.offset,
		traits.stride,
		default_value,
		vertex_buffer,
		attribute_descriptions);
}

template<typename TVertex>
void HwVideo::create_vertex_input(
	const R3rBufferUPtr& index_buffer,
	const R3rBufferUPtr& vertex_buffer,
	R3rVertexInputUPtr& vertex_input)
try {
	auto param = R3rCreateVertexInputParam{};
	param.index_buffer = index_buffer.get();

	auto& descriptions = param.attrib_descrs;
	descriptions.reserve(3);

	add_vertex_input_attrib_descr<TVertex, VertexAttribLocationId::position>(
		R3rVertexAttribFormat::rgb_32_sfloat,
		cgm::Vec4F{},
		vertex_buffer,
		descriptions);

	add_vertex_input_attrib_descr<TVertex, VertexAttribLocationId::color>(
		R3rVertexAttribFormat::rgba_8_unorm,
		cgm::Vec4F{1.0F, 1.0F, 1.0F, 1.0F},
		vertex_buffer,
		descriptions);

	add_vertex_input_attrib_descr<TVertex, VertexAttribLocationId::texture_coordinates>(
		R3rVertexAttribFormat::rg_32_sfloat,
		cgm::Vec4F{},
		vertex_buffer,
		descriptions);

	vertex_input = renderer_->create_vertex_input(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_player_direction() noexcept
{
	player_angle_rad_ = math::deg_to_rad(player->angle);
	view_direction_ = cgm::Vec2D{std::cos(player_angle_rad_), -std::sin(player_angle_rad_)};
	bs_view_direction_ = view_direction_;
}

void HwVideo::update_player_position() noexcept
{
	player_position_[0] = player->x;
	player_position_[1] = player->y;
}

void HwVideo::update_player_view_position() noexcept
{
	const auto pos = player_position_ - (view_direction_ * focallength);
	view_position_ = cgm::Vec3D{pos[0], pos[1], 0.5};
	bs_view_position_ = pos;
}

void HwVideo::update_player() noexcept
{
	update_player_direction();
	update_player_position();
	update_player_view_position();
}

void HwVideo::initialize_ui_buffer()
try {
	vid_initialize_ui_buffer();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_shader(R3rShaderUPtr& shader) noexcept
{
	shader = nullptr;
}

void HwVideo::create_shader(R3rShaderType type, R3rShaderUPtr& shader)
try {
	auto param = R3rShaderInitParam{};
	param.type = type;

	const auto renderer_type = renderer_->get_type();

	switch (type)
	{
		case R3rShaderType::fragment:
			param.source = HwShaderRegistry::get_fragment(renderer_type);
			break;

		case R3rShaderType::vertex:
			param.source = HwShaderRegistry::get_vertex(renderer_type);
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported shader type.");
	}

	shader = renderer_->create_shader(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_fragment_shader() noexcept
{
	destroy_shader(fragment_shader_);
}

void HwVideo::create_fragment_shader()
try {
	create_shader(R3rShaderType::fragment, fragment_shader_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_vertex_shader() noexcept
{
	destroy_shader(vertex_shader_);
}

void HwVideo::create_vertex_shader()
try {
	create_shader(R3rShaderType::vertex, vertex_shader_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_shader_stage() noexcept
{
	shader_stage_ = nullptr;
}

void HwVideo::create_shader_stage()
try {
	const R3rShaderStageInputBinding input_bindings[]
	{
		{0, HwShaderRegistry::get_a_position_name()},
		{1, HwShaderRegistry::get_a_color_name()},
		{2, HwShaderRegistry::get_a_tx_coords_name()},
	};

	auto param = R3rShaderStageInitParam{};
	param.fragment_shader = fragment_shader_.get();
	param.vertex_shader = vertex_shader_.get();
	param.input_bindings = make_span(input_bindings);

	shader_stage_ = renderer_->create_shader_stage(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<typename T>
void HwVideo::uninitialize_uniform(T*& var) noexcept
{
	var = nullptr;
}

void HwVideo::uninitialize_model_mat_uniform() noexcept
{
	uninitialize_uniform(model_mat_uniform_);
}

void HwVideo::initialize_uniform(
	const char* name,
	R3rShaderInt32Var*& var,
	InitializeInt32UniformTag) noexcept
{
	var = shader_stage_->find_int32_var(name);
}

void HwVideo::initialize_uniform(
	const char* name,
	R3rShaderFloat32Var*& var,
	InitializeFloat32UniformTag) noexcept
{
	var = shader_stage_->find_float32_var(name);
}

void HwVideo::initialize_uniform(
	const char* name,
	R3rShaderVec2Var*& var,
	InitializeVec2UniformTag) noexcept
{
	var = shader_stage_->find_vec2_var(name);
}

void HwVideo::initialize_uniform(
	const char* name,
	R3rShaderMat4Var*& var,
	InitializeMat4UniformTag) noexcept
{
	var = shader_stage_->find_mat4_var(name);
}

void HwVideo::initialize_uniform(
	const char* name,
	R3rShaderR2SamplerVar*& var,
	InitializeSampler2dUniformTag) noexcept
{
	var = shader_stage_->find_r2_sampler_var(name);
}

template<typename T>
void HwVideo::initialize_uniform(const char* name, T*& var)
try {
	using Tag = std::conditional_t<
		std::is_same<T, R3rShaderInt32Var>::value,
		InitializeInt32UniformTag,
		std::conditional_t<
			std::is_same<T, R3rShaderFloat32Var>::value,
			InitializeFloat32UniformTag,
			std::conditional_t<
				std::is_same<T, R3rShaderVec2Var>::value,
				InitializeVec2UniformTag,
				std::conditional_t<
					std::is_same<T, R3rShaderMat4Var>::value,
					InitializeMat4UniformTag,
					std::conditional_t<
						std::is_same<T, R3rShaderR2SamplerVar>::value,
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
		BSTONE_THROW_DYNAMIC_SOURCE(("Shader variable \"" + std::string{name} + "\" not found.").c_str());
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::initialize_model_mat_uniform()
try {
	initialize_uniform(HwShaderRegistry::get_u_model_mat_name(), model_mat_uniform_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_view_mat_uniform() noexcept
{
	uninitialize_uniform(view_mat_uniform_);
}

void HwVideo::initialize_view_mat_uniform()
try {
	initialize_uniform(HwShaderRegistry::get_u_view_mat_name(), view_mat_uniform_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_projection_mat_uniform() noexcept
{
	uninitialize_uniform(projection_mat_uniform_);
}

void HwVideo::initialize_projection_mat_uniform()
try {
	initialize_uniform(HwShaderRegistry::get_u_projection_mat_name(), projection_mat_uniform_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_sampler_uniform() noexcept
{
	uninitialize_uniform(sampler_uniform_);
}

void HwVideo::initialize_sampler_uniform()
try {
	sampler_var_.set_is_modified(true);
	initialize_uniform(HwShaderRegistry::get_u_sampler_name(), sampler_uniform_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_shading_mode_uniform() noexcept
{
	uninitialize_uniform(shading_mode_uniform_);
}

void HwVideo::initialize_shading_mode_uniform()
try {
	shading_mode_.set_is_modified(true);
	initialize_uniform(HwShaderRegistry::get_u_shading_mode_name(), shading_mode_uniform_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_shade_max_uniform() noexcept
{
	uninitialize_uniform(shade_max_uniform_);
}

void HwVideo::initialize_shade_max_uniform()
try {
	bs_shade_max_.set_is_modified(true);
	initialize_uniform(HwShaderRegistry::get_u_shade_max_name(), shade_max_uniform_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_normal_shade_uniform() noexcept
{
	uninitialize_uniform(normal_shade_uniform_);
}

void HwVideo::initialize_normal_shade_uniform()
try {
	bs_normal_shade_.set_is_modified(true);
	initialize_uniform(HwShaderRegistry::get_u_normal_shade_name(), normal_shade_uniform_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_height_numerator_uniform() noexcept
{
	uninitialize_uniform(height_numerator_uniform_);
}

void HwVideo::initialize_height_numerator_uniform()
try {
	bs_height_numerator_.set_is_modified(true);
	initialize_uniform(HwShaderRegistry::get_u_height_numerator_name(), height_numerator_uniform_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_extra_lighting_uniform() noexcept
{
	uninitialize_uniform(extra_lighting_uniform_);
}

void HwVideo::initialize_extra_lighting_uniform()
try {
	bs_lighting_.set_is_modified(true);
	initialize_uniform(HwShaderRegistry::get_u_extra_lighting_name(), extra_lighting_uniform_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_view_direction_uniform() noexcept
{
	uninitialize_uniform(view_direction_uniform_);
}

void HwVideo::initialize_view_direction_uniform()
try {
	bs_view_direction_.set_is_modified(true);
	initialize_uniform(HwShaderRegistry::get_u_view_direction_name(), view_direction_uniform_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_view_position_uniform() noexcept
{
	uninitialize_uniform(view_position_uniform_);
}

void HwVideo::initialize_view_position_uniform()
try {
	bs_view_position_.set_is_modified(true);
	initialize_uniform(HwShaderRegistry::get_u_view_position_name(), view_position_uniform_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_shading_uniforms() noexcept
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

void HwVideo::initialize_shading_uniforms()
try {
	initialize_sampler_uniform();
	initialize_shading_mode_uniform();
	initialize_shade_max_uniform();
	initialize_normal_shade_uniform();
	initialize_height_numerator_uniform();
	initialize_extra_lighting_uniform();
	initialize_view_direction_uniform();
	initialize_view_position_uniform();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_uniforms() noexcept
{
	uninitialize_model_mat_uniform();
	uninitialize_view_mat_uniform();
	uninitialize_projection_mat_uniform();
	uninitialize_shading_uniforms();
}

void HwVideo::initialize_uniforms()
try {
	initialize_model_mat_uniform();
	initialize_view_mat_uniform();
	initialize_projection_mat_uniform();
	initialize_shading_uniforms();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_program() noexcept
{
	uninitialize_uniforms();
	destroy_shader_stage();
	destroy_fragment_shader();
	destroy_vertex_shader();
}

void HwVideo::initialize_program()
try {
	create_fragment_shader();
	create_vertex_shader();
	create_shader_stage();
	initialize_uniforms();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rType HwVideo::get_renderer_type(RendererType renderer_type)
try {
	switch (renderer_type)
	{
		case RendererType::gl_2_0: return R3rType::gl_2_0;
		case RendererType::gl_3_2_core: return R3rType::gl_3_2_core;
		case RendererType::gles_2_0: return R3rType::gles_2_0;
		default: BSTONE_THROW_STATIC_SOURCE("Unsupported renderer type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::log_device_features()
try {
	const auto& device_features = renderer_->get_device_features();

	log("");
	log("Device features");
	log("===============");
	log("V-Sync: " + vid_to_string(device_features.is_vsync_available));
	log("V-Sync requires restart: " + vid_to_string(device_features.is_vsync_requires_restart));
	log("Texture max dimension: " + vid_to_string(device_features.max_texture_dimension));
	log("Viewport max width: " + vid_to_string(device_features.max_viewport_width));
	log("Viewport max height: " + vid_to_string(device_features.max_viewport_height));
	log("Anisotropy: " + vid_to_string(device_features.is_anisotropy_available));
	log("Anisotropy max degree: " + vid_to_string(device_features.max_anisotropy_degree));
	log("Non-power-of-two textures: " + vid_to_string(device_features.is_npot_available));
	log("Mipmap auto-generation: " + vid_to_string(device_features.is_mipmap_available));
	log("Samplers: " + vid_to_string(device_features.is_mipmap_available));
	log("MSAA: " + vid_to_string(device_features.is_msaa_available));
	log("MSAA (render-to-window): " + vid_to_string(device_features.is_msaa_render_to_window));
	log("MSAA requires restart: " + vid_to_string(device_features.is_msaa_requires_restart));
	log("MSAA max degree: " + vid_to_string(device_features.max_msaa_degree));
	log("Vertex input max locations: " + vid_to_string(device_features.max_vertex_input_locations));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::initialize_renderer()
try {
	log("");
	log("Initializing 3D renderer.");

	const auto title = vid_get_game_name_and_game_version_string();

	// Initialization parameter.
	//
	auto param = R3rInitParam{};
	param.aa_type = vid_cfg_get_aa_type();
	param.aa_value = vid_cfg_get_aa_degree();
	param.is_vsync = vid_cfg_is_vsync();

	using RendererTypeList = std::vector<R3rType>;
	auto renderer_type_list = RendererTypeList{};

	const auto is_auto_detect = (vid_cfg_get_renderer_type() == RendererType::auto_detect);

	if (is_auto_detect)
	{
#ifndef BSTONE_R3R_TEST_NO_GL
		renderer_type_list =
		{
#ifndef BSTONE_R3R_TEST_NO_GL_3_2_C
			R3rType::gl_3_2_core,
#endif
#ifndef BSTONE_R3R_TEST_NO_GL_2_0
			R3rType::gl_2_0,
#endif
#ifndef BSTONE_R3R_TEST_NO_GLES_2_0
			R3rType::gles_2_0,
#endif
		};
#endif
	}
	else
	{
		renderer_type_list = {get_renderer_type(vid_cfg_get_renderer_type())};
	}

	for (const auto renderer_type : renderer_type_list)
	{
		param.renderer_type = renderer_type;

		log("Trying to initialize \"" + vid_to_string(renderer_type) + "\".");

		try
		{
			renderer_ = renderer_mgr_->renderer_initialize(param);
			break;
		}
		catch (...)
		{
			const auto messages = extract_exception_messages();

			for (const auto& message : messages)
			{
				log_error(message);
			}
		}
	}

	if (renderer_ == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Not found any 3D renderer.");
	}

// TODO
#ifdef __vita__
	param.window_.is_visible = true;
#endif // __vita__

	auto& window = renderer_->get_window();

	auto window_param = R3rUtilsSetWindowModeParam{};
	window_param.is_native = vid_is_native_mode();
	window_param.is_positioned = vid_cfg_is_positioned();
	window_param.position.x = sys::WindowOffset{vid_cfg_get_x()};
	window_param.position.y = sys::WindowOffset{vid_cfg_get_y()};
	window_param.size.width = vid_layout_.window_width;
	window_param.size.height = vid_layout_.window_height;
	R3rUtils::set_window_mode(window, window_param);

	renderer_->handle_resize(window_param.size);

	window.set_title(title.c_str());
	window.show(true);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_ui_ib() noexcept
{
	destroy_index_buffer(ui_ib_);
}

void HwVideo::create_ui_ib()
try {
	ui_ib_ = create_index_buffer(R3rBufferUsageType::draw_static, 1, r2_index_count_);

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

	update_index_buffer(ui_ib_, 0, r2_index_count_, indices.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_ui_vi() noexcept
{
	destroy_vertex_input(ui_vi_);
}

void HwVideo::create_ui_vi()
try {
	create_vertex_input<Vertex>(ui_ib_, ui_vb_, ui_vi_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::fill_2d_vb(float left_f, float right_f, float width_f, int vertex_offset) noexcept
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

void HwVideo::fill_2d_vb_stretched() noexcept
{
	const auto left_f = 0.0F;
	const auto right_f = static_cast<float>(vid_layout_.screen_width);
	const auto width_f = static_cast<float>(vid_layout_.screen_width);

	fill_2d_vb(left_f, right_f, width_f, r2_stretched_vertex_offset_);
}

void HwVideo::fill_2d_vb_non_stretched() noexcept
{
	const auto left_f = static_cast<float>(vid_layout_.screen_left_filler_width);
	const auto right_f = static_cast<float>(vid_layout_.screen_width - vid_layout_.screen_right_filler_width);
	const auto width_f = static_cast<float>(vid_layout_.screen_width_4x3);

	fill_2d_vb(left_f, right_f, width_f, r2_non_stretched_vertex_offset_);
}

void HwVideo::destroy_ui_vb() noexcept
{
	destroy_vertex_buffer(ui_vb_);
}

void HwVideo::create_ui_vb()
try {
	ui_vb_ = create_vertex_buffer<Vertex>(R3rBufferUsageType::draw_static, r2_vertex_count_);
	fill_2d_vb_stretched();
	fill_2d_vb_non_stretched();
	update_vertex_buffer(ui_vb_, 0, r2_vertex_count_, r2_vertices_.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_2d_fillers_ib() noexcept
{
	destroy_index_buffer(r2_fillers_ib_);
}

void HwVideo::create_2d_fillers_ib()
try {
	r2_fillers_ib_ = create_index_buffer(R3rBufferUsageType::draw_static, 1, r2_fillers_index_count_);

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

	update_index_buffer(r2_fillers_ib_, 0, r2_fillers_index_count_, indices.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_2d_fillers_vb() noexcept
{
	destroy_vertex_buffer(r2_fillers_vb_);
}

void HwVideo::create_2d_fillers_vb()
try {
	r2_fillers_vb_ = create_vertex_buffer<Vertex>(R3rBufferUsageType::draw_static, r2_fillers_vertex_count_);

	const auto& filler_color = vga_color_to_rgba_8(
		vgapal[(vid_cfg_get_filler_color_index() * 3) + 0],
		vgapal[(vid_cfg_get_filler_color_index() * 3) + 1],
		vgapal[(vid_cfg_get_filler_color_index() * 3) + 2]);

	const auto left_left_f = static_cast<float>(0.0F);
	const auto left_right_f = static_cast<float>(vid_layout_.screen_left_filler_width);

	const auto right_left_f = static_cast<float>(
		vid_layout_.screen_width - vid_layout_.screen_right_filler_width);
	const auto right_right_f = static_cast<float>(vid_layout_.screen_width);

	const auto top_top_f = static_cast<float>(vid_layout_.screen_height);
	const auto top_bottom_f = static_cast<float>(
		vid_layout_.screen_height - vid_layout_.screen_top_filler_height);

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

	update_vertex_buffer(r2_fillers_vb_, 0, r2_fillers_vertex_count_, vertices.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_2d_fillers_vi() noexcept
{
	destroy_vertex_input(r2_fillers_vi_);
}

void HwVideo::create_2d_fillers_vi()
try {
	create_vertex_input<Vertex>(r2_fillers_ib_, r2_fillers_vb_, r2_fillers_vi_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_black_1x1_2d_texture() noexcept
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->try_destroy_solid_1x1(HwTextureMgrSolid1x1Id::black);
	}

	r2_black_t2d_1x1_ = nullptr;
}

void HwVideo::create_black_1x1_2d_texture()
try {
	texture_mgr_->create_solid_1x1(HwTextureMgrSolid1x1Id::black);
	r2_black_t2d_1x1_ = texture_mgr_->get_solid_1x1(HwTextureMgrSolid1x1Id::black);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_white_1x1_2d_texture() noexcept
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->try_destroy_solid_1x1(HwTextureMgrSolid1x1Id::white);
	}

	r2_white_t2d_1x1_ = nullptr;
}

void HwVideo::create_white_1x1_2d_texture()
try {
	texture_mgr_->create_solid_1x1(HwTextureMgrSolid1x1Id::white);
	r2_white_t2d_1x1_ = texture_mgr_->get_solid_1x1(HwTextureMgrSolid1x1Id::white);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_fade_1x1_2d_texture() noexcept
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->try_destroy_solid_1x1(HwTextureMgrSolid1x1Id::fade_2d);
	}

	r2_fade_t2d_ = nullptr;
}

void HwVideo::create_fade_1x1_2d_texture()
try {
	texture_mgr_->create_solid_1x1(HwTextureMgrSolid1x1Id::fade_2d);
	r2_fade_t2d_ = texture_mgr_->get_solid_1x1(HwTextureMgrSolid1x1Id::fade_2d);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_ui_texture() noexcept
{
	if (ui_t2d_ == nullptr)
	{
		return;
	}

	texture_mgr_->destroy_ui();
	ui_t2d_ = nullptr;
}

void HwVideo::create_ui_texture()
try {
	texture_mgr_->create_ui(vid_ui_buffer_.data(), vid_mask_buffer_.data(), &palette_);
	ui_t2d_ = texture_mgr_->get_ui();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_2d() noexcept
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

void HwVideo::initialize_2d()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_flooring_ib() noexcept
{
	destroy_index_buffer(flooring_ib_);
}

void HwVideo::create_flooring_ib()
try {
	const auto index_count = 6;

	flooring_ib_ = create_index_buffer(R3rBufferUsageType::draw_static, 1, index_count);

	using Indices = std::array<std::uint8_t, index_count>;

	const auto& indices = Indices
	{
		0, 1, 2,
		0, 2, 3,
	};

	update_index_buffer(flooring_ib_, 0, index_count, indices.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_flooring_vb() noexcept
{
	destroy_vertex_buffer(flooring_vb_);
}

void HwVideo::create_flooring_vb()
try {
	const auto vertex_count = 4;

	flooring_vb_ = create_vertex_buffer<FlooringVertex>(
		R3rBufferUsageType::draw_static,
		vertex_count);

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

	update_vertex_buffer(flooring_vb_, 0, vertex_count, vertices.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_flooring_vi() noexcept
{
	destroy_vertex_input(flooring_vi_);
}

void HwVideo::create_flooring_vi()
try {
	create_vertex_input<FlooringVertex>(flooring_ib_, flooring_vb_, flooring_vi_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_solid_flooring_r2_texture() noexcept
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->try_destroy_solid_1x1(HwTextureMgrSolid1x1Id::flooring);
	}

	flooring_solid_t2d_ = nullptr;
}

void HwVideo::create_solid_flooring_r2_texture()
try {
	texture_mgr_->create_solid_1x1(HwTextureMgrSolid1x1Id::flooring);
	flooring_solid_t2d_ = texture_mgr_->get_solid_1x1(HwTextureMgrSolid1x1Id::flooring);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_flooring() noexcept
{
	destroy_flooring_vi();
	destroy_flooring_ib();
	destroy_flooring_vb();

	destroy_solid_flooring_r2_texture();

	// Managed by texture manager. No need to destroy.
	flooring_textured_t2d_ = nullptr;
}

void HwVideo::initialize_flooring()
try {
	create_flooring_ib();
	create_flooring_vb();
	create_flooring_vi();
	create_solid_flooring_r2_texture();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_ceiling_ib() noexcept
{
	destroy_index_buffer(ceiling_ib_);
}

void HwVideo::create_ceiling_ib()
try {
	const auto index_count = 6;
	ceiling_ib_ = create_index_buffer(R3rBufferUsageType::draw_static, 1, index_count);

	using Indices = std::array<std::uint8_t, index_count>;

	const auto& indices = Indices
	{
		0, 2, 1,
		0, 3, 2,
	};

	update_index_buffer(ceiling_ib_, 0, index_count, indices.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_ceiling_vb() noexcept
{
	destroy_vertex_buffer(ceiling_vb_);
}

void HwVideo::create_ceiling_vb()
try {
	const auto vertex_count = 4;

	ceiling_vb_ = create_vertex_buffer<CeilingVertex>(R3rBufferUsageType::draw_static, vertex_count);

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

	update_vertex_buffer(ceiling_vb_, 0, vertex_count, vertices.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_ceiling_vi() noexcept
{
	destroy_vertex_input(ceiling_vi_);
}

void HwVideo::create_ceiling_vi()
try {
	create_vertex_input<CeilingVertex>(ceiling_ib_, ceiling_vb_, ceiling_vi_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_solid_ceiling_r2_texture() noexcept
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->try_destroy_solid_1x1(HwTextureMgrSolid1x1Id::ceiling);
	}

	ceiling_solid_t2d_ = nullptr;
}

void HwVideo::create_solid_ceiling_r2_texture()
try {
	texture_mgr_->create_solid_1x1(HwTextureMgrSolid1x1Id::ceiling);
	ceiling_solid_t2d_ = texture_mgr_->get_solid_1x1(HwTextureMgrSolid1x1Id::ceiling);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::initialize_ceiling()
try {
	create_ceiling_ib();
	create_ceiling_vb();
	create_ceiling_vi();
	create_solid_ceiling_r2_texture();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_ceiling() noexcept
{
	destroy_ceiling_vi();
	destroy_ceiling_ib();
	destroy_ceiling_vb();

	destroy_solid_ceiling_r2_texture();

	// Managed by texture manager. No need to destroy.
	ceiling_textured_t2d_ = nullptr;
}

void HwVideo::create_walls_ib()
try {
	const auto index_count = wall_side_count_ * indices_per_wall_side;

	wall_sides_ib_ = create_index_buffer(R3rBufferUsageType::draw_streaming, 2, index_count);
	wall_sides_ibi_.clear();
	wall_sides_ibi_.resize(index_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_walls_ib() noexcept
{
	destroy_index_buffer(wall_sides_ib_);
	wall_sides_ibi_.clear();
}

void HwVideo::create_walls_vb()
try {
	const auto vertex_count = wall_side_count_ * vertices_per_wall_side;
	wall_sides_vb_ = create_vertex_buffer<WallVertex>(R3rBufferUsageType::draw_static, vertex_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_walls_vi() noexcept
{
	destroy_vertex_input(wall_sides_vi_);
}

void HwVideo::create_walls_vi()
try {
	create_vertex_input<WallVertex>(wall_sides_ib_, wall_sides_vb_, wall_sides_vi_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_walls_vb() noexcept
{
	destroy_vertex_buffer(wall_sides_vb_);
}

void HwVideo::initialize_walls()
try {
	xy_wall_map_.reserve(wall_count_);

	walls_to_render_.clear();

	wall_side_draw_item_count_ = 0;
	wall_side_draw_items_.clear();
	wall_side_draw_items_.resize(wall_side_count_);

	create_walls_ib();
	create_walls_vb();
	create_walls_vi();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_walls() noexcept
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

void HwVideo::create_pushwalls_ibi()
try {
	const auto index_count = pushwall_side_count_ * indices_per_wall_side;

	pushwall_sides_ibi_.clear();
	pushwall_sides_ibi_.resize(index_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::create_pushwalls_ib()
try {
	const auto index_count = pushwall_side_count_ * indices_per_wall_side;
	pushwall_sides_ib_ = create_index_buffer(R3rBufferUsageType::draw_streaming, 2, index_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_pushwalls_ibi() noexcept
{
	pushwall_sides_ibi_.clear();
}

void HwVideo::destroy_pushwalls_ib() noexcept
{
	destroy_index_buffer(pushwall_sides_ib_);
}

void HwVideo::create_pushwalls_vb()
try {
	const auto vertex_count = pushwall_side_count_ * vertices_per_wall_side;

	pushwall_sides_vb_ = create_vertex_buffer<PushwallVertex>(
		R3rBufferUsageType::draw_dynamic,
		vertex_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_pushwalls_vi() noexcept
{
	destroy_vertex_input(pushwall_sides_vi_);
}

void HwVideo::create_pushwalls_vi()
try {
	create_vertex_input<PushwallVertex>(pushwall_sides_ib_, pushwall_sides_vb_, pushwall_sides_vi_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_pushwalls_vb() noexcept
{
	destroy_vertex_buffer(pushwall_sides_vb_);
}

void HwVideo::initialize_pushwalls()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_pushwalls() noexcept
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

void HwVideo::create_door_sides_ibi()
try {
	const auto index_count = door_count_ * indices_per_door_side;

	door_sides_ibi_.clear();
	door_sides_ibi_.resize(index_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::create_door_sides_ib()
try {
	const auto index_count = door_count_ * indices_per_door_side;
	door_sides_ib_ = create_index_buffer(R3rBufferUsageType::draw_streaming, 2, index_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_door_sides_ibi() noexcept
{
	door_sides_ibi_.clear();
}

void HwVideo::destroy_door_sides_ib() noexcept
{
	destroy_index_buffer(door_sides_ib_);
	door_sides_ibi_.clear();
}

void HwVideo::create_door_sides_vb()
try {
	const auto vertex_count = door_count_ * indices_per_door_side;
	door_sides_vb_ = create_vertex_buffer<DoorVertex>(R3rBufferUsageType::draw_dynamic, vertex_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_door_sides_vb() noexcept
{
	destroy_vertex_buffer(door_sides_vb_);
}

void HwVideo::destroy_door_sides_vi() noexcept
{
	destroy_vertex_input(door_sides_vi_);
}

void HwVideo::create_door_sides_vi()
try {
	create_vertex_input<DoorVertex>(door_sides_ib_, door_sides_vb_, door_sides_vi_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::initialize_door_sides()
try {
	xy_door_map_.reserve(door_count_);

	const auto max_draw_item_count = door_count_ * door_halves_per_door;

	door_draw_item_count_ = 0;
	door_draw_items_.clear();
	door_draw_items_.resize(max_draw_item_count);

	create_door_sides_ibi();
	create_door_sides_ib();
	create_door_sides_vb();
	create_door_sides_vi();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_door_sides() noexcept
{
	xy_door_map_.clear();

	door_draw_item_count_ = 0;
	door_draw_items_.clear();

	destroy_door_sides_vi();

	destroy_door_sides_ibi();
	destroy_door_sides_ib();

	destroy_door_sides_vb();
}

void HwVideo::update_palette_from_vga(int offset, int count) noexcept
{
	for (auto i = 0; i < count; ++i)
	{
		const auto& vga_color = vga_palette_[offset + i];
		auto& hw_color = palette_[offset + i];
		hw_color = vga_color_to_rgba_8(vga_color[0], vga_color[1], vga_color[2]);
	}
}

void HwVideo::initialize_palette() noexcept
{
	vga_palette_ = {};
	palette_ = {};
	default_palette_ = {};

	for (int i = 0; i < RgbPalette::get_max_color_count(); ++i)
	{
		const auto vga_color = vgapal + (i * 3);
		auto& color = default_palette_[i];
		color = vga_color_to_rgba_8(vga_color[0], vga_color[1], vga_color[2]);
	}
}

void HwVideo::calculate_dimensions() noexcept
{
	auto src_param = vid_create_screen_size_param();
	vid_calculate_window_elements_dimensions(src_param, vid_layout_);
	vid_calculate_vga_dimensions();
}

void HwVideo::build_2d_model_matrix() noexcept
{
	r2_matrix_model_ = cgm::Mat4D::get_identity();
}

void HwVideo::build_2d_view_matrix() noexcept
{
	r2_matrix_view_ = cgm::Mat4D::get_identity();
}

void HwVideo::build_2d_projection_matrix() noexcept
{
	r2_matrix_projection_ = cgm::make_ortho_rh_n1p1(
		0.0, // left
		static_cast<double>(vid_layout_.screen_width), // right
		0.0, // bottom
		static_cast<double>(vid_layout_.screen_height), // top
		0.0, // zNear
		1.0 // zFar
	);
}

void HwVideo::build_2d_matrices() noexcept
{
	build_2d_model_matrix();
	build_2d_view_matrix();
	build_2d_projection_matrix();
}

void HwVideo::calculate_camera_parameters() noexcept
{
	// Vertical FOV.
	//
	// V = 2 * arctan(tan(H / 2) * (h / w))
	//

	const auto ref_r_ratio =
		static_cast<double>(vga_ref_height_4x3) / static_cast<double>(vga_ref_width);

	constexpr auto half_hfov_deg = ref_camera_hfov_deg / 2.0;
	constexpr auto half_hfov_rad = math::deg_to_rad(half_hfov_deg);
	const auto tan_half_hfov_rad = std::tan(half_hfov_rad);
	const auto half_vfov_rad = tan_half_hfov_rad * ref_r_ratio;

	// Radians.
	const auto vfov_rad = 2.0 * half_vfov_rad;
	camera_vfov_rad_ = vfov_rad;

	// Degrees.
	const auto half_vfov_deg = math::rad_to_deg(half_vfov_rad);
	const auto vfov_deg = 2.0 * half_vfov_deg;
	camera_vfov_deg_ = vfov_deg;


	// Distances to the planes.
	//
	camera_near_distance_ = 0.05;
	camera_far_distance_ = (std::sqrt(2.0) * map_dimension_d) + 0.5;
}

void HwVideo::build_bs_to_ren_matrix() noexcept
{
	//
	// |  0 y   0   0 |
	// |  0 0 z*1.2 0 |
	// | -x 0   0   0 |
	// |  0 0   0   1 |
	//

	const auto m_11 = 0.0;
	const auto m_12 = 1.0;
	const auto m_13 = 0.0;
	const auto m_14 = 0.0;

	const auto m_21 = 0.0;
	const auto m_22 = 0.0;
	const auto m_23 = height_compensation_factor;
	const auto m_24 = 0.0;

	const auto m_31 = -1.0;
	const auto m_32 = 0.0;
	const auto m_33 = 0.0;
	const auto m_34 = 0.0;

	const auto m_41 = 0.0;
	const auto m_42 = 0.0;
	const auto m_43 = 0.0;
	const auto m_44 = 1.0;

	r3_matrix_bs_to_r_ = cgm::Mat4D
	{
		m_11, m_21, m_31, m_41,
		m_12, m_22, m_32, m_42,
		m_13, m_23, m_33, m_43,
		m_14, m_24, m_34, m_44,
	};
}

void HwVideo::build_model_matrix() noexcept
{
	r3_matrix_model_ = cgm::Mat4D::get_identity();
}

void HwVideo::build_view_matrix() noexcept
{
	if (player == nullptr)
	{
		r3_matrix_view_ = cgm::Mat4D::get_identity();
		return;
	}

	auto view_matrix = cgm::Mat4D::get_identity();
	view_matrix = cgm::rotate(view_matrix, player_angle_rad_, cgm::Vec3D{0.0, 0.0, 1.0});
	view_matrix = cgm::translate(view_matrix, -view_position_);
	r3_matrix_view_ = view_matrix;
}

void HwVideo::build_projection_matrix() noexcept
{
	const auto perspective = cgm::make_perspective_vfov_rh_n1p1(
		camera_vfov_rad_,
		static_cast<double>(vid_layout_.screen_viewport_width),
		static_cast<double>(vid_layout_.screen_viewport_height),
		camera_near_distance_,
		camera_far_distance_);

	r3_matrix_projection_ = perspective * r3_matrix_bs_to_r_;
}

void HwVideo::build_3d_matrices() noexcept
{
	calculate_camera_parameters();

	build_bs_to_ren_matrix();
	build_model_matrix();
	build_view_matrix();
	build_projection_matrix();
}

void HwVideo::build_matrices() noexcept
{
	build_2d_matrices();
	build_3d_matrices();
}

void HwVideo::set_ui_sampler_default_state() noexcept
{
	ui_sampler_state_.min_filter = R3rFilterType::nearest;
	ui_sampler_state_.mag_filter = R3rFilterType::nearest;
	ui_sampler_state_.mipmap_mode = R3rMipmapMode::none;
	ui_sampler_state_.address_mode_u = R3rAddressMode::clamp;
	ui_sampler_state_.address_mode_v = R3rAddressMode::clamp;
	ui_sampler_state_.anisotropy = R3rLimits::min_anisotropy_off();
}

void HwVideo::update_ui_sampler_state() noexcept
{
	ui_sampler_state_.min_filter = vid_cfg_get_2d_texture_filter();
	ui_sampler_state_.mag_filter = vid_cfg_get_2d_texture_filter();
}

void HwVideo::update_ui_sampler()
try {
	update_ui_sampler_state();

	if (ui_sampler_ != nullptr)
	{
		auto param = R3rSamplerUpdateParam{};
		param.state = ui_sampler_state_;
		ui_sampler_->update(param);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_ui_sampler() noexcept
{
	ui_sampler_ = nullptr;
}

void HwVideo::create_ui_sampler()
try {
	update_ui_sampler_state();

	auto param = R3rSamplerInitParam{};
	param.state = ui_sampler_state_;

	ui_sampler_ = renderer_->create_sampler(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::set_sprite_sampler_default_state() noexcept
{
	sprite_sampler_state_.min_filter = R3rFilterType::nearest;
	sprite_sampler_state_.mag_filter = R3rFilterType::nearest;
	sprite_sampler_state_.mipmap_mode = R3rMipmapMode::nearest;
	sprite_sampler_state_.address_mode_u = R3rAddressMode::clamp;
	sprite_sampler_state_.address_mode_v = R3rAddressMode::clamp;
	sprite_sampler_state_.anisotropy = R3rLimits::min_anisotropy_off();
}

void HwVideo::update_sprite_sampler_state()
try {
	sprite_sampler_state_.min_filter = vid_cfg_get_3d_texture_image_filter();
	sprite_sampler_state_.mag_filter = vid_cfg_get_3d_texture_image_filter();

	sprite_sampler_state_.mipmap_mode = cfg_texture_mipmap_filter_to_renderer(
		vid_cfg_get_3d_texture_mipmap_filter());

	sprite_sampler_state_.anisotropy = cfg_texture_anisotropy_to_renderer(
		vid_cfg_get_3d_texture_anisotropy());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_sprite_sampler()
try {
	update_sprite_sampler_state();

	if (sprite_sampler_ != nullptr)
	{
		auto param = R3rSamplerUpdateParam{};
		param.state = sprite_sampler_state_;
		sprite_sampler_->update(param);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_sprite_sampler() noexcept
{
	destroy_sampler(sprite_sampler_);
}

void HwVideo::create_sprite_sampler()
try {
	update_sprite_sampler_state();

	auto param = R3rSamplerInitParam{};
	param.state = sprite_sampler_state_;

	sprite_sampler_ = renderer_->create_sampler(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::set_wall_sampler_default_state() noexcept
{
	wall_sampler_state_.min_filter = R3rFilterType::nearest;
	wall_sampler_state_.mag_filter = R3rFilterType::nearest;
	wall_sampler_state_.mipmap_mode = R3rMipmapMode::nearest;
	wall_sampler_state_.address_mode_u = R3rAddressMode::repeat;
	wall_sampler_state_.address_mode_v = R3rAddressMode::repeat;
	wall_sampler_state_.anisotropy = R3rLimits::min_anisotropy_off();
}

void HwVideo::update_wall_sampler_state()
try {
	wall_sampler_state_.min_filter = vid_cfg_get_3d_texture_image_filter();
	wall_sampler_state_.mag_filter = vid_cfg_get_3d_texture_image_filter();

	wall_sampler_state_.mipmap_mode = cfg_texture_mipmap_filter_to_renderer(
		vid_cfg_get_3d_texture_mipmap_filter());

	wall_sampler_state_.anisotropy = cfg_texture_anisotropy_to_renderer(
		vid_cfg_get_3d_texture_anisotropy());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_wall_sampler()
try {
	update_wall_sampler_state();

	if (wall_sampler_ != nullptr)
	{
		auto param = R3rSamplerUpdateParam{};
		param.state = wall_sampler_state_;
		wall_sampler_->update(param);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_wall_sampler() noexcept
{
	destroy_sampler(wall_sampler_);
}

void HwVideo::create_wall_sampler()
try {
	update_wall_sampler_state();

	auto param = R3rSamplerInitParam{};
	param.state = wall_sampler_state_;

	wall_sampler_ = renderer_->create_sampler(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_player_weapon_vb()
try {
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
	update_vertex_buffer(player_weapon_vb_, 0, vertices_per_sprite, vertices.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_player_weapon_ib() noexcept
{
	destroy_index_buffer(player_weapon_ib_);
}

void HwVideo::create_player_weapon_ib()
try {
	player_weapon_ib_ = create_index_buffer(R3rBufferUsageType::draw_static, 1, indices_per_sprite);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_player_weapon_ib()
try {
	using Indices = std::array<std::uint8_t, indices_per_sprite>;

	auto indices = Indices
	{
		0, 1, 2,
		0, 2, 3,
	}; // indices

	update_index_buffer(player_weapon_ib_, 0, indices_per_sprite, indices.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_player_weapon_vb() noexcept
{
	destroy_vertex_buffer(player_weapon_vb_);
}

void HwVideo::create_player_weapon_vb()
try {
	player_weapon_vb_ = create_vertex_buffer<PlayerWeaponVertex>(
		R3rBufferUsageType::draw_static,
		vertices_per_sprite);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_player_weapon_vi() noexcept
{
	destroy_vertex_input(player_weapon_vi_);
}

void HwVideo::create_player_weapon_vi()
try {
	create_vertex_input<PlayerWeaponVertex>(player_weapon_ib_, player_weapon_vb_, player_weapon_vi_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_player_weapon_model_matrix()
try {
	const auto& assets_info = get_assets_info();

	const auto aog_scale = 128.0 / 64.0;
	const auto ps_scale = 88.0 / 64.0;

	const auto game_scalar = (assets_info.is_ps() ? ps_scale : aog_scale);
	const auto scalar = game_scalar * vga_height_scale;

	const auto translate_x = 0.5 * static_cast<double>(vid_layout_.screen_viewport_width);

	const auto is_bobbing_enabled = (!gp_no_weapon_bobbing() && assets_info.is_ps());
	const auto bounce_offset = (is_bobbing_enabled ? -player_get_weapon_bounce_offset() : 0.0);
	const auto translate_y = vga_height_scale * bounce_offset;

	const auto translate_v = cgm::Vec3D
	{
		translate_x,
		height_compensation_factor * translate_y,
		0.0F
	};

	const auto identity = cgm::Mat4D::get_identity();
	const auto translate = cgm::translate(identity, translate_v);
	const auto scale = cgm::scale(identity, cgm::Vec3D{scalar, height_compensation_factor * scalar, 0.0F});
	player_weapon_model_matrix_ = translate * scale;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_player_weapon_view_matrix() noexcept
{
	player_weapon_view_matrix_ = cgm::Mat4D::get_identity();
}

void HwVideo::build_player_weapon_projection_matrix() noexcept
{
	const auto ortho = cgm::make_ortho_rh_n1p1(
		0.0, // left
		static_cast<double>(vid_layout_.screen_viewport_width), // right
		0.0, // bottom
		static_cast<double>(vid_layout_.screen_viewport_height), // top
		0.0, // zNear
		1.0 // zFar
	);

	player_weapon_projection_matrix_ = ortho;
}

void HwVideo::set_player_weapon_sampler_default_state() noexcept
{
	player_weapon_sampler_state_.min_filter = R3rFilterType::nearest;
	player_weapon_sampler_state_.mag_filter = R3rFilterType::nearest;
	player_weapon_sampler_state_.mipmap_mode = R3rMipmapMode::none;
	player_weapon_sampler_state_.address_mode_u = R3rAddressMode::clamp;
	player_weapon_sampler_state_.address_mode_v = R3rAddressMode::clamp;
	player_weapon_sampler_state_.anisotropy = R3rLimits::min_anisotropy_off();
}

void HwVideo::update_player_weapon_sampler_state() noexcept
{
	player_weapon_sampler_state_.min_filter = vid_cfg_get_3d_texture_image_filter();
	player_weapon_sampler_state_.mag_filter = vid_cfg_get_3d_texture_image_filter();
}

void HwVideo::update_player_weapon_sampler()
try {
	update_player_weapon_sampler_state();

	if (player_weapon_sampler_ != nullptr)
	{
		auto param = R3rSamplerUpdateParam{};
		param.state = player_weapon_sampler_state_;
		player_weapon_sampler_->update(param);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_player_weapon_sampler() noexcept
{
	player_weapon_sampler_ = nullptr;
}

void HwVideo::create_player_weapon_sampler()
try {
	update_player_weapon_sampler_state();

	auto param = R3rSamplerInitParam{};
	param.state = player_weapon_sampler_state_;
	player_weapon_sampler_ = renderer_->create_sampler(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_player_weapon() noexcept
{
	destroy_player_weapon_vi();
	destroy_player_weapon_ib();
	destroy_player_weapon_vb();
	destroy_player_weapon_sampler();
}

void HwVideo::initialize_player_weapon()
try {
	create_player_weapon_ib();
	create_player_weapon_vb();
	create_player_weapon_vi();
	create_player_weapon_sampler();

	update_player_weapon_ib();
	update_player_weapon_vb();

	update_player_weapon_model_matrix();
	update_player_weapon_view_matrix();
	build_player_weapon_projection_matrix();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_fade_sampler() noexcept
{
	destroy_sampler(fade_sampler_);
}

void HwVideo::create_fade_sampler()
try {
	auto param = R3rSamplerInitParam{};
	param.state.min_filter = R3rFilterType::nearest;
	param.state.mag_filter = R3rFilterType::nearest;
	param.state.mipmap_mode = R3rMipmapMode::none;
	param.state.address_mode_u = R3rAddressMode::repeat;
	param.state.address_mode_v = R3rAddressMode::repeat;
	param.state.anisotropy = R3rLimits::min_anisotropy_off();
	fade_sampler_ = renderer_->create_sampler(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::set_samplers_default_states() noexcept
{
	set_ui_sampler_default_state();
	set_sprite_sampler_default_state();
	set_wall_sampler_default_state();
}

void HwVideo::uninitialize_samplers() noexcept
{
	destroy_ui_sampler();
	destroy_sprite_sampler();
	destroy_wall_sampler();
	destroy_fade_sampler();
}

void HwVideo::initialize_samplers()
try {
	create_ui_sampler();
	create_sprite_sampler();
	create_wall_sampler();
	create_fade_sampler();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_command_buffers() noexcept
{
	command_buffers_.fill(nullptr);
}

void HwVideo::create_command_buffers() noexcept
{
	command_buffers_[0] = common_command_buffer_.get();
	command_buffers_[1] = r3_command_buffer_.get();
	command_buffers_[2] = r2_command_buffer_.get();
}

void HwVideo::destroy_common_command_buffer() noexcept
{
	common_command_buffer_ = nullptr;
}

void HwVideo::create_vsync_command_buffer()
try {
	auto param = R3rCmdBufferInitParam{};
	param.initial_size = 32;
	param.resize_delta = 0;
	vsync_command_buffer_ = make_r3r_cmd_buffer(param);

	vsync_command_buffer_->begin_write();
	auto& command = vsync_command_buffer_->write_clear();
	command.clear.color = sys::Color{};
	vsync_command_buffer_->end_write();

	vsync_command_buffer_->enable(true);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_vsync_command_buffer() noexcept
{
	vsync_command_buffer_ = nullptr;
}

void HwVideo::create_common_command_buffer()
try {
	auto param = R3rCmdBufferInitParam{};
	param.initial_size = common_command_buffer_initial_size;
	param.resize_delta = common_command_buffer_resize_delta_size;

	common_command_buffer_ = make_r3r_cmd_buffer(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_2d_command_buffer() noexcept
{
	r2_command_buffer_ = nullptr;
}

void HwVideo::create_2d_command_buffer()
try {
	auto param = R3rCmdBufferInitParam{};
	param.initial_size = r2_command_buffer_initial_size;
	param.resize_delta = r2_command_buffer_resize_delta_size;

	r2_command_buffer_ = make_r3r_cmd_buffer(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_3d_command_buffer() noexcept
{
	r3_command_buffer_ = nullptr;
}

void HwVideo::create_3d_command_buffer()
try {
	auto param = R3rCmdBufferInitParam{};
	param.initial_size = r3_command_buffer_initial_size;
	param.resize_delta = r3_command_buffer_resize_delta_size;

	r3_command_buffer_ = make_r3r_cmd_buffer(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_command_buffers() noexcept
{
	destroy_vsync_command_buffer();
	destroy_3d_command_buffer();
	destroy_2d_command_buffer();
	destroy_common_command_buffer();
	destroy_command_buffers();
}

void HwVideo::initialize_command_buffers()
try {
	create_vsync_command_buffer();
	create_common_command_buffer();
	create_3d_command_buffer();
	create_2d_command_buffer();
	create_command_buffers();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_3d_fade_ib() noexcept
{
	destroy_index_buffer(r3_fade_ib_);
}

void HwVideo::create_3d_fade_ib()
try {
	r3_fade_ib_ = create_index_buffer(R3rBufferUsageType::draw_static, 1, 6);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_3d_fade_vb() noexcept
{
	destroy_vertex_buffer(r3_fade_vb_);
}

void HwVideo::create_3d_fade_vb()
try {
	r3_fade_vb_ = create_vertex_buffer<FadeVertex>(R3rBufferUsageType::draw_static, 4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_3d_fade_vi() noexcept
{
	destroy_vertex_input(r3_fade_vi_);
}

void HwVideo::create_3d_fade_vi()
try {
	create_vertex_input<FadeVertex>(r3_fade_ib_, r3_fade_vb_, r3_fade_vi_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_3d_fade_ib()
try {
	using Indices = std::array<std::uint8_t, 6>;

	const auto& indices = Indices
	{
		0, 1, 2,
		0, 2, 3,
	};

	update_index_buffer(r3_fade_ib_, 0, 6, indices.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_3d_fade_vb()
try {
	auto vertices = FadeVbi{};
	vertices.resize(4);

	auto vertex_index = 0;

	const auto width_f = static_cast<float>(vid_layout_.screen_viewport_width);
	const auto height_f = static_cast<float>(vid_layout_.screen_viewport_height);

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

	update_vertex_buffer<FadeVertex>(r3_fade_vb_, 0, 4, vertices.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_3d_fade_r2_texture() noexcept
{
	if (texture_mgr_ != nullptr)
	{
		texture_mgr_->try_destroy_solid_1x1(HwTextureMgrSolid1x1Id::fade_3d);
	}

	r3_fade_t2d_ = nullptr;
}

void HwVideo::create_r3_fade_r2_texture()
try {
	texture_mgr_->create_solid_1x1(HwTextureMgrSolid1x1Id::fade_3d);
	r3_fade_t2d_ = texture_mgr_->get_solid_1x1(HwTextureMgrSolid1x1Id::fade_3d);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_3d_fade() noexcept
{
	destroy_3d_fade_vi();
	destroy_3d_fade_vb();
	destroy_3d_fade_ib();
	destroy_3d_fade_r2_texture();
}

void HwVideo::destroy_texture_manager() noexcept
{
	texture_mgr_ = nullptr;
}

void HwVideo::create_texture_manager()
try {
	texture_mgr_ = make_hw_texture_mgr(renderer_, &vid_sprite_cache, mt_task_manager_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::initialize_3d_fade()
try {
	create_3d_fade_ib();
	create_3d_fade_vb();
	create_3d_fade_vi();
	create_r3_fade_r2_texture();

	update_3d_fade_ib();
	update_3d_fade_vb();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::present_common()
try {
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
		const auto clear_color = sys::Color{};
		auto& clear = command_buffer->write_clear();
		clear.clear.color = clear_color;
	}

	// Set shader's 2D sampler.
	//
	if (sampler_var_.is_modified())
	{
		sampler_var_.set_is_modified(false);

		BSTONE_ASSERT(sampler_uniform_ != nullptr);
		auto& command = command_buffer->write_set_r2_sampler_uniform();
		command.var = sampler_uniform_;
		command.value = sampler_var_;
	}

	// Set shader stage.
	//
	{
		auto& shader_stage = command_buffer->write_set_shader_stage().shader_stage;
		shader_stage = shader_stage_.get();
	}

	// Set shading mode.
	//
	if (shading_mode_.is_modified())
	{
		shading_mode_.set_is_modified(false);

		auto& command = command_buffer->write_set_i32_uniform();
		command.var = shading_mode_uniform_;
		command.value = shading_mode_;
	}

	// Set shade_max.
	//
	if (bs_shade_max_.is_modified())
	{
		bs_shade_max_.set_is_modified(false);

		auto& command = command_buffer->write_set_f32_uniform();
		command.var = shade_max_uniform_;
		command.value = static_cast<float>(bs_shade_max_);
	}

	// Set normal_shade.
	//
	if (bs_normal_shade_.is_modified())
	{
		bs_normal_shade_.set_is_modified(false);

		auto& command = command_buffer->write_set_f32_uniform();
		command.var = normal_shade_uniform_;
		command.value = static_cast<float>(bs_normal_shade_);
	}

	// Set height_numerator.
	//
	if (bs_height_numerator_.is_modified())
	{
		bs_height_numerator_.set_is_modified(false);

		auto& command = command_buffer->write_set_f32_uniform();
		command.var = height_numerator_uniform_;
		command.value = static_cast<float>(bs_height_numerator_);
	}

	// Set extra_lighting.
	//
	if (bs_lighting_.is_modified())
	{
		bs_lighting_.set_is_modified(false);

		auto& command = command_buffer->write_set_f32_uniform();
		command.var = extra_lighting_uniform_;
		command.value = static_cast<float>(bs_lighting_);
	}

	// Set view_direction.
	//
	if (is_draw_3d_ && bs_view_direction_.is_modified())
	{
		bs_view_direction_.set_is_modified(false);

		auto& command = command_buffer->write_set_vec2_uniform();
		command.var = view_direction_uniform_;
		convert(bs_view_direction_, command.value);
	}

	// Set view_position.
	//
	if (is_draw_3d_ && bs_view_position_.is_modified())
	{
		bs_view_position_.set_is_modified(false);

		auto& command = command_buffer->write_set_vec2_uniform();
		command.var = view_position_uniform_;
		convert(bs_view_position_, command.value);
	}

	// Finalize.
	//
	command_buffer->end_write();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::present_2d()
try {
	// Update 2D texture.
	//
	texture_mgr_->update_ui();

	// Update fade color.
	//
	if (r2_fade_is_enabled_)
	{
		texture_mgr_->update_solid_1x1(HwTextureMgrSolid1x1Id::fade_2d, r2_fade_color_);
	}

	auto& command_buffer = r2_command_buffer_;
	command_buffer->enable(true);

	command_buffer->begin_write();

	// Build commands.
	//

	// Disable back-face culling.
	//
	{
		auto& command = command_buffer->write_enable_culling();
		command.is_enable = false;
	}

	// Disable depth test.
	//
	{
		auto& command = command_buffer->write_enable_depth_test();
		command.is_enable = false;
	}

	// Set viewport.
	//
	{
		auto& viewport = command_buffer->write_set_viewport().viewport;
		viewport.x = vid_layout_.window_viewport_left_width;
		viewport.y = vid_layout_.window_viewport_bottom_height;
		viewport.width = vid_layout_.screen_width;
		viewport.height = vid_layout_.screen_height;
		viewport.min_depth = 0.0F;
		viewport.max_depth = 0.0F;
	}

	// Set sampler.
	//
	{
		auto& command = command_buffer->write_set_sampler();
		command.sampler = ui_sampler_.get();
	}

	// Set model matrix.
	//
	{
		auto& command = command_buffer->write_set_mat4_uniform();
		command.var = model_mat_uniform_;
		convert(r2_matrix_model_, command.value);
	}

	// Set view matrix.
	//
	{
		auto& command = command_buffer->write_set_mat4_uniform();
		command.var = view_mat_uniform_;
		convert(r2_matrix_view_, command.value);
	}

	// Set projection matrix.
	//
	{
		auto& command = command_buffer->write_set_mat4_uniform();
		command.var = projection_mat_uniform_;
		convert(r2_matrix_projection_, command.value);
	}

	// Fillers.
	//
	if (!vid_cfg_is_ui_stretched())
	{
		{
			auto& command = command_buffer->write_set_texture();
			command.r2_texture = (vid_is_movie ? r2_black_t2d_1x1_ : r2_white_t2d_1x1_);
		}

		{
			auto& command = command_buffer->write_set_vertex_input();
			command.vertex_input = r2_fillers_vi_.get();
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

			auto& command = command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed;

			param.primitive_type = R3rPrimitiveType::triangle_list;
			param.vertex_count = count * vertices_per_quad;
			param.index_byte_depth = 1;
			param.index_buffer_offset = 0;
			param.index_offset = index_offset;
		}
	}

	// Draw 2D (UI, menu, etc.).
	//
	{
		if (vid_is_hud)
		{
			{
				auto& command = command_buffer->write_enable_blending();
				command.is_enable = true;
			}

			// Set blending function.
			//
			{
				auto& blending_func = command_buffer->write_set_blending_func().blending_func;
				blending_func.src_factor = R3rBlendingFactor::src_alpha;
				blending_func.dst_factor = R3rBlendingFactor::one_minus_src_alpha;
			}
		}

		{
			auto& command = command_buffer->write_set_texture();
			command.r2_texture = ui_t2d_;
		}

		{
			auto& command = command_buffer->write_set_vertex_input();
			command.vertex_input = ui_vi_.get();
		}

		{
			const auto index_offset = (vid_cfg_is_ui_stretched()
				?
				r2_stretched_index_offset_
				:
				r2_non_stretched_index_offset_
			);

			auto& command = command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed;

			param.primitive_type = R3rPrimitiveType::triangle_list;
			param.vertex_count = vertices_per_quad;
			param.index_byte_depth = 1;
			param.index_buffer_offset = 0;
			param.index_offset = index_offset;
		}

		if (vid_is_hud)
		{
			auto& command = command_buffer->write_enable_blending();
			command.is_enable = false;
		}
	}

	// 2D fade in or out.
	//
	if (r2_fade_is_enabled_)
	{
		// Enable blending.
		//
		{
			auto& command = command_buffer->write_enable_blending();
			command.is_enable = true;
		}

		// Set blending function.
		//
		{
			auto& blending_func = command_buffer->write_set_blending_func().blending_func;
			blending_func.src_factor = R3rBlendingFactor::src_alpha;
			blending_func.dst_factor = R3rBlendingFactor::one_minus_src_alpha;
		}

		// Set texture.
		//
		{
			auto& command = command_buffer->write_set_texture();
			command.r2_texture = r2_fade_t2d_;
		}

		// Set sampler.
		//
		{
			auto& command = command_buffer->write_set_sampler();
			command.sampler = fade_sampler_.get();
		}

		// Set vertex input.
		//
		{
			auto& command = command_buffer->write_set_vertex_input();
			command.vertex_input = ui_vi_.get();
		}

		// Draw the quad.
		//
		{
			const auto index_offset = (vid_cfg_is_ui_stretched() || is_draw_3d_
				?
				r2_stretched_index_offset_
				:
				r2_non_stretched_index_offset_);

			auto& command = command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed;

			param.primitive_type = R3rPrimitiveType::triangle_list;
			param.vertex_count = vertices_per_quad;
			param.index_byte_depth = 1;
			param.index_buffer_offset = 0;
			param.index_offset = index_offset;
		}

		// Disable blending.
		//
		{
			auto& command = command_buffer->write_enable_blending();
			command.is_enable = false;
		}
	}

	// Finalize.
	//
	command_buffer->end_write();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::render_walls()
try {
	if (wall_count_ <= 0 ||
		wall_sides_ib_ == nullptr ||
		wall_sides_vb_ == nullptr ||
		wall_sides_vi_ == nullptr)
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
			draw_item.wall_side = &side;
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
		});

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = wall_sides_ibi_;

		for (int i = 0; i < draw_side_index; ++i)
		{
			const auto& wall_side = *draw_items[i].wall_side;

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 1);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 2);

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 2);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 3);
		}

		update_index_buffer(wall_sides_ib_, 0, ib_index, ib_buffer.data());
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
				const auto r2_texture = texture_mgr_->get_wall(last_texture_id);
				auto& command = command_buffer->write_set_texture();
				command.r2_texture = r2_texture;
			}

			{
				auto& command = command_buffer->write_set_vertex_input();
				command.vertex_input = wall_sides_vi_.get();
			}

			{
				auto& command = command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed;

				param.primitive_type = R3rPrimitiveType::triangle_list;
				param.vertex_count = draw_quad_count * vertices_per_quad;
				param.index_byte_depth = 2;
				param.index_buffer_offset = 0;
				param.index_offset = draw_index_offset_;

				draw_index_offset_ += indices_per_wall_side * draw_quad_count;
			}
		}
	}

	wall_side_draw_item_count_ = draw_side_index;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::render_pushwalls()
try {
	if (pushwall_count_ <= 0 ||
		pushwall_sides_ib_ == nullptr ||
		pushwall_sides_vb_ == nullptr ||
		pushwall_sides_vi_ == nullptr)
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
			draw_item.wall_side = &side;
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
		});

	// Update index buffer.
	//
	{
		auto ib_index = 0;
		auto& ib_buffer = pushwall_sides_ibi_;

		for (int i = 0; i < draw_side_index; ++i)
		{
			const auto& wall_side = *draw_items[i].wall_side;

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 1);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 2);

			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 0);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 2);
			ib_buffer[ib_index++] = static_cast<std::uint16_t>(wall_side.vertex_index + 3);
		}

		update_index_buffer(pushwall_sides_ib_, 0, ib_index, ib_buffer.data());
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
				const auto r2_texture = texture_mgr_->get_wall(last_texture_id);
				auto& command = command_buffer->write_set_texture();
				command.r2_texture = r2_texture;
			}

			{
				auto& command = command_buffer->write_set_vertex_input();
				command.vertex_input = pushwall_sides_vi_.get();
			}

			{
				auto& command = command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed;

				param.primitive_type = R3rPrimitiveType::triangle_list;
				param.vertex_count = draw_quad_count * vertices_per_quad;
				param.index_byte_depth = 2;
				param.index_buffer_offset = 0;
				param.index_offset = draw_index_offset_;

				draw_index_offset_ += indices_per_wall_side * draw_quad_count;
			}
		}
	}

	pushwall_side_draw_item_count_ = draw_side_index;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::render_doors()
try {
	if (door_count_ == 0 ||
		door_sides_ib_ == nullptr ||
		door_sides_vb_ == nullptr ||
		door_sides_vi_ == nullptr)
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
		});

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

		update_index_buffer(door_sides_ib_, 0, ib_index, ib_buffer.data());
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
				const auto r2_texture = texture_mgr_->get_wall(last_texture_id);
				auto& command = command_buffer->write_set_texture();
				command.r2_texture = r2_texture;
			}

			{
				auto& command = command_buffer->write_set_vertex_input();
				command.vertex_input = door_sides_vi_.get();
			}

			{
				auto& command = command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed;

				param.primitive_type = R3rPrimitiveType::triangle_list;
				param.vertex_count = draw_quad_count * vertices_per_quad;
				param.index_byte_depth = 2;
				param.index_buffer_offset = 0;
				param.index_offset = draw_index_offset;

				draw_index_offset += 6 * draw_quad_count;
			}
		}
	}

	door_draw_item_count_ = draw_side_index;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_cloaked_actor(const Sprite& sprite)
try {
	if (!sprite.flags.is_visible)
	{
		return;
	}

	if (sprite.type != SpriteType::actor)
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
		VertexColor{0xFF, 0xFF, 0xFF, 0xFF});


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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int HwVideo::calculate_actor_anim_rotation(const objtype& bs_actor)
try {
	if (!bs_actor.state)
	{
		BSTONE_THROW_STATIC_SOURCE("Null state.");
	}

	auto dir = bs_actor.dir;

	const auto view_dir_x = bs_actor.x - player->x;
	const auto view_dir_y = -bs_actor.y + player->y;

	const auto view_angle_rad = std::atan2(view_dir_y, view_dir_x);
	const auto view_angle = math::rad_to_deg(view_angle_rad);

	if (dir == nodir)
	{
		dir = static_cast<dirtype>(bstone::clamp(bs_actor.trydir & 127, 0, 8));
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int HwVideo::get_bs_actor_sprite_id(const objtype& bs_actor)
try {
	if (bs_actor.state == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null state.");
	}

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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_actor(std::intptr_t bs_actor_index)
try {
	auto& actor = actors_[bs_actor_index];
	const auto& bs_actor = objlist[bs_actor_index];

	if (actor.type == SpriteType::none)
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::orient_sprite(Sprite& sprite)
try {
	sprite.flags.is_visible = false;

	if (sprite.bs_sprite_id <= 0)
	{
		return;
	}

	if (sprite.type == SpriteType::actor)
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

	auto sprite_origin = cgm::Vec2D{};

	if (sprite.type == SpriteType::actor)
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

	const auto square_distance = cgm::dot(direction, direction);

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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_static(int bs_static_index)
try {
	auto& sprite = statics_[bs_static_index];
	const auto& bs_static = statobjlist[bs_static_index];
	auto& hw_static = statics_[bs_static_index];

	if (sprite.type == SpriteType::none)
	{
		map_static(bs_static);
	}
	else
	{
		hw_static.bs_sprite_id = bs_static.shapenum;
	}

	orient_sprite(hw_static);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::render_sprites()
try {
	if (sprites_ib_ == nullptr || sprites_vb_ == nullptr || sprites_vi_ == nullptr)
	{
		return;
	}

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
	update_vertex_buffer(sprites_vb_, min_vertex_index, vertex_count, &sprites_vbi_[min_vertex_index]);

	// Sort by distance (farthest -> nearest).
	//
	std::sort(
		draw_items.begin(),
		draw_items.begin() + draw_sprite_index,
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.sprite->square_distance > rhs.sprite->square_distance;
		});

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

		update_index_buffer(sprites_ib_, 0, ib_index, ib_buffer.data());
	}

	// Add render commands.
	//
	auto& command_buffer = r3_command_buffer_;

	// Disable depth write.
	//
	{
		auto& command = command_buffer->write_enable_depth_write();
		command.is_enable = false;
	}

	// Enable blending.
	//
	{
		auto& command = command_buffer->write_enable_blending();
		command.is_enable = true;
	}

	using CurrentTextureId = ModValue<int>;

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

			if (!gp_no_shading())
			{
				const auto& sprite = *draw_item.sprite;

				switch (sprite.type)
				{
					case SpriteType::actor:
						lighting = sprite.bs_object.actor->lighting;
						break;

					case SpriteType::stat:
						lighting = sprite.bs_object.stat->lighting;
						break;

					default:
						BSTONE_ASSERT(false && "Invalid sprite type.");
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
			if (!gp_no_shading())
			{
				bs_lighting_ = last_lighting;

				if (bs_lighting_.is_modified())
				{
					bs_lighting_.set_is_modified(false);

					auto& command = command_buffer->write_set_f32_uniform();
					command.var = extra_lighting_uniform_;
					command.value = static_cast<float>(bs_lighting_);
				}
			}

			current_texture_id = last_texture_id;

			if (current_texture_id.is_modified())
			{
				current_texture_id.set_is_modified(false);
				const auto r2_texture = texture_mgr_->get_sprite(current_texture_id);

				auto& command = command_buffer->write_set_texture();
				command.r2_texture = r2_texture;
			}

			{
				auto& command = command_buffer->write_set_vertex_input();
				command.vertex_input = sprites_vi_.get();
			}

			{
				auto& command = command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed;

				param.primitive_type = R3rPrimitiveType::triangle_list;
				param.vertex_count = draw_quad_count * vertices_per_quad;
				param.index_byte_depth = 2;
				param.index_buffer_offset = 0;
				param.index_offset = draw_index_offset_;

				draw_index_offset_ += indices_per_sprite * draw_quad_count;
			}
		}
	}

	// Enable depth write.
	//
	{
		auto& command = command_buffer->write_enable_depth_write();
		command.is_enable = true;
	}

	// Disable blending.
	//
	{
		auto& command = command_buffer->write_enable_blending();
		command.is_enable = false;
	}

	sprites_draw_count_ = draw_sprite_index;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::update_3d_fade()
try {
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

	const auto rgba_8 = Rgba8{r, g, b, a};

	texture_mgr_->update_solid_1x1(HwTextureMgrSolid1x1Id::fade_3d, rgba_8);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::present_3d()
try {
	auto& command_buffer = r3_command_buffer_;

	command_buffer->enable(false);

	if (!is_draw_3d_ || screenfaded)
	{
		return;
	}

	const auto is_shading = (!gp_no_shading());

	update_3d_fade();

	command_buffer->enable(true);

	command_buffer->begin_write();

	// Set viewport.
	//
	{
		auto& viewport = command_buffer->write_set_viewport().viewport;
		viewport.x = vid_layout_.screen_viewport_left_width;
		viewport.y = vid_layout_.window_viewport_bottom_height + vid_layout_.screen_viewport_bottom_height;
		viewport.width = vid_layout_.screen_viewport_width;
		viewport.height = vid_layout_.screen_viewport_height;
		viewport.min_depth = 0.0F;
		viewport.max_depth = 1.0F;
	}

	// Enable back-face culling.
	//
	{
		auto& command = command_buffer->write_enable_culling();
		command.is_enable = true;
	}

	// Enable depth test.
	//
	{
		auto& command = command_buffer->write_enable_depth_test();
		command.is_enable = true;
	}

	// Enable depth write.
	//
	{
		auto& command = command_buffer->write_enable_depth_write();
		command.is_enable = true;
	}

	// Set model matrix.
	//
	{
		auto& command = command_buffer->write_set_mat4_uniform();
		command.var = model_mat_uniform_;
		convert(r3_matrix_model_, command.value);
	}

	// Set view matrix.
	//
	{
		auto& command = command_buffer->write_set_mat4_uniform();
		command.var = view_mat_uniform_;
		convert(r3_matrix_view_, command.value);
	}

	// Set projection matrix.
	//
	{
		auto& command = command_buffer->write_set_mat4_uniform();
		command.var = projection_mat_uniform_;
		convert(r3_matrix_projection_, command.value);
	}

	// Set sampler.
	//
	{
		auto& command = command_buffer->write_set_sampler();
		command.sampler = wall_sampler_.get();
	}

	// Set shading mode.
	//
	{
		shading_mode_ = is_shading;

		if (shading_mode_.is_modified())
		{
			shading_mode_.set_is_modified(false);

			auto& command = command_buffer->write_set_i32_uniform();
			command.var = shading_mode_uniform_;
			command.value = shading_mode_;
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
		auto& command = command_buffer->write_set_sampler();
		command.sampler = sprite_sampler_.get();
	}

	// Draw doors.
	//
	render_doors();

	// Set sampler.
	//
	{
		auto& command = command_buffer->write_set_sampler();
		command.sampler = wall_sampler_.get();
	}

	// Draw flooring.
	//
	{
		auto r2_texture = (!gp_is_flooring_solid()
			?
			flooring_textured_t2d_
			:
			flooring_solid_t2d_);

		{
			auto& command = command_buffer->write_set_texture();
			command.r2_texture = r2_texture;
		}

		{
			auto& command = command_buffer->write_set_vertex_input();
			command.vertex_input = flooring_vi_.get();
		}

		{
			auto& command = command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed;

			param.primitive_type = R3rPrimitiveType::triangle_list;
			param.vertex_count = vertices_per_quad;
			param.index_byte_depth = 1;
			param.index_buffer_offset = 0;
			param.index_offset = 0;
		}
	}

	// Draw ceiling.
	//
	{
		auto r2_texture = (!gp_is_ceiling_solid()
			?
			ceiling_textured_t2d_
			:
			ceiling_solid_t2d_);

		{
			auto& command = command_buffer->write_set_texture();
			command.r2_texture = r2_texture;
		}

		{
			auto& command = command_buffer->write_set_vertex_input();
			command.vertex_input = ceiling_vi_.get();
		}

		{
			auto& command = command_buffer->write_draw_indexed();
			auto& param = command.draw_indexed;

			param.primitive_type = R3rPrimitiveType::triangle_list;
			param.vertex_count = vertices_per_quad;
			param.index_byte_depth = 1;
			param.index_buffer_offset = 0;
			param.index_offset = 0;
		}
	}

	// Set sampler.
	//
	{
		auto& command = command_buffer->write_set_sampler();
		command.sampler = sprite_sampler_.get();
	}

	// Draw statics and actors.
	//
	render_sprites();

	// Disable back-face culling.
	//
	{
		auto& command = command_buffer->write_enable_culling();
		command.is_enable = false;
	}

	// Disable depth test.
	//
	{
		auto& command = command_buffer->write_enable_depth_test();
		command.is_enable = false;
	}

	// Disable shading mode.
	//
	if (is_shading)
	{
		shading_mode_ = 0;

		if (shading_mode_.is_modified())
		{
			shading_mode_.set_is_modified(false);

			auto& command = command_buffer->write_set_i32_uniform();
			command.var = shading_mode_uniform_;
			command.value = shading_mode_;
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
				auto& command = command_buffer->write_set_mat4_uniform();
				command.var = projection_mat_uniform_;
				convert(player_weapon_projection_matrix_, command.value);
			}
		}

		if (player_weapon_sprite_id > 0)
		{
			update_player_weapon_model_matrix();
			const auto player_weapon_texture = texture_mgr_->get_sprite(player_weapon_sprite_id);

			// Set model matrix.
			//
			{
				auto& command = command_buffer->write_set_mat4_uniform();
				command.var = model_mat_uniform_;
				convert(player_weapon_model_matrix_, command.value);
			}

			// Set view matrix.
			//
			{
				auto& command = command_buffer->write_set_mat4_uniform();
				command.var = view_mat_uniform_;
				convert(player_weapon_view_matrix_, command.value);
			}

			// Set texture.
			//
			{
				auto& command = command_buffer->write_set_texture();
				command.r2_texture = player_weapon_texture;
			}

			// Set sampler.
			//
			{
				auto& command = command_buffer->write_set_sampler();
				command.sampler = player_weapon_sampler_.get();
			}

			// Set vertex input.
			//
			{
				auto& command = command_buffer->write_set_vertex_input();
				command.vertex_input = player_weapon_vi_.get();
			}

			// Enable blending.
			//
			{
				auto& command = command_buffer->write_enable_blending();
				command.is_enable = true;
			}

			// Set blending function.
			//
			{
				auto& blending_func = command_buffer->write_set_blending_func().blending_func;
				blending_func.src_factor = R3rBlendingFactor::src_alpha;
				blending_func.dst_factor = R3rBlendingFactor::one_minus_src_alpha;
			}

			// Draw the weapon.
			//
			{
				auto& command = command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed;

				param.primitive_type = R3rPrimitiveType::triangle_list;
				param.vertex_count = vertices_per_quad;
				param.index_byte_depth = 1;
				param.index_buffer_offset = 0;
				param.index_offset = 0;
			}

			// Disable blending.
			//
			{
				auto& command = command_buffer->write_enable_blending();
				command.is_enable = false;
			}
		}


		// 3D fade (bonus, damage, death, etc).
		//
		if (r3_fade_is_enabled_)
		{
			// Set model matrix.
			//
			{
				auto& command = command_buffer->write_set_mat4_uniform();
				command.var = model_mat_uniform_;
				convert(cgm::Mat4D::get_identity(), command.value);
			}

			// Set view matrix.
			//
			{
				auto& command = command_buffer->write_set_mat4_uniform();
				command.var = view_mat_uniform_;
				convert(cgm::Mat4D::get_identity(), command.value);
			}

			// Enable blending.
			//
			{
				auto& command = command_buffer->write_enable_blending();
				command.is_enable = true;
			}

			// Set blending function.
			//
			{
				auto& blending_func = command_buffer->write_set_blending_func().blending_func;
				blending_func.src_factor = R3rBlendingFactor::src_alpha;
				blending_func.dst_factor = R3rBlendingFactor::one_minus_src_alpha;
			}

			// Set texture.
			//
			{
				auto& command = command_buffer->write_set_texture();
				command.r2_texture = r3_fade_t2d_;
			}

			// Set sampler.
			//
			{
				auto& command = command_buffer->write_set_sampler();
				command.sampler = fade_sampler_.get();
			}

			// Set vertex input.
			//
			{
				auto& command = command_buffer->write_set_vertex_input();
				command.vertex_input = r3_fade_vi_.get();
			}

			// Draw the quad.
			//
			{
				auto& command = command_buffer->write_draw_indexed();
				auto& param = command.draw_indexed;

				param.primitive_type = R3rPrimitiveType::triangle_list;
				param.vertex_count = vertices_per_quad;
				param.index_byte_depth = 1;
				param.index_buffer_offset = 0;
				param.index_offset = 0;
			}

			// Disable blending.
			//
			{
				auto& command = command_buffer->write_enable_blending();
				command.is_enable = false;
			}
		}
	}

	// Finalize.
	//
	command_buffer->end_write();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool HwVideo::bs_is_activated_pushwall_tile(int tile) noexcept
{
	return (tile & tilemap_door_flags) == tilemap_door_flags;
}

bool HwVideo::bs_is_door(int tile) noexcept
{
	return (tile & tilemap_door_flags) == tilemap_door_flag;
}

bool HwVideo::bs_is_door_track(int tile) noexcept
{
	return (tile & tilemap_door_flags) == tilemap_door_track_flag;
}

bool HwVideo::bs_is_solid_wall(int tile) noexcept
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

bool HwVideo::bs_is_pushwall(int x, int y) noexcept
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

bool HwVideo::bs_is_solid_wall(int x, int y) const noexcept
{
	if (x < 0 || x >= MAPSIZE || y < 0 || y >= MAPSIZE)
	{
		return false;
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

int HwVideo::get_solid_wall_side_count(int x, int y) const
try {
	validate_tile_xy(x, y);

	BSTONE_ASSERT(bs_is_solid_wall(x, y));

	auto side_count = 4;

	side_count -= bs_is_solid_wall(x + 0, y - 1); // north
	side_count -= bs_is_solid_wall(x + 1, y + 0); // east
	side_count -= bs_is_solid_wall(x + 0, y + 1); // south
	side_count -= bs_is_solid_wall(x - 1, y + 0); // west

	return side_count;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int HwVideo::get_door_track_wall_id(int x, int y, controldir_t direction) const
try {
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
			BSTONE_THROW_STATIC_SOURCE("Invalid direction.");
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_flooring()
try {
	texture_mgr_->cache_wall(FloorTile);
	flooring_textured_t2d_ = texture_mgr_->get_wall(FloorTile);

	const auto vga_index = BottomColor & 0xFF;
	const auto vga_color = vgapal + (3 * vga_index);

	const auto renderer_color = vga_color_to_rgba_8(
		vga_color[0],
		vga_color[1],
		vga_color[2]
	);

	texture_mgr_->update_solid_1x1(HwTextureMgrSolid1x1Id::flooring, renderer_color);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_ceiling()
try {
	texture_mgr_->cache_wall(CeilingTile);
	ceiling_textured_t2d_ = texture_mgr_->get_wall(CeilingTile);

	const auto vga_index = TopColor & 0xFF;
	const auto vga_color = vgapal + (3 * vga_index);

	const auto renderer_color = vga_color_to_rgba_8(
		vga_color[0],
		vga_color[1],
		vga_color[2]
	);

	texture_mgr_->update_solid_1x1(HwTextureMgrSolid1x1Id::ceiling, renderer_color);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_wall(int wall_id)
try {
	texture_mgr_->cache_wall(wall_id);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_horizontal_wall(int tile_wall)
try {
	const auto wall_id = horizwall[tile_wall];

	precache_wall(wall_id);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_vertical_wall(int tile_wall)
try {
	const auto wall_id = vertwall[tile_wall];

	precache_wall(wall_id);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_switches()
try {
	precache_horizontal_wall(OFF_SWITCH);
	precache_vertical_wall(OFF_SWITCH);
	precache_horizontal_wall(ON_SWITCH);
	precache_vertical_wall(ON_SWITCH);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_door_track(int x, int y)
try {
	validate_tile_xy(x, y);

	const auto tile = tilemap[x][y];
	const auto tile_wall = tile & tilemap_wall_mask;

	const auto& bs_door = doorobjlist[tile_wall];

	if (bs_door.tilex != x || bs_door.tiley != y)
	{
		BSTONE_THROW_DYNAMIC_SOURCE(
			("Expected a door at (" + std::to_string(x) + ", " + std::to_string(y) + ").").c_str());
	}

	const auto wall_id = door_get_track_texture_id(bs_door);

	precache_wall(wall_id);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_walls()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_pushwalls()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_door_side(int page_number)
try {
	texture_mgr_->cache_wall(page_number);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_door(const doorobj_t& door)
try {
	switch (door.type)
	{
		case dr_normal:
			precache_door_side(DOORWALL + L_METAL);
			precache_door_side(DOORWALL + L_METAL + UL_METAL);
			precache_door_side(DOORWALL + L_METAL_SHADE);
			precache_door_side(DOORWALL + L_METAL_SHADE + UL_METAL);
			break;

		case dr_elevator:
			precache_door_side(DOORWALL + L_ELEVATOR);
			precache_door_side(DOORWALL + L_ELEVATOR + UL_METAL);
			precache_door_side(DOORWALL + L_ELEVATOR_SHADE);
			precache_door_side(DOORWALL + L_ELEVATOR_SHADE + UL_METAL);
			break;

		case dr_prison:
			precache_door_side(DOORWALL + L_PRISON);
			precache_door_side(DOORWALL + L_PRISON + UL_METAL);
			precache_door_side(DOORWALL + L_PRISON_SHADE);
			precache_door_side(DOORWALL + L_PRISON_SHADE + UL_METAL);
			break;

		case dr_space:
			precache_door_side(DOORWALL + L_SPACE);
			precache_door_side(DOORWALL + L_SPACE + UL_METAL);
			precache_door_side(DOORWALL + L_SPACE_SHADE);
			precache_door_side(DOORWALL + L_SPACE_SHADE + UL_METAL);
			break;

		case dr_bio:
			precache_door_side(DOORWALL + L_BIO);
			precache_door_side(DOORWALL + L_BIO + UL_METAL);
			precache_door_side(DOORWALL + L_BIO_SHADE);
			precache_door_side(DOORWALL + L_BIO_SHADE + UL_METAL);
			break;

		case dr_high_security:
			precache_door_side(DOORWALL + L_HIGH_SECURITY);
			precache_door_side(DOORWALL + L_HIGH_SECURITY + UL_METAL);
			precache_door_side(DOORWALL + L_HIGH_SECURITY_SHADE);
			precache_door_side(DOORWALL + L_HIGH_SECURITY_SHADE + UL_METAL);
			break;

		case dr_office:
			precache_door_side(DOORWALL + L_HIGH_TECH);
			precache_door_side(DOORWALL + L_HIGH_TECH + UL_METAL);
			precache_door_side(DOORWALL + L_HIGH_TECH_SHADE);
			precache_door_side(DOORWALL + L_HIGH_TECH_SHADE + UL_METAL);
			break;

		case dr_oneway_up:
		case dr_oneway_left:
		case dr_oneway_right:
		case dr_oneway_down:
			precache_door_side(DOORWALL + L_ENTER_ONLY);
			precache_door_side(DOORWALL + L_ENTER_ONLY + UL_METAL);
			precache_door_side(DOORWALL + L_ENTER_ONLY_SHADE);
			precache_door_side(DOORWALL + L_ENTER_ONLY_SHADE + UL_METAL);
			precache_door_side(DOORWALL + NOEXIT);
			precache_door_side(DOORWALL + NOEXIT_SHADE);
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Invalid door type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_doors()
try {
	door_count_ = 0;

	for (auto bs_door = doorobjlist; lastdoorobj && bs_door != lastdoorobj; ++bs_door)
	{
		++door_count_;

		precache_door(*bs_door);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<typename TVertex>
void HwVideo::update_vertex_xyz(TVertex& vertex, const VertexPosition& xyz) noexcept
{
	const auto traits = VertexAttributeTraits<TVertex, VertexAttribLocationId::position>{};
	UpdateVertexXyz<TVertex, traits.is_valid>{}(vertex, xyz);
}

template<typename TVertex>
void HwVideo::update_vertex_rgba(TVertex& vertex, const VertexColor& rgba_8) noexcept
{
	const auto traits = VertexAttributeTraits<TVertex, VertexAttribLocationId::color>{};
	UpdateVertexRgba<TVertex, traits.is_valid>{}(vertex, rgba_8);
}

template<typename TVertex>
void HwVideo::update_vertex_uv(TVertex& vertex, const VertexTextureCoordinates& uv) noexcept
{
	const auto traits = VertexAttributeTraits<TVertex, VertexAttribLocationId::texture_coordinates>{};
	UpdateVertexUv<TVertex, traits.is_valid>{}(vertex, uv);
}

template<typename TVertex>
void HwVideo::map_wall_side(
	controldir_t side_direction,
	Wall& wall,
	int& vertex_index,
	VertexBufferImageT<TVertex>& vb_buffer)
try {
	const float all_vertex_offsets[4][4] =
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
			BSTONE_THROW_STATIC_SOURCE("Invalid direction.");
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<typename TVertex>
void HwVideo::map_xy_to_xwall(
	XyWallType wall_type,
	int x,
	int y,
	XyWallMap& map,
	int& vertex_index,
	VertexBufferImageT<TVertex>& vb_buffer)
try {
	switch (wall_type)
	{
		case XyWallType::solid:
		case XyWallType::push:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Invalid wall type.");
	}

	auto is_north_solid = false;
	auto is_east_solid = false;
	auto is_south_solid = false;
	auto is_west_solid = false;

	if (wall_type == XyWallType::solid)
	{
		is_north_solid = bs_is_solid_wall(x + 0, y - 1);
		is_east_solid = bs_is_solid_wall(x + 1, y + 0);
		is_south_solid = bs_is_solid_wall(x + 0, y + 1);
		is_west_solid = bs_is_solid_wall(x - 1, y + 0);
	}

	if (is_north_solid && is_east_solid && is_south_solid && is_west_solid)
	{
		// Nothing to draw.
		// This solid wall is surrounded by other ones.
		return;
	}

	const auto xy = encode_xy(x, y);

	if (map.find(xy) != map.cend())
	{
		BSTONE_THROW_STATIC_SOURCE("Wall mapping already exist.");
	}

	map[xy] = Wall{};
	auto& wall = map[xy];

	wall.x = x;
	wall.y = y;

	// A north side.
	if (!is_north_solid)
	{
		map_wall_side(di_north, wall, vertex_index, vb_buffer);
	}

	// An east side.
	if (!is_east_solid)
	{
		map_wall_side(di_east, wall, vertex_index, vb_buffer);
	}

	// An south side.
	if (!is_south_solid)
	{
		map_wall_side(di_south, wall, vertex_index, vb_buffer);
	}

	// A west side.
	if (!is_west_solid)
	{
		map_wall_side(di_west, wall, vertex_index, vb_buffer);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::build_walls()
try {
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
				BSTONE_THROW_STATIC_SOURCE("Invalid direction.");
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

			if (has_active_pushwall_ && x == active_pushwall_next_x_ && y == active_pushwall_next_y_)
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
		BSTONE_THROW_STATIC_SOURCE("Too many wall indices.");
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

			if (has_active_pushwall_ && x == active_pushwall_next_x_ && y == active_pushwall_next_y_)
			{
				continue;
			}

			map_xy_to_xwall(XyWallType::solid, x, y, xy_wall_map_, vertex_index, vb_buffer);
		}
	}

	wall_vertex_count_ = vertex_index;

	// Update vertex buffer.
	//
	update_vertex_buffer(wall_sides_vb_, 0, vertex_count, vb_buffer.data());

	walls_to_render_.reserve(wall_count_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::translate_pushwall_side(
	float translate_x,
	float translate_y,
	controldir_t side_direction,
	const Wall& wall,
	int& vertex_index,
	PushwallsVbi& vb_buffer)
try {
	const float all_vertex_offsets[4][4] =
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
			BSTONE_THROW_STATIC_SOURCE("Invalid direction.");
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::translate_pushwall(const Wall& wall, int& vertex_index, PushwallsVbi& vb_buffer)
try {
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
			BSTONE_THROW_STATIC_SOURCE("Invalid direction.");
	}

	translate_pushwall_side(translate_x, translate_y, di_north, wall, vertex_index, vb_buffer);
	translate_pushwall_side(translate_x, translate_y, di_east, wall, vertex_index, vb_buffer);
	translate_pushwall_side(translate_x, translate_y, di_south, wall, vertex_index, vb_buffer);
	translate_pushwall_side(translate_x, translate_y, di_west, wall, vertex_index, vb_buffer);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::translate_pushwall()
try {
	const auto xy = encode_xy(pwallx, pwally);
	const auto wall_item_it = xy_pushwall_map_.find(xy);

	if (wall_item_it == xy_pushwall_map_.cend())
	{
		BSTONE_THROW_STATIC_SOURCE("Pushwall mapping not found.");
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
		&pushwalls_vbi_[first_vertex_index]);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::step_pushwall(int old_x, int old_y)
try {
	const auto old_xy = encode_xy(old_x, old_y);
	const auto old_wall_item_it = xy_pushwall_map_.find(old_xy);

	if (old_wall_item_it == xy_pushwall_map_.cend())
	{
		BSTONE_THROW_STATIC_SOURCE("Pushwall mapping not found.");
	}

	const auto new_xy = encode_xy(pwallx, pwally);

	auto wall = old_wall_item_it->second;
	wall.x = pwallx;
	wall.y = pwally;

	static_cast<void>(xy_pushwall_map_.erase(old_xy));
	xy_pushwall_map_[new_xy] = wall;

	translate_pushwall();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::build_pushwalls()
try {
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
		BSTONE_THROW_STATIC_SOURCE("Too many pushwall indices.");
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

			map_xy_to_xwall(XyWallType::push, x, y, xy_pushwall_map_, vertex_index,pushwalls_vbi_);
		}
	}

	// Update vertex buffer.
	//
	update_vertex_buffer(pushwall_sides_vb_, 0, vertex_count, pushwalls_vbi_.data());

	pushwalls_to_render_.reserve(pushwall_count_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<typename TVertex>
void HwVideo::update_quad_vertices(
	QuadFlags flags,
	const VertexPosition& origin,
	const cgm::Vec2F& size,
	int& vertex_index,
	VertexBufferImageT<TVertex>& vb_buffer) noexcept
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

void HwVideo::map_door_side(DoorSide& door_side, int& vertex_index, DoorsVbi& vb_buffer) noexcept
{
	const auto& door = *door_side.door;
	const auto bs_door_index = door.bs_door_index;
	const auto& bs_door = doorobjlist[bs_door_index];
	const auto door_offset = static_cast<float>(0.5 * doorposition[bs_door_index]);

	auto flags = QuadFlags{};
	flags.is_back_face = door_side.is_back_face;
	flags.is_vertical = bs_door.vertical;

	const auto origin_axis_index = (flags.is_vertical ? 1 : 0);
	const auto size = cgm::Vec2F{0.5F, 1.0F};
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
		update_quad_vertices(flags, origin, size, vertex_index, vb_buffer);
	}
}

void HwVideo::map_xy_to_door(const doorobj_t& bs_door, int& vertex_index, DoorsVbi& vb_buffer)
try {
	const auto xy = encode_xy(bs_door.tilex, bs_door.tiley);
	const auto map_it = xy_door_map_.find(xy);

	if (map_it != xy_door_map_.cend())
	{
		BSTONE_THROW_STATIC_SOURCE("Door already mapped.");
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
	door_get_page_numbers(bs_door, door.sides[0].texture_id, door.sides[1].texture_id);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::build_doors()
try {
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
		map_xy_to_door(*bs_door, vertex_index, doors_vbi_);
	}

	// Update vertex buffer.
	//
	update_vertex_buffer(door_sides_vb_, 0, vertex_count, doors_vbi_.data());
	doors_to_render_.reserve(door_count_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::initialize_sprites_ibi()
try {
	const auto index_count = max_sprites_indices;

	sprites_ib_ = create_index_buffer(R3rBufferUsageType::draw_streaming, 2, index_count);
	sprites_ibi_.clear();
	sprites_ibi_.resize(index_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_sprites_ib() noexcept
{
	destroy_index_buffer(sprites_ib_);
	sprites_ibi_.clear();
}

void HwVideo::initialize_sprites_vb()
try {
	const auto vertex_count = max_sprites_vertices;
	sprites_vb_ = create_vertex_buffer<SpriteVertex>(R3rBufferUsageType::draw_streaming, vertex_count);
	sprites_vbi_.resize(vertex_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_sprites_vb() noexcept
{
	destroy_vertex_buffer(sprites_vb_);
	sprites_vbi_.clear();
}

void HwVideo::uninitialize_sprites_vi() noexcept
{
	destroy_vertex_input(sprites_vi_);
}

void HwVideo::initialize_sprites_vi()
try {
	create_vertex_input<SpriteVertex>(sprites_ib_, sprites_vb_, sprites_vi_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::initialize_statics()
try {
	statics_.resize(MAXSTATS);

	statics_to_render_.clear();
	statics_to_render_.reserve(MAXSTATS);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::initialize_actors()
try {
	actors_.resize(MAXACTORS);

	actors_to_render_.clear();
	actors_to_render_.reserve(MAXACTORS);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::initialize_sprites()
try {
	sprites_draw_count_ = 0;
	sprites_draw_list_.clear();
	sprites_draw_list_.resize(max_sprites);

	initialize_sprites_ibi();
	initialize_sprites_vb();
	initialize_sprites_vi();
	initialize_statics();
	initialize_actors();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_statics() noexcept
{
	statics_.clear();
}

void HwVideo::uninitialize_actors() noexcept
{
	actors_.clear();
}

void HwVideo::uninitialize_sprites() noexcept
{
	uninitialize_statics();
	uninitialize_actors();

	sprites_draw_count_ = 0;
	sprites_draw_list_.clear();

	uninitialize_sprites_vi();
	uninitialize_sprites_ib();
	uninitialize_sprites_vb();
}

void HwVideo::map_sprite(SpriteType sprite_type, int vertex_index, Sprite& sprite) noexcept
{
	sprite.type = sprite_type;
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
		vertex.rgba8 = Rgba8{0xFF, 0xFF, 0xFF, 0xFF};
		vertex.uv = VertexTextureCoordinates{0.0F, 1.0F};
	}
}

void HwVideo::map_static(const statobj_t& bs_static) noexcept
{
	const auto bs_static_index = static_cast<int>(&bs_static - statobjlist.data());

	auto vertex_index = statics_base_vertex_index;
	vertex_index += (bs_static_index * vertices_per_sprite);

	auto& sprite = statics_[bs_static_index];
	sprite.tile_x = bs_static.tilex;
	sprite.tile_y = bs_static.tiley;
	sprite.bs_object.stat = &bs_static;
	sprite.bs_sprite_id = bs_static.shapenum;

	map_sprite(SpriteType::stat, vertex_index, sprite);
}

void HwVideo::cache_sprite(int bs_sprite_id)
try {
	texture_mgr_->cache_sprite(bs_sprite_id);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_water_bowl()
try {
	cache_sprite(SPR_STAT_40); // Full.
	cache_sprite(SPR_STAT_41); // Empty.
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_chicken_leg()
try {
	cache_sprite(SPR_STAT_42); // Intact.
	cache_sprite(SPR_STAT_43); // Gnawed.
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_ham()
try {
	cache_sprite(SPR_STAT_44); // Intact.
	cache_sprite(SPR_STAT_45); // Bone.
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_candy_bar()
try {
	cache_sprite(SPR_CANDY_BAR); // Intact.
	cache_sprite(SPR_CANDY_WRAPER); // Wrapper.
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_sandwich()
try {
	cache_sprite(SPR_SANDWICH); // Intact.
	cache_sprite(SPR_SANDWICH_WRAPER); // Wrapper.
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_plasma_detonator_explosion()
try {
	cache_sprite(SPR_DETONATOR_EXP1);
	cache_sprite(SPR_DETONATOR_EXP2);
	cache_sprite(SPR_DETONATOR_EXP3);
	cache_sprite(SPR_DETONATOR_EXP4);
	cache_sprite(SPR_DETONATOR_EXP5);
	cache_sprite(SPR_DETONATOR_EXP6);
	cache_sprite(SPR_DETONATOR_EXP7);
	cache_sprite(SPR_DETONATOR_EXP8);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_plasma_detonator()
try {
	cache_sprite(SPR_DOORBOMB);
	cache_sprite(SPR_ALT_DOORBOMB);

	precache_plasma_detonator_explosion();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_static(const statobj_t& bs_static)
try {
	const auto sprite_number = bs_static.shapenum;

	if (false) {}
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_statics()
try {
	for (auto bs_static = statobjlist.data(); laststatobj && bs_static != laststatobj; ++bs_static)
	{
		if (bs_static->shapenum == -1 || (bs_static->tilex == 0 && bs_static->tiley == 0))
		{
			continue;
		}

		precache_static(*bs_static);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::map_actor(const objtype& bs_actor)
try {
	const auto bs_actor_index = get_actor_index(bs_actor);

	auto vertex_index = actors_base_vertex_index;
	vertex_index += (bs_actor_index * vertices_per_sprite);

	auto& sprite = actors_[bs_actor_index];

	map_sprite(SpriteType::actor, vertex_index, sprite);

	sprite.x = bs_actor.x;
	sprite.y = bs_actor.y;
	sprite.tile_x = bs_actor.tilex;
	sprite.tile_y = bs_actor.tiley;
	sprite.bs_sprite_id = get_bs_actor_sprite_id(bs_actor);

	sprite.bs_object.actor = &bs_actor;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Explosion.
void HwVideo::precache_explosion()
try {
	cache_sprite(SPR_EXPLOSION_1);
	cache_sprite(SPR_EXPLOSION_2);
	cache_sprite(SPR_EXPLOSION_3);
	cache_sprite(SPR_EXPLOSION_4);
	cache_sprite(SPR_EXPLOSION_5);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Clip Explosion.
void HwVideo::precache_clip_explosion()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Grenade explosion.
void HwVideo::precache_grenade_explosion()
try {
	cache_sprite(SPR_GRENADE_EXPLODE1);
	cache_sprite(SPR_GRENADE_EXPLODE2);
	cache_sprite(SPR_GRENADE_EXPLODE3);
	cache_sprite(SPR_GRENADE_EXPLODE4);
	cache_sprite(SPR_GRENADE_EXPLODE5);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Flying grenade.
void HwVideo::precache_flying_grenade()
try {
	cache_sprite(SPR_GRENADE_FLY1);
	cache_sprite(SPR_GRENADE_FLY2);
	cache_sprite(SPR_GRENADE_FLY3);
	cache_sprite(SPR_GRENADE_FLY4);

	precache_grenade_explosion();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_anti_plasma_cannon_explosion()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_anti_plasma_cannon_shot()
try {
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		cache_sprite(SPR_BFG_WEAPON_SHOT1);
		cache_sprite(SPR_BFG_WEAPON_SHOT2);
		cache_sprite(SPR_BFG_WEAPON_SHOT3);


		precache_anti_plasma_cannon_explosion();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// A rubble.
void HwVideo::precache_rubble()
try {
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		cache_sprite(SPR_RUBBLE);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Toxic waste (green #1).
void HwVideo::precache_toxic_waste_green_1()
try {
	cache_sprite(SPR_GREEN_OOZE1);
	cache_sprite(SPR_GREEN_OOZE2);
	cache_sprite(SPR_GREEN_OOZE3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Toxic waste (green #2).
void HwVideo::precache_toxic_waste_green_2()
try {
	cache_sprite(SPR_GREEN2_OOZE1);
	cache_sprite(SPR_GREEN2_OOZE2);
	cache_sprite(SPR_GREEN2_OOZE3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Toxic waste (black #1).
void HwVideo::precache_toxic_waste_black_1()
try {
	cache_sprite(SPR_BLACK_OOZE1);
	cache_sprite(SPR_BLACK_OOZE2);
	cache_sprite(SPR_BLACK_OOZE3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Toxic waste (black #2).
void HwVideo::precache_toxic_waste_black_2()
try {
	cache_sprite(SPR_BLACK2_OOZE1);
	cache_sprite(SPR_BLACK2_OOZE2);
	cache_sprite(SPR_BLACK2_OOZE3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Coin (1).
void HwVideo::precache_coin_1()
try {
	cache_sprite(SPR_STAT_77);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Red Access Card.
void HwVideo::precache_red_access_card()
try {
	cache_sprite(SPR_STAT_32);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Yellow Access Card.
void HwVideo::precache_yellow_access_card()
try {
	cache_sprite(SPR_STAT_33);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Green Access Card (AOG).
void HwVideo::precache_green_access_card()
try {
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		cache_sprite(SPR_STAT_34);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Blue Access Card.
void HwVideo::precache_blue_access_card()
try {
	cache_sprite(SPR_STAT_35);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Golden Access Card (AOG).
void HwVideo::precache_golden_access_card()
try {
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog())
	{
		cache_sprite(SPR_STAT_36);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Small yellow box (PS).
void HwVideo::precache_small_yellow_box()
try {
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		cache_sprite(SPR_STAT_36);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Partial / Full Charge Pack.
void HwVideo::precache_charge_packs()
try {
	cache_sprite(SPR_STAT_26);
	cache_sprite(SPR_STAT_31);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Slow Fire Protector.
void HwVideo::precache_slow_fire_protector()
try {
	cache_sprite(SPR_STAT_24);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Rapid Assault Weapon.
void HwVideo::precache_rapid_assault_weapon()
try {
	cache_sprite(SPR_STAT_27);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Dual Neutron Disruptor.
void HwVideo::precache_dual_neutron_disruptor_weapon()
try {
	cache_sprite(SPR_STAT_28);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Plasma Discharge Unit.
void HwVideo::precache_plasma_discharge_unit_weapon()
try {
	cache_sprite(SPR_STAT_46);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Anti-Plasma Cannon.
void HwVideo::precache_anti_plasma_cannon_weapon()
try {
	const auto& assets_info = get_assets_info();

	if (assets_info.is_ps())
	{
		cache_sprite(SPR_STAT_34);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Generic alien spit (#1).
void HwVideo::precache_generic_alien_spit_1()
try {
	cache_sprite(SPR_SPIT1_1);
	cache_sprite(SPR_SPIT1_2);
	cache_sprite(SPR_SPIT1_3);

	cache_sprite(SPR_SPIT_EXP1_1);
	cache_sprite(SPR_SPIT_EXP1_2);
	cache_sprite(SPR_SPIT_EXP1_3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Generic alien spit (#2).
void HwVideo::precache_generic_alien_spit_2()
try {
	cache_sprite(SPR_SPIT2_1);
	cache_sprite(SPR_SPIT2_2);
	cache_sprite(SPR_SPIT2_3);

	cache_sprite(SPR_SPIT_EXP2_1);
	cache_sprite(SPR_SPIT_EXP2_2);
	cache_sprite(SPR_SPIT_EXP2_3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Generic alien spit (#3).
void HwVideo::precache_generic_alien_spit_3()
try {
	cache_sprite(SPR_SPIT3_1);
	cache_sprite(SPR_SPIT3_2);
	cache_sprite(SPR_SPIT3_3);

	cache_sprite(SPR_SPIT_EXP3_1);
	cache_sprite(SPR_SPIT_EXP3_2);
	cache_sprite(SPR_SPIT_EXP3_3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Electrical Shot.
void HwVideo::precache_electrical_shot()
try {
	cache_sprite(SPR_ELEC_SHOT1);
	cache_sprite(SPR_ELEC_SHOT2);
	cache_sprite(SPR_ELEC_SHOT_EXP1);
	cache_sprite(SPR_ELEC_SHOT_EXP2);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Sector Patrol (AOG) / Sector Guard (PS).
void HwVideo::precache_sector_patrol_or_sector_guard()
try {
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
	precache_charge_packs();
	precache_coin_1();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Robot Turret.
void HwVideo::precache_robot_turret()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Bio-Technician.
void HwVideo::precache_bio_technician()
try {
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
	precache_charge_packs();
	precache_coin_1();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Pod Alien.
void HwVideo::precache_pod_alien()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Pod Alien Egg.
void HwVideo::precache_pod_alien_egg()
try {
	cache_sprite(SPR_POD_EGG);
	cache_sprite(SPR_POD_HATCH1);
	cache_sprite(SPR_POD_HATCH2);
	cache_sprite(SPR_POD_HATCH3);


	precache_pod_alien();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// High Energy Plasma Alien.
void HwVideo::precache_high_energy_plasma_alien()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Plasma Sphere.
void HwVideo::precache_plasma_sphere()
try {
	cache_sprite(SPR_ELECTRO_SPHERE_ROAM1);
	cache_sprite(SPR_ELECTRO_SPHERE_ROAM2);
	cache_sprite(SPR_ELECTRO_SPHERE_ROAM3);
	cache_sprite(SPR_ELECTRO_SPHERE_OUCH);
	cache_sprite(SPR_ELECTRO_SPHERE_DIE1);
	cache_sprite(SPR_ELECTRO_SPHERE_DIE2);
	cache_sprite(SPR_ELECTRO_SPHERE_DIE3);
	cache_sprite(SPR_ELECTRO_SPHERE_DIE4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Star Sentinel (AOG) / Tech Warrior (PS).
void HwVideo::precache_star_sentinel_or_tech_warrior()
try {
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
	precache_charge_packs();
	precache_coin_1();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// High-Security Genetic Guard.
void HwVideo::precache_high_security_genetic_guard()
try {
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
	precache_charge_packs();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Experimental Mech-Sentinel.
void HwVideo::precache_experimental_mech_sentinel()
try {
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
	precache_charge_packs();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Experimental Mutant Human.
void HwVideo::precache_experimental_mutant_human()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Morphing Experimental Mutant Human.
void HwVideo::precache_experimental_mutant_human_morphing()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Large Experimental Genetic Alien.
void HwVideo::precache_large_experimental_genetic_alien()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// A canister with large Experimental Genetic Alien.
void HwVideo::precache_canister_with_large_experimental_genetic_alien()
try {
	cache_sprite(SPR_LCAN_ALIEN_READY);
	cache_sprite(SPR_LCAN_ALIEN_B1);
	cache_sprite(SPR_LCAN_ALIEN_B2);
	cache_sprite(SPR_LCAN_ALIEN_B3);
	cache_sprite(SPR_LCAN_ALIEN_EMPTY);

	precache_large_experimental_genetic_alien();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Small Experimental Genetic Alien.
void HwVideo::precache_experimental_genetic_alien_small()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// A canister with small Experimental Genetic Alien.
void HwVideo::precache_canister_with_small_experimental_genetic_alien()
try {
	cache_sprite(SPR_SCAN_ALIEN_READY);
	cache_sprite(SPR_SCAN_ALIEN_B1);
	cache_sprite(SPR_SCAN_ALIEN_B2);
	cache_sprite(SPR_SCAN_ALIEN_B3);
	cache_sprite(SPR_SCAN_ALIEN_EMPTY);


	precache_experimental_genetic_alien_small();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Mutated Guard.
void HwVideo::precache_mutated_guard()
try {
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
	precache_charge_packs();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Mutated Guard (waiting).
void HwVideo::precache_mutated_guard_waiting()
try {
	cache_sprite(SPR_GURNEY_MUT_READY);
	cache_sprite(SPR_GURNEY_MUT_B1);
	cache_sprite(SPR_GURNEY_MUT_B2);
	cache_sprite(SPR_GURNEY_MUT_B3);
	cache_sprite(SPR_GURNEY_MUT_EMPTY);


	precache_mutated_guard();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Fluid Alien Shot.
void HwVideo::precache_fluid_alien_shot()
try {
	cache_sprite(SPR_LIQUID_SHOT_FLY_1);
	cache_sprite(SPR_LIQUID_SHOT_FLY_2);
	cache_sprite(SPR_LIQUID_SHOT_FLY_3);
	cache_sprite(SPR_LIQUID_SHOT_BURST_1);
	cache_sprite(SPR_LIQUID_SHOT_BURST_2);
	cache_sprite(SPR_LIQUID_SHOT_BURST_3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Fluid Alien.
void HwVideo::precache_fluid_alien()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Star Trooper (AOG) / Alien Protector (PS).
void HwVideo::precache_star_trooper_or_alien_protector()
try {
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
	precache_charge_packs();
	precache_coin_1();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Dr. Goldfire.
void HwVideo::precache_dr_goldfire()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Morphed Dr. Goldfire Shot.
void HwVideo::precache_morphed_dr_goldfire_shot()
try {
	cache_sprite(SPR_MGOLD_SHOT1);
	cache_sprite(SPR_MGOLD_SHOT2);
	cache_sprite(SPR_MGOLD_SHOT3);
	cache_sprite(SPR_MGOLD_SHOT_EXP1);
	cache_sprite(SPR_MGOLD_SHOT_EXP2);
	cache_sprite(SPR_MGOLD_SHOT_EXP3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Morphed Dr. Goldfire.
void HwVideo::precache_morphed_dr_goldfire()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Volatile Material Transport.
void HwVideo::precache_volatile_material_transport()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_perscan_drone()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Security Cube Explosion.
void HwVideo::precache_security_cube_explosion()
try {
	cache_sprite(SPR_CUBE_EXP1);
	cache_sprite(SPR_CUBE_EXP2);
	cache_sprite(SPR_CUBE_EXP3);
	cache_sprite(SPR_CUBE_EXP4);
	cache_sprite(SPR_CUBE_EXP5);
	cache_sprite(SPR_CUBE_EXP6);
	cache_sprite(SPR_CUBE_EXP7);
	cache_sprite(SPR_CUBE_EXP8);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Security Cube.
void HwVideo::precache_security_cube_or_projection_generator()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Spider Mutant Shot.
void HwVideo::precache_spider_mutant_shot()
try {
	cache_sprite(SPR_BOSS1_PROJ1);
	cache_sprite(SPR_BOSS1_PROJ2);
	cache_sprite(SPR_BOSS1_PROJ3);
	cache_sprite(SPR_BOSS1_EXP1);
	cache_sprite(SPR_BOSS1_EXP2);
	cache_sprite(SPR_BOSS1_EXP3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Spider Mutant.
void HwVideo::precache_spider_mutant()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Morphing Spider Mutant.
void HwVideo::precache_spider_mutant_morphing()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Breather Beast.
void HwVideo::precache_breather_beast()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Cyborg Warrior.
void HwVideo::precache_cyborg_warrior()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Reptilian Warrior.
void HwVideo::precache_reptilian_warrior()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Reptilian Warrior (morphing).
void HwVideo::precache_reptilian_warrior_morphing()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Acid Dragon Shot.
void HwVideo::precache_acid_dragon_shot()
try {
	cache_sprite(SPR_BOSS5_PROJ1);
	cache_sprite(SPR_BOSS5_PROJ2);
	cache_sprite(SPR_BOSS5_PROJ3);
	cache_sprite(SPR_BOSS5_EXP1);
	cache_sprite(SPR_BOSS5_EXP2);
	cache_sprite(SPR_BOSS5_EXP3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Acid Dragon.
void HwVideo::precache_acid_dragon()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Bio-Mech Guardian.
void HwVideo::precache_bio_mech_guardian()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// The Giant Stalker.
void HwVideo::precache_the_giant_stalker()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// The Spector Demon.
void HwVideo::precache_the_spector_demon()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// The Armored Stalker.
void HwVideo::precache_the_armored_stalker()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// The Crawler Beast Shot.
void HwVideo::precache_the_crawler_beast_shot()
try {
	cache_sprite(SPR_BOSS10_SPIT1);
	cache_sprite(SPR_BOSS10_SPIT2);
	cache_sprite(SPR_BOSS10_SPIT3);

	cache_sprite(SPR_BOSS10_SPIT_EXP1);
	cache_sprite(SPR_BOSS10_SPIT_EXP2);
	cache_sprite(SPR_BOSS10_SPIT_EXP3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// The Crawler Beast.
void HwVideo::precache_the_crawler_beast()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Blake Stone.
void HwVideo::precache_blake_stone()
try {
	cache_sprite(SPR_BLAKE_W1);
	cache_sprite(SPR_BLAKE_W2);
	cache_sprite(SPR_BLAKE_W3);
	cache_sprite(SPR_BLAKE_W4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_vent_and_dripping_blood()
try {
	cache_sprite(SPR_BLOOD_DRIP1);
	cache_sprite(SPR_BLOOD_DRIP2);
	cache_sprite(SPR_BLOOD_DRIP3);
	cache_sprite(SPR_BLOOD_DRIP4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_vent_and_dripping_water()
try {
	cache_sprite(SPR_WATER_DRIP1);
	cache_sprite(SPR_WATER_DRIP2);
	cache_sprite(SPR_WATER_DRIP3);
	cache_sprite(SPR_WATER_DRIP4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_flicker_light()
try {
	cache_sprite(SPR_DECO_ARC_1);
	cache_sprite(SPR_DECO_ARC_2);
	cache_sprite(SPR_DECO_ARC_3);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_crate_content()
try {
	precache_chicken_leg();
	precache_ham();

	precache_charge_packs();

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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_crate_1()
try {
	cache_sprite(SPR_CRATE_1);


	// Goodies.
	//
	precache_grenade_explosion();
	precache_crate_content();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_crate_2()
try {
	cache_sprite(SPR_CRATE_2);


	// Goodies.
	//
	precache_grenade_explosion();
	precache_crate_content();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_crate_3()
try {
	cache_sprite(SPR_CRATE_3);


	// Goodies.
	//
	precache_grenade_explosion();
	precache_crate_content();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_electrical_post_barrier()
try {
	cache_sprite(SPR_ELEC_POST1);
	cache_sprite(SPR_ELEC_POST2);
	cache_sprite(SPR_ELEC_POST3);
	cache_sprite(SPR_ELEC_POST4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_electrical_arc_barrier()
try {
	cache_sprite(SPR_ELEC_ARC1);
	cache_sprite(SPR_ELEC_ARC2);
	cache_sprite(SPR_ELEC_ARC3);
	cache_sprite(SPR_ELEC_ARC4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_vertical_post_barrier()
try {
	cache_sprite(SPR_VPOST1);
	cache_sprite(SPR_VPOST2);
	cache_sprite(SPR_VPOST3);
	cache_sprite(SPR_VPOST4);
	cache_sprite(SPR_VPOST5);
	cache_sprite(SPR_VPOST6);
	cache_sprite(SPR_VPOST7);
	cache_sprite(SPR_VPOST8);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_vertical_spike_barrier()
try {
	cache_sprite(SPR_VSPIKE1);
	cache_sprite(SPR_VSPIKE2);
	cache_sprite(SPR_VSPIKE3);
	cache_sprite(SPR_VSPIKE4);
	cache_sprite(SPR_VSPIKE5);
	cache_sprite(SPR_VSPIKE6);
	cache_sprite(SPR_VSPIKE7);
	cache_sprite(SPR_VSPIKE8);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_security_light()
try {
	cache_sprite(SPR_SECURITY_NORMAL);
	cache_sprite(SPR_SECURITY_ALERT);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_grate_and_steam()
try {
	cache_sprite(SPR_GRATE);
	cache_sprite(SPR_STEAM_1);
	cache_sprite(SPR_STEAM_2);
	cache_sprite(SPR_STEAM_3);
	cache_sprite(SPR_STEAM_4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_pipe_and_steam()
try {
	cache_sprite(SPR_STEAM_PIPE);
	cache_sprite(SPR_PIPE_STEAM_1);
	cache_sprite(SPR_PIPE_STEAM_2);
	cache_sprite(SPR_PIPE_STEAM_3);
	cache_sprite(SPR_PIPE_STEAM_4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_special_stuff()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_access_cards()
try {
	precache_red_access_card();
	precache_yellow_access_card();
	precache_green_access_card();
	precache_blue_access_card();
	precache_golden_access_card();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_player_weapon_auto_charge_pistol()
try {
	cache_sprite(SPR_KNIFEREADY);
	cache_sprite(SPR_KNIFEATK1);
	cache_sprite(SPR_KNIFEATK2);
	cache_sprite(SPR_KNIFEATK3);
	cache_sprite(SPR_KNIFEATK4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_player_weapon_slow_fire_protector()
try {
	cache_sprite(SPR_PISTOLREADY);
	cache_sprite(SPR_PISTOLATK1);
	cache_sprite(SPR_PISTOLATK2);
	cache_sprite(SPR_PISTOLATK3);
	cache_sprite(SPR_PISTOLATK4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_player_weapon_rapid_assault_weapon()
try {
	cache_sprite(SPR_MACHINEGUNREADY);
	cache_sprite(SPR_MACHINEGUNATK1);
	cache_sprite(SPR_MACHINEGUNATK2);
	cache_sprite(SPR_MACHINEGUNATK3);
	cache_sprite(SPR_MACHINEGUNATK4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_player_weapon_dual_neutron_disruptor()
try {
	cache_sprite(SPR_CHAINREADY);
	cache_sprite(SPR_CHAINATK1);
	cache_sprite(SPR_CHAINATK2);
	cache_sprite(SPR_CHAINATK3);
	cache_sprite(SPR_CHAINATK4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_player_weapon_plasma_discharge_unit()
try {
	cache_sprite(SPR_GRENADEREADY);
	cache_sprite(SPR_GRENADEATK1);
	cache_sprite(SPR_GRENADEATK2);
	cache_sprite(SPR_GRENADEATK3);
	cache_sprite(SPR_GRENADEATK4);

	precache_flying_grenade();
	precache_grenade_explosion();
	precache_explosion();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_player_weapon_anti_plasma_cannon()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_player_weapons()
try {
	precache_player_weapon_auto_charge_pistol();
	precache_player_weapon_slow_fire_protector();
	precache_player_weapon_rapid_assault_weapon();
	precache_player_weapon_dual_neutron_disruptor();
	precache_player_weapon_plasma_discharge_unit();
	precache_player_weapon_anti_plasma_cannon();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_dead(const objtype& bs_actor)
try {
	const auto sprite_id = get_bs_actor_sprite_id(bs_actor);
	cache_sprite(sprite_id);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_actors()
try {
	if (player == nullptr)
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_sprites()
try {
	precache_statics();
	precache_actors();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::build_statics()
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::build_actors()
try {
	uninitialize_actors();
	initialize_actors();

	if (player == nullptr)
	{
		return;
	}

	for (auto bs_actor = player->next; bs_actor; bs_actor = bs_actor->next)
	{
		map_actor(*bs_actor);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::build_sprites()
try {
	uninitialize_sprites();
	initialize_sprites();

	build_statics();
	build_actors();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::precache_resources()
try {
	texture_mgr_->begin_cache();

	precache_flooring();
	precache_ceiling();
	precache_walls();
	precache_pushwalls();
	precache_doors();
	precache_sprites();

	texture_mgr_->end_cache();
	texture_mgr_->purge_cache();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_texture_upscale_resources() noexcept
{
	ui_t2d_ = nullptr;
	flooring_textured_t2d_ = nullptr;
	ceiling_textured_t2d_ = nullptr;
}

void HwVideo::create_texture_upscale_resources()
try {
	ui_t2d_ = texture_mgr_->get_ui();

	if (FloorTile > 0)
	{
		flooring_textured_t2d_ = texture_mgr_->get_wall(FloorTile);
	}

	if (CeilingTile > 0)
	{
		ceiling_textured_t2d_ = texture_mgr_->get_wall(CeilingTile);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::destroy_external_textures_resources() noexcept
{
	flooring_textured_t2d_ = nullptr;
	ceiling_textured_t2d_ = nullptr;
}

void HwVideo::create_external_textures_resources()
try {
	if (FloorTile > 0)
	{
		flooring_textured_t2d_ = texture_mgr_->get_wall(FloorTile);
	}

	if (CeilingTile > 0)
	{
		ceiling_textured_t2d_ = texture_mgr_->get_wall(CeilingTile);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void HwVideo::uninitialize_video() noexcept
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

void HwVideo::initialize_video()
try {
	set_samplers_default_states();
	set_player_weapon_sampler_default_state();

	renderer_mgr_ = make_r3r_mgr(*globals::sys_video_mgr, *globals::sys_window_mgr);

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

	auto& window = renderer_->get_window();
	const auto window_title = vid_get_window_title_for_renderer(renderer_->get_name());
	window.set_title(window_title.c_str());
	window.show(true);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

// ==========================================================================

VideoUPtr make_hw_video()
{
	return std::make_unique<HwVideo>();
}

} // namespace bstone
