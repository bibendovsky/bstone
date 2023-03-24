/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// 3D-renderer texture manager (implementation).
//


#include "bstone_hw_texture_mgr.h"

#include <cassert>

#include <array>
#include <unordered_map>

#include "xbrz.h"

#include "id_ca.h"
#include "id_vl.h"

#include "bstone_atomic_flag.h"
#include "bstone_exception.h"
#include "bstone_file_system.h"
#include "bstone_file_stream.h"
#include "bstone_globals.h"
#include "bstone_missing_sprite_64x64_image.h"
#include "bstone_missing_wall_64x64_image.h"
#include "bstone_mt_task_mgr.h"
#include "bstone_ref_values.h"
#include "bstone_ren_3d.h"
#include "bstone_ren_3d_limits.h"
#include "bstone_rgb_palette.h"
#include "bstone_rgb8.h"
#include "bstone_sprite_cache.h"
#include "bstone_image_decoder.h"

#include "bstone_detail_ren_3d_utils.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// HwTextureMgrXbrzTaskException
//

class HwTextureMgrXbrzTaskException :
	public Exception
{
public:
	explicit HwTextureMgrXbrzTaskException(
		const char* message) noexcept
		:
		Exception{"HW_TEXTURE_MGR_XBRZ_TASK", message}
	{
	}
}; // HwTextureMgrXbrzTaskException

//
// HwTextureMgrXbrzTaskException
// ==========================================================================


// ==========================================================================
// HwTextureMgrXbrzTask
//

class HwTextureMgrXbrzTask final :
	public MtTask
{
public:
	void execute() override;


	bool is_completed() const noexcept override;

	void set_completed() override;


	bool is_failed() const noexcept override;

	std::exception_ptr get_exception_ptr() const noexcept override;

	void set_failed(
		std::exception_ptr exception_ptr) override;


	void initialize(
		const int factor,
		const int first_index,
		const int last_index,
		const int src_width,
		const int src_height,
		const std::uint32_t* const src_colors,
		std::uint32_t* const dst_colors) noexcept;


private:
	AtomicFlag is_completed_{};
	AtomicFlag is_failed_{};
	std::exception_ptr exception_ptr_{};


	int factor_{};
	int first_index_{};
	int last_index_{};
	int src_width_{};
	int src_height_{};
	const std::uint32_t* src_colors_{};
	std::uint32_t* dst_colors_{};


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);
}; // HwTextureMgrXbrzTask

using XbrzTaskPtr = HwTextureMgrXbrzTask*;

using XbrzTasks = std::vector<HwTextureMgrXbrzTask>;
using XbrzTaskPtrs = std::vector<MtTaskPtr>;


void HwTextureMgrXbrzTask::execute()
try
{
	const auto xbrz_cfg = xbrz::ScalerCfg{};

	xbrz::scale(
		static_cast<std::size_t>(factor_),
		src_colors_,
		dst_colors_,
		src_width_,
		src_height_,
		xbrz::ColorFormat::ARGB_UNBUFFERED,
		xbrz_cfg,
		first_index_,
		last_index_
	);
}
catch (...)
{
	fail_nested(__func__);
}

bool HwTextureMgrXbrzTask::is_completed() const noexcept
{
	return is_completed_;
}

void HwTextureMgrXbrzTask::set_completed()
try
{
	if (is_completed_)
	{
		fail("Already completed.");
	}

	is_completed_ = true;
}
catch (...)
{
	fail_nested(__func__);
}

bool HwTextureMgrXbrzTask::is_failed() const noexcept
{
	return is_failed_;
}

std::exception_ptr HwTextureMgrXbrzTask::get_exception_ptr() const noexcept
{
	return exception_ptr_;
}

void HwTextureMgrXbrzTask::set_failed(
	std::exception_ptr exception_ptr)
try
{
	if (is_completed_)
	{
		fail("Already completed.");
	}

	if (is_failed_)
	{
		fail("Already failed.");
	}

	is_completed_ = true;
	is_failed_ = true;

	exception_ptr_ = exception_ptr;
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrXbrzTask::initialize(
	const int factor,
	const int first_index,
	const int last_index,
	const int src_width,
	const int src_height,
	const std::uint32_t* const src_colors,
	std::uint32_t* const dst_colors) noexcept
{
	factor_ = factor;
	first_index_ = first_index;
	last_index_ = last_index;
	src_width_ = src_width;
	src_height_ = src_height;
	src_colors_ = src_colors;
	dst_colors_ = dst_colors;
}

[[noreturn]]
void HwTextureMgrXbrzTask::fail(
	const char* message)
{
	throw HwTextureMgrXbrzTaskException{message};
}

[[noreturn]]
void HwTextureMgrXbrzTask::fail_nested(
	const char* message)
{
	std::throw_with_nested(HwTextureMgrXbrzTaskException{message});
}

//
// HwTextureMgrXbrzTask
// ==========================================================================


// ==========================================================================
// HwTextureMgrException
//

class HwTextureMgrException :
	public Exception
{
public:
	explicit HwTextureMgrException(
		const char* message) noexcept
		:
		Exception{"HW_TEXTURE_MGR", message}
	{
	}
}; // HwTextureMgrException

//
// HwTextureMgrException
// ==========================================================================


} // detail


// ==========================================================================
// HwTextureMgrImpl
//

class HwTextureMgrImpl final :
	public HwTextureMgr
{
public:
	HwTextureMgrImpl(
		const Ren3dPtr renderer,
		const SpriteCachePtr cache_sprite,
		const MtTaskMgrPtr mt_task_manager);

	~HwTextureMgrImpl() override;


	int get_min_upscale_filter_degree(
		const HwTextureMgrUpscaleFilterKind upscale_filter_kind) const override;

	int get_max_upscale_filter_degree(
		const HwTextureMgrUpscaleFilterKind upscale_filter_kind) const override;

	HwTextureMgrUpscaleFilterKind get_upscale_filter_kind() const noexcept override;

	int get_upscale_filter_degree() const noexcept override;

	void set_upscale_filter(
		const HwTextureMgrUpscaleFilterKind upscale_filter,
		const int upscale_filter_factor) override;

	void enable_external_textures(
		bool is_enable) override;


	void begin_cache() override;

	void end_cache() override;

	void purge_cache() override;


	void cache_wall(
		const int id) override;

	Ren3dTexture2dPtr get_wall(
		const int id) const override;


	void cache_sprite(
		const int id) override;

	Ren3dTexture2dPtr get_sprite(
		const int id) const override;


	void destroy_ui() override;

	void create_ui(
		const std::uint8_t* const indexed_pixels,
		const bool* const indexed_alphas,
		const Rgba8PaletteCPtr indexed_palette) override;

	void update_ui() override;

	Ren3dTexture2dPtr get_ui() const noexcept override;


	void try_destroy_solid_1x1(
		const HwTextureMgrSolid1x1Id id) noexcept override;

	void destroy_solid_1x1(
		const HwTextureMgrSolid1x1Id id) override;

	void create_solid_1x1(
		const HwTextureMgrSolid1x1Id id) override;

	void update_solid_1x1(
		const HwTextureMgrSolid1x1Id id,
		const Rgba8 color) override;

	Ren3dTexture2dPtr get_solid_1x1(
		const HwTextureMgrSolid1x1Id id) const override;


private:
	static constexpr auto wall_dimension = 64;

	static constexpr auto max_walls = 256;
	static constexpr auto max_sprites = 1'024;


	using GenerationId = unsigned int;

	static constexpr auto invalid_generation_id = GenerationId{};
	static constexpr auto first_generation_id = GenerationId{1};


	enum class ImageKind
	{
		sprite,
		wall,
	}; // ImageKind

	struct Texture2dProperties
	{
		Ren3dPixelFormat image_pixel_format_;

		bool is_npot_;

		int width_;
		int height_;

		int upscale_width_;
		int upscale_height_;

		int actual_width_;
		int actual_height_;

		bool is_generate_mipmaps_;
		int mipmap_count_;

		bool indexed_is_column_major_;
		const std::uint8_t* indexed_pixels_;
		Rgba8PaletteCPtr indexed_palette_;
		const bool* indexed_alphas_;

		SpriteCPtr indexed_sprite_;

		Rgba8CPtr rgba_8_pixels_;
	}; // Texture2dProperties

	struct Texture2dItem
	{
		GenerationId generation_id_;
		Texture2dProperties properties_;
		Ren3dTexture2dUPtr texture_2d_;

		Texture2dItem() noexcept;

		Texture2dItem(
			Texture2dItem&& rhs) noexcept;

		Texture2dItem& operator=(
			Texture2dItem&& rhs) noexcept;
	}; // Texture2dItem

	using IdToTexture2dMap = std::unordered_map<int, Texture2dItem>;

	struct Solid1x1Item
	{
		Rgba8 color_;
		Texture2dProperties properties_;
		Ren3dTexture2dUPtr texture_2d_;


		void clear() noexcept;
	}; // Solid1x1Item

	using Solid1x1Items = std::array<Solid1x1Item, static_cast<std::size_t>(HwTextureMgrSolid1x1Id::count_)>;

	struct ExternalImageProbeItem
	{
		std::string file_name_extension;
		ImageDecoder* image_decoder;
	}; // ExternalImageProbeItem

	using ExternalImageProbeItems = std::vector<ExternalImageProbeItem>;


	Ren3dPtr renderer_;
	SpriteCachePtr sprite_cache_;
	MtTaskMgrPtr mt_task_manager_;

	HwTextureMgrUpscaleFilterKind upscale_filter_kind_;
	int upscale_filter_factor_;

	bool is_caching_;
	GenerationId generation_id_;

	IdToTexture2dMap wall_map_;
	IdToTexture2dMap sprite_map_;

	Texture2dItem missing_sprite_texture_2d_item_;
	Texture2dItem missing_wall_texture_2d_item_;

	Texture2dItem ui_t2d_item_;

	detail::Ren3dUtils::Rgba8Buffer mipmap_buffer_;
	detail::Ren3dUtils::Rgba8Buffer upscale_buffer_;

	Solid1x1Items solid_1x1_items_;

	detail::XbrzTasks xbrz_tasks_;
	detail::XbrzTaskPtrs xbrz_task_ptrs_;

	bool is_external_textures_enabled_{};
	FileStream image_file_stream_;
	Buffer image_buffer_;
	Rgba8Buffer image_buffer_rgba8_;
	ImageDecodeUPtr bmp_image_decoder_;
	ImageDecodeUPtr png_image_decoder_;
	ExternalImageProbeItems image_probe_items_;
	std::string image_data_path_;
	std::string image_mod_path_;


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	static void validate_upscale_filter(
		const HwTextureMgrUpscaleFilterKind upscale_filter_kind,
		const int upscale_filter_factor);


	void validate_image_source_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_image_pixel_format_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_dimensions_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_mipmap_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_common_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_indexed_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_indexed_sprite_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_rgba_8_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_source_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_texture_2d_properties(
		const Texture2dProperties& properties);

	void set_common_texture_2d_properties(
		Texture2dProperties& properties);


	void recreate_indexed_resources();

	void recreate_sprites_and_walls();

	void uninitialize() noexcept;

	void initialize(
		Ren3dPtr renderer,
		SpriteCachePtr cache_sprite);

	void uninitialize_internal() noexcept;


	void upscale_xbrz(
		const Texture2dProperties& properties);

	void upscale(
		const Texture2dProperties& properties);


	Texture2dItem create_texture(
		const Texture2dProperties& properties);

	void update_mipmaps(
		const Texture2dProperties& properties,
		const Ren3dTexture2dUPtr& texture_2d);


	void destroy_missing_sprite_texture() noexcept;

	void create_missing_sprite_texture();


	void destroy_missing_wall_texture() noexcept;

	void create_missing_wall_texture();


	Texture2dItem create_from_external_image(
		int id,
		ImageKind kind);

	Texture2dItem wall_create_texture(
		const int wall_id);

	Texture2dItem sprite_create_texture(
		const int sprite_id);


	void initialize_internal(
		Ren3dPtr renderer,
		SpriteCachePtr cache_sprite);

	void purge_cache(
		IdToTexture2dMap& map);

	Ren3dTexture2dPtr get_texture_2d(
		const ImageKind image_kind,
		const int id,
		const IdToTexture2dMap& map) const noexcept;

	void solid_1x1_destroy_all() noexcept;

	static int solid_1x1_try_get_index(
		const HwTextureMgrSolid1x1Id id) noexcept;

	static int solid_1x1_get_index(
		const HwTextureMgrSolid1x1Id id);

	static int solid_1x1_get_updateable_index(
		const HwTextureMgrSolid1x1Id id);

	static Rgba8 solid_1x1_get_default_color(
		const HwTextureMgrSolid1x1Id id);

	static int upscale_filter_get_min_factor_internal(
		const HwTextureMgrUpscaleFilterKind upscale_filter_kind);

	static int upscale_filter_get_max_factor_internal(
		const HwTextureMgrUpscaleFilterKind upscale_filter_kind);

	static int upscale_filter_clamp_factor(
		const HwTextureMgrUpscaleFilterKind upscale_filter_kind,
		const int upscale_filter_factor);
}; // Detail

using HwTextureMgrImplUPtr = std::unique_ptr<HwTextureMgrImpl>;


HwTextureMgrImpl::Texture2dItem::Texture2dItem() noexcept
	:
	generation_id_{},
	properties_{},
	texture_2d_{}
{
}

HwTextureMgrImpl::Texture2dItem::Texture2dItem(
	Texture2dItem&& rhs) noexcept
	:
	generation_id_{std::move(rhs.generation_id_)},
	properties_{std::move(rhs.properties_)},
	texture_2d_{std::move(rhs.texture_2d_)}
{
}

HwTextureMgrImpl::Texture2dItem& HwTextureMgrImpl::Texture2dItem::operator=(
	Texture2dItem&& rhs) noexcept
{
	std::swap(generation_id_, rhs.generation_id_);
	std::swap(properties_, rhs.properties_);
	std::swap(texture_2d_, rhs.texture_2d_);

	return *this;
}

void HwTextureMgrImpl::Solid1x1Item::clear() noexcept
{
	color_.reset();
	properties_ = {};
	texture_2d_ = nullptr;
}

HwTextureMgrImpl::HwTextureMgrImpl(
	const Ren3dPtr renderer,
	const SpriteCachePtr cache_sprite,
	const MtTaskMgrPtr mt_task_manager)
try
	:
	renderer_{},
	sprite_cache_{},
	mt_task_manager_{mt_task_manager},
	upscale_filter_kind_{},
	upscale_filter_factor_{},
	is_caching_{},
	generation_id_{},
	wall_map_{},
	sprite_map_{},
	missing_sprite_texture_2d_item_{},
	missing_wall_texture_2d_item_{},
	ui_t2d_item_{},
	mipmap_buffer_{},
	upscale_buffer_{},
	solid_1x1_items_{},
	xbrz_tasks_{},
	xbrz_task_ptrs_{}
{
	initialize(renderer, cache_sprite);
}
catch (...)
{
	fail_nested(__func__);
}

HwTextureMgrImpl::~HwTextureMgrImpl()
{
	uninitialize_internal();
}

void HwTextureMgrImpl::uninitialize() noexcept
{
	uninitialize_internal();
}

void HwTextureMgrImpl::recreate_indexed_resources()
try
{
	// Sprites.
	//
	for (auto& sprite_item : sprite_map_)
	{
		const auto sprite_id = sprite_item.first;
		auto& texture_2d_item = sprite_item.second;

		if (texture_2d_item.properties_.rgba_8_pixels_)
		{
			continue;
		}

		auto new_texture_2d_item = sprite_create_texture(sprite_id);
		new_texture_2d_item.generation_id_ = generation_id_;

		texture_2d_item = std::move(new_texture_2d_item);
	}

	// Walls.
	//
	for (auto& wall_item : wall_map_)
	{
		const auto wall_id = wall_item.first;
		auto& texture_2d_item = wall_item.second;

		if (texture_2d_item.properties_.rgba_8_pixels_)
		{
			continue;
		}

		auto new_texture_2d_item = wall_create_texture(wall_id);
		new_texture_2d_item.generation_id_ = generation_id_;

		texture_2d_item = std::move(new_texture_2d_item);
	}

	// UI texture.
	//
	{
		destroy_ui();

		create_ui(
			ui_t2d_item_.properties_.indexed_pixels_,
			ui_t2d_item_.properties_.indexed_alphas_,
			ui_t2d_item_.properties_.indexed_palette_);
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::recreate_sprites_and_walls()
try
{
	// Sprites.
	//
	for (auto& sprite_item : sprite_map_)
	{
		const auto sprite_id = sprite_item.first;
		auto& texture_2d_item = sprite_item.second;

		auto new_texture_2d_item = sprite_create_texture(sprite_id);
		new_texture_2d_item.generation_id_ = generation_id_;

		texture_2d_item = std::move(new_texture_2d_item);
	}

	// Walls.
	//
	for (auto& wall_item : wall_map_)
	{
		const auto wall_id = wall_item.first;
		auto& texture_2d_item = wall_item.second;

		auto new_texture_2d_item = wall_create_texture(wall_id);
		new_texture_2d_item.generation_id_ = generation_id_;

		texture_2d_item = std::move(new_texture_2d_item);
	}
}
catch (...)
{
	fail_nested(__func__);
}

int HwTextureMgrImpl::get_min_upscale_filter_degree(
	const HwTextureMgrUpscaleFilterKind upscale_filter_kind) const
try
{
	return upscale_filter_get_min_factor_internal(upscale_filter_kind);
}
catch (...)
{
	fail_nested(__func__);
}

int HwTextureMgrImpl::get_max_upscale_filter_degree(
	const HwTextureMgrUpscaleFilterKind upscale_filter_kind) const
try
{
	return upscale_filter_get_max_factor_internal(upscale_filter_kind);
}
catch (...)
{
	fail_nested(__func__);
}

HwTextureMgrUpscaleFilterKind HwTextureMgrImpl::get_upscale_filter_kind() const noexcept
{
	return upscale_filter_kind_;
}

int HwTextureMgrImpl::get_upscale_filter_degree() const noexcept
{
	return upscale_filter_factor_;
}

void HwTextureMgrImpl::set_upscale_filter(
	const HwTextureMgrUpscaleFilterKind upscale_filter_kind,
	const int upscale_filter_factor)
try
{
	const auto clamped_upscale_filter_factor = upscale_filter_clamp_factor(
		upscale_filter_kind,
		upscale_filter_factor
	);

	if (upscale_filter_kind_ == upscale_filter_kind &&
		upscale_filter_factor_ == clamped_upscale_filter_factor)
	{
		return;
	}

	validate_upscale_filter(upscale_filter_kind, clamped_upscale_filter_factor);

	upscale_filter_kind_ = upscale_filter_kind;
	upscale_filter_factor_ = clamped_upscale_filter_factor;

	recreate_indexed_resources();
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::enable_external_textures(
	bool is_enable)
try
{
	if (is_external_textures_enabled_ == is_enable)
	{
		return;
	}

	is_external_textures_enabled_ = is_enable;

	recreate_sprites_and_walls();
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::begin_cache()
try
{
	if (is_caching_)
	{
		fail("Already caching.");
	}

	is_caching_ = true;

	++generation_id_;

	if (generation_id_ == invalid_generation_id)
	{
		generation_id_ = first_generation_id;
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::end_cache()
try
{
	if (!is_caching_)
	{
		fail("Not caching.");
	}

	is_caching_ = false;
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::purge_cache()
try
{
	if (is_caching_)
	{
		fail("Caching is active.");
	}

	purge_cache(wall_map_);
	purge_cache(sprite_map_);
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::cache_wall(
	const int id)
try
{
	if (id < 0 || id >= max_walls)
	{
		fail("Id out of range.");
	}

	auto wall_it = wall_map_.find(id);

	if (wall_it != wall_map_.end())
	{
		wall_it->second.generation_id_ = generation_id_;

		return;
	}

	auto texture_2d_item = wall_create_texture(id);
	texture_2d_item.generation_id_ = generation_id_;

	wall_map_[id] = std::move(texture_2d_item);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dTexture2dPtr HwTextureMgrImpl::get_wall(
	const int id) const
try
{
	if (id < 0 || id >= max_walls)
	{
		fail("Id out of range.");
	}

	return get_texture_2d(ImageKind::wall, id, wall_map_);
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::cache_sprite(
	const int id)
try
{
	if (id <= 0 || id >= max_sprites)
	{
		fail("Id out of range.");
	}

	auto sprite_it = sprite_map_.find(id);

	if (sprite_it != sprite_map_.end())
	{
		sprite_it->second.generation_id_ = generation_id_;

		return;
	}

	auto texture_2d_item = sprite_create_texture(id);
	texture_2d_item.generation_id_ = generation_id_;

	sprite_map_[id] = std::move(texture_2d_item);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dTexture2dPtr HwTextureMgrImpl::get_sprite(
	const int id) const
try
{
	if (id <= 0 || id >= max_sprites)
	{
		fail("Sprite id out of range.");
	}

	return get_texture_2d(ImageKind::sprite, id, sprite_map_);
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::destroy_ui()
{
	ui_t2d_item_.texture_2d_ = nullptr;
}

void HwTextureMgrImpl::create_ui(
	const std::uint8_t* const indexed_pixels,
	const bool* const indexed_alphas,
	const Rgba8PaletteCPtr indexed_palette)
try
{
	if (ui_t2d_item_.texture_2d_)
	{
		fail("UI texture already created.");
	}

	if (!indexed_pixels)
	{
		fail("Null indexed pixels for UI texture.");
	}

	if (!indexed_alphas)
	{
		fail("Null indexed alphas for UI texture.");
	}

	if (!indexed_palette)
	{
		fail("Null indexed palette for UI texture.");
	}

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = Ren3dPixelFormat::rgba_8_unorm;
	param.width_ = vga_ref_width;
	param.height_ = vga_ref_height;
	param.mipmap_count_ = 1;
	param.indexed_pixels_ = indexed_pixels;
	param.indexed_palette_ = indexed_palette;
	param.indexed_alphas_ = indexed_alphas;

	auto texture_2d_item = create_texture(param);
	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	ui_t2d_item_ = std::move(texture_2d_item);
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::update_ui()
try
{
	update_mipmaps(ui_t2d_item_.properties_, ui_t2d_item_.texture_2d_);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dTexture2dPtr HwTextureMgrImpl::get_ui() const noexcept
{
	return ui_t2d_item_.texture_2d_.get();
}

void HwTextureMgrImpl::try_destroy_solid_1x1(
	const HwTextureMgrSolid1x1Id id) noexcept
{
	const auto index = solid_1x1_try_get_index(id);

	if (index < 0)
	{
		return;
	}

	auto& item = solid_1x1_items_[index];
	item.clear();
}

void HwTextureMgrImpl::destroy_solid_1x1(
	const HwTextureMgrSolid1x1Id id)
try
{
	const auto index = solid_1x1_get_index(id);

	auto& item = solid_1x1_items_[index];
	item.clear();
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::create_solid_1x1(
	const HwTextureMgrSolid1x1Id id)
try
{
	const auto index = solid_1x1_get_index(id);

	const auto default_color = solid_1x1_get_default_color(id);

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = bstone::Ren3dPixelFormat::rgba_8_unorm;
	param.width_ = 1;
	param.height_ = 1;
	param.mipmap_count_ = 1;
	param.rgba_8_pixels_ = &default_color;

	auto texture_2d_item = create_texture(param);

	auto& item = solid_1x1_items_[index];
	item.color_ = default_color;
	item.properties_ = texture_2d_item.properties_;
	item.texture_2d_ = std::move(texture_2d_item.texture_2d_);

	update_mipmaps(item.properties_, item.texture_2d_);
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::update_solid_1x1(
	const HwTextureMgrSolid1x1Id id,
	const Rgba8 color)
try
{
	const auto index = solid_1x1_get_updateable_index(id);

	auto& item = solid_1x1_items_[index];
	item.color_ = color;

	auto param = Ren3dTexture2dUpdateParam{};
	param.image_ = &item.color_;

	item.texture_2d_->update(param);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dTexture2dPtr HwTextureMgrImpl::get_solid_1x1(
	const HwTextureMgrSolid1x1Id id) const
try
{
	const auto index = solid_1x1_get_index(id);

	if (index < 0)
	{
		fail("Invalid solid 1x1 2D-texture id.");
	}

	return solid_1x1_items_[index].texture_2d_.get();
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::initialize(
	Ren3dPtr renderer,
	SpriteCachePtr cache_sprite)
try
{
	initialize_internal(renderer, cache_sprite);
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::uninitialize_internal() noexcept
{
	generation_id_ = invalid_generation_id;

	// Wall map.
	//
	wall_map_.clear();

	// Sprite map.
	//
	sprite_map_.clear();

	// "Missing" textures.
	//
	destroy_missing_sprite_texture();
	destroy_missing_wall_texture();

	// UI texture.
	//
	destroy_ui();

	// Solid 1x1 textures.
	//
	solid_1x1_destroy_all();

	renderer_ = nullptr;
	sprite_cache_ = nullptr;
	mipmap_buffer_.clear();
}

[[noreturn]]
void HwTextureMgrImpl::fail(
	const char* message)
{
	throw detail::HwTextureMgrException{message};
}

[[noreturn]]
void HwTextureMgrImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(detail::HwTextureMgrException{message});
}

void HwTextureMgrImpl::validate_upscale_filter(
	const HwTextureMgrUpscaleFilterKind upscale_filter_kind,
	const int upscale_filter_factor)
try
{
	switch (upscale_filter_kind)
	{
		case HwTextureMgrUpscaleFilterKind::none:
		case HwTextureMgrUpscaleFilterKind::xbrz:
			break;

		default:
			fail("Unsupported upscale filter kind.");
	}

	const auto min_factor = upscale_filter_get_min_factor_internal(upscale_filter_kind);
	const auto max_factor = upscale_filter_get_max_factor_internal(upscale_filter_kind);

	if (upscale_filter_factor < min_factor ||
		upscale_filter_factor > max_factor)
	{
		fail("Upscale factor out of range.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::validate_image_source_texture_2d_properties(
	const Texture2dProperties& properties)
try
{
	auto source_count = 0;

	if (properties.indexed_pixels_ != nullptr)
	{
		++source_count;
	}

	if (properties.indexed_sprite_ != nullptr)
	{
		++source_count;
	}

	if (properties.rgba_8_pixels_ != nullptr)
	{
		++source_count;
	}

	if (source_count == 0)
	{
		fail("No image source.");
	}

	if (source_count > 1)
	{
		fail("Multiple image source.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::validate_image_pixel_format_texture_2d_properties(
	const Texture2dProperties& properties)
try
{
	switch (properties.image_pixel_format_)
	{
		case Ren3dPixelFormat::rgba_8_unorm:
			return;

		default:
			fail("Invalid pixel format.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::validate_dimensions_texture_2d_properties(
	const Texture2dProperties& properties)
try
{
	if (properties.width_ <= 0)
	{
		fail("Invalid width.");
	}

	if (properties.height_ <= 0)
	{
		fail("Invalid height.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::validate_mipmap_texture_2d_properties(
	const Texture2dProperties& properties)
try
{
	if (properties.mipmap_count_ <= 0 ||
		properties.mipmap_count_ > Ren3dLimits::max_mipmap_count)
	{
		fail("Mipmap count out of range.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::validate_common_texture_2d_properties(
	const Texture2dProperties& properties)
try
{
	validate_image_source_texture_2d_properties(properties);
	validate_image_pixel_format_texture_2d_properties(properties);
	validate_dimensions_texture_2d_properties(properties);
	validate_mipmap_texture_2d_properties(properties);
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::validate_indexed_texture_2d_properties(
	const Texture2dProperties& properties)
try
{
	if (properties.indexed_pixels_ == nullptr)
	{
		fail("Null indexed image source.");
	}

	if (properties.indexed_palette_ == nullptr)
	{
		fail("Null indexed palette.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::validate_indexed_sprite_texture_2d_properties(
	const Texture2dProperties& properties)
try
{
	if (properties.indexed_sprite_ == nullptr)
	{
		fail("Null indexed sprite.");
	}

	if (properties.indexed_palette_ == nullptr)
	{
		fail("Null indexed palette.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::validate_rgba_8_texture_2d_properties(
	const Texture2dProperties& properties)
try
{
	if (properties.rgba_8_pixels_ == nullptr)
	{
		fail("Null RGBA image.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::validate_source_texture_2d_properties(
	const Texture2dProperties& properties)
try
{
	if (properties.indexed_pixels_ != nullptr)
	{
		validate_indexed_texture_2d_properties(properties);
	}
	else if (properties.indexed_sprite_ != nullptr)
	{
		validate_indexed_sprite_texture_2d_properties(properties);
	}
	else if (properties.rgba_8_pixels_ != nullptr)
	{
		validate_rgba_8_texture_2d_properties(properties);
	}
	else
	{
		fail("No image source.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::validate_texture_2d_properties(
	const Texture2dProperties& properties)
try
{
	validate_common_texture_2d_properties(properties);
	validate_source_texture_2d_properties(properties);
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::set_common_texture_2d_properties(
	Texture2dProperties& properties)
try
{
	const auto& device_features = renderer_->get_device_features();

	auto upscale_width = properties.width_;
	auto upscale_height = properties.height_;

	if (properties.indexed_pixels_ || properties.indexed_sprite_)
	{
		switch (upscale_filter_kind_)
		{
			case HwTextureMgrUpscaleFilterKind::xbrz:
				upscale_width *= upscale_filter_factor_;
				upscale_height *= upscale_filter_factor_;
				break;

			default:
				break;
		}
	}

	auto actual_width = upscale_width;
	auto actual_height = upscale_height;

	const auto is_width_pot = detail::Ren3dUtils::is_pot_value(actual_width);
	const auto is_height_pot = detail::Ren3dUtils::is_pot_value(actual_height);

	const auto is_npot = (!is_width_pot || !is_height_pot);
	const auto has_hw_npot = (!is_npot || (is_npot && device_features.is_npot_available_));

	if (has_hw_npot)
	{
		actual_width = std::min(actual_width, device_features.max_texture_dimension_);
		actual_height = std::min(actual_height, device_features.max_texture_dimension_);
	}
	else
	{
		actual_width = detail::Ren3dUtils::find_nearest_pot_value(actual_width);
		actual_width = std::min(actual_width, device_features.max_texture_dimension_);

		actual_height = detail::Ren3dUtils::find_nearest_pot_value(actual_height);
		actual_height = std::min(actual_height, device_features.max_texture_dimension_);
	}

	properties.upscale_width_ = upscale_width;
	properties.upscale_height_ = upscale_height;

	properties.actual_width_ = actual_width;
	properties.actual_height_ = actual_height;

	properties.is_npot_ = is_npot;

	if (properties.mipmap_count_ > 1)
	{
		properties.mipmap_count_ = detail::Ren3dUtils::calculate_mipmap_count(
			properties.upscale_width_,
			properties.upscale_height_
		);
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::upscale_xbrz(
	const Texture2dProperties& properties)
try
{
	const auto area = properties.width_ * properties.height_;

	if (mipmap_buffer_.size() < static_cast<std::size_t>(area))
	{
		mipmap_buffer_.clear();
		mipmap_buffer_.resize(area);
	}

	const auto upscale_area = properties.upscale_width_ * properties.upscale_height_;

	if (upscale_buffer_.size() < static_cast<std::size_t>(upscale_area))
	{
		upscale_buffer_.clear();
		upscale_buffer_.resize(upscale_area);
	}

	if (properties.indexed_pixels_)
	{
		auto param = detail::Ren3dUtils::IndexedToRgba8Param{};
		param.width_ = properties.width_;
		param.height_ = properties.height_;
		param.indexed_is_column_major_ = properties.indexed_is_column_major_;
		param.indexed_pixels_ = properties.indexed_pixels_;
		param.indexed_palette_ = properties.indexed_palette_;
		param.indexed_alphas_ = properties.indexed_alphas_;
		param.rgba_8_buffer_ = &mipmap_buffer_;

		detail::Ren3dUtils::indexed_to_rgba_8(param);
	}
	else if (properties.indexed_sprite_)
	{
		detail::Ren3dUtils::indexed_sprite_to_rgba_8_pot(
			*properties.indexed_sprite_,
			*properties.indexed_palette_,
			mipmap_buffer_
		);
	}
	else
	{
		fail("Unsupported image source.");
	}

	const auto lines_per_slice = 16;

	auto slice_count = properties.height_ / lines_per_slice;
	auto lines_remain = properties.height_ % lines_per_slice;

	if (slice_count > 0 && lines_remain > 0 && lines_remain < 8)
	{
		slice_count -= 1;
		lines_remain += lines_per_slice;
	}

	if (slice_count > 1)
	{
		xbrz_tasks_.clear();
		xbrz_tasks_.reserve(slice_count + 1);

		xbrz_task_ptrs_.clear();
		xbrz_task_ptrs_.reserve(slice_count + 1);

		auto line_index = 0;

		const auto src_lines = reinterpret_cast<const std::uint32_t*>(mipmap_buffer_.data());
		const auto dst_lines = reinterpret_cast<std::uint32_t*>(upscale_buffer_.data());

		for (int i = 0; i < slice_count; ++i)
		{
			xbrz_tasks_.emplace_back();
			auto& xbrz_task = xbrz_tasks_.back();

			xbrz_task_ptrs_.emplace_back(&xbrz_task);

			xbrz_task.initialize(
				upscale_filter_factor_,
				line_index,
				line_index + lines_per_slice,
				properties.width_,
				properties.height_,
				src_lines,
				dst_lines
			);

			line_index += lines_per_slice;
		}

		if (lines_remain > 0)
		{
			xbrz_tasks_.emplace_back();
			auto& xbrz_task = xbrz_tasks_.back();

			xbrz_task_ptrs_.emplace_back(&xbrz_task);

			xbrz_task.initialize(
				upscale_filter_factor_,
				line_index,
				properties.height_,
				properties.width_,
				properties.height_,
				src_lines,
				dst_lines
			);
		}

		mt_task_manager_->add_tasks_and_wait_for_added(
			xbrz_task_ptrs_.data(),
			static_cast<int>(xbrz_task_ptrs_.size())
		);
	}
	else
	{
		xbrz::scale(
			static_cast<std::size_t>(upscale_filter_factor_),
			reinterpret_cast<const std::uint32_t*>(mipmap_buffer_.data()),
			reinterpret_cast<std::uint32_t*>(upscale_buffer_.data()),
			properties.width_,
			properties.height_,
			xbrz::ColorFormat::ARGB
		);
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::upscale(
	const Texture2dProperties& properties)
try
{
	if (properties.upscale_width_ == properties.width_ &&
		properties.upscale_height_ == properties.height_)
	{
		return;
	}

	switch (upscale_filter_kind_)
	{
		case HwTextureMgrUpscaleFilterKind::none:
			break;

		case HwTextureMgrUpscaleFilterKind::xbrz:
			upscale_xbrz(properties);
			break;

		default:
			fail("Invalid upscale filter type.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

HwTextureMgrImpl::Texture2dItem HwTextureMgrImpl::create_texture(
	const Texture2dProperties& properties)
try
{
	validate_texture_2d_properties(properties);

	auto new_properties = properties;

	set_common_texture_2d_properties(new_properties);

	// Create texture object.
	//
	auto param = Ren3dCreateTexture2dParam{};
	param.pixel_format_ = new_properties.image_pixel_format_;
	param.width_ = new_properties.actual_width_;
	param.height_ = new_properties.actual_height_;
	param.mipmap_count_ = new_properties.mipmap_count_;

	auto texture_2d = renderer_->create_texture_2d(param);

	update_mipmaps(new_properties, texture_2d);

	// Return the result.
	//
	auto result = Texture2dItem{};

	result.properties_ = new_properties;
	result.texture_2d_ = std::move(texture_2d);

	return result;
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::update_mipmaps(
	const Texture2dProperties& properties,
	const Ren3dTexture2dUPtr& texture_2d)
try
{
	upscale(properties);

	const auto is_upscale = (
		properties.upscale_width_ != properties.width_ &&
		properties.upscale_height_ != properties.height_);

	const auto& device_features = renderer_->get_device_features();

	const auto npot_is_available = device_features.is_npot_available_;

	const auto max_subbuffer_size = properties.actual_width_ * properties.actual_height_;

	auto max_buffer_size = max_subbuffer_size;

	const auto is_manual_mipmaps =
		properties.is_generate_mipmaps_ &&
		properties.mipmap_count_ > 1 &&
		!device_features.is_mipmap_available_;

	if (is_manual_mipmaps)
	{
		max_buffer_size *= 2;
	}

	if (static_cast<int>(mipmap_buffer_.size()) < max_buffer_size)
	{
		mipmap_buffer_.clear();
		mipmap_buffer_.resize(max_buffer_size);
	}

	auto texture_subbuffer_0 = &mipmap_buffer_[0];
	auto texture_subbuffer_1 = Rgba8Ptr{};

	if (is_manual_mipmaps)
	{
		texture_subbuffer_1 = &mipmap_buffer_[max_subbuffer_size];
	}

	auto is_set_subbuffer_0 = false;

	if (is_upscale)
	{
		if (properties.is_npot_ && !npot_is_available)
		{
			detail::Ren3dUtils::rgba_8_npot_to_rgba_8_pot(
				properties.upscale_width_,
				properties.upscale_height_,
				properties.actual_width_,
				properties.actual_height_,
				upscale_buffer_.data(),
				mipmap_buffer_
			);
		}
		else
		{
			// Don't copy the base mipmap into a buffer.

			is_set_subbuffer_0 = true;

			texture_subbuffer_0 = upscale_buffer_.data();
		}
	}
	else if (properties.rgba_8_pixels_)
	{
		if (properties.is_npot_ && !npot_is_available)
		{
			detail::Ren3dUtils::rgba_8_npot_to_rgba_8_pot(
				properties.width_,
				properties.height_,
				properties.actual_width_,
				properties.actual_height_,
				properties.rgba_8_pixels_,
				mipmap_buffer_
			);
		}
		else
		{
			// Don't copy the base mipmap into a buffer.

			is_set_subbuffer_0 = true;

			texture_subbuffer_0 = const_cast<Rgba8Ptr>(properties.rgba_8_pixels_);
		}
	}
	else if (properties.indexed_pixels_)
	{
		auto param = detail::Ren3dUtils::IndexedToRgba8Param{};

		param.width_ = properties.width_;
		param.height_ = properties.height_;
		param.actual_width_ = properties.actual_width_;
		param.actual_height_ = properties.actual_height_;
		param.indexed_is_column_major_ = properties.indexed_is_column_major_;
		param.indexed_pixels_ = properties.indexed_pixels_;
		param.indexed_palette_ = properties.indexed_palette_;
		param.indexed_alphas_ = properties.indexed_alphas_;
		param.rgba_8_buffer_ = &mipmap_buffer_;

		detail::Ren3dUtils::indexed_to_rgba_8_pot(param);
	}
	else if (properties.indexed_sprite_)
	{
		detail::Ren3dUtils::indexed_sprite_to_rgba_8_pot(
			*properties.indexed_sprite_,
			*properties.indexed_palette_,
			mipmap_buffer_
		);
	}

	auto mipmap_width = properties.actual_width_;
	auto mipmap_height = properties.actual_height_;

	auto mipmap_count = properties.mipmap_count_;

	if (properties.is_generate_mipmaps_ &&
		properties.mipmap_count_ > 1 &&
		device_features.is_mipmap_available_)
	{
		mipmap_count = 1;
	}

	for (int i_mipmap = 0; i_mipmap < mipmap_count; ++i_mipmap)
	{
		if (i_mipmap > 0)
		{
			detail::Ren3dUtils::build_mipmap(
				mipmap_width,
				mipmap_height,
				texture_subbuffer_0,
				texture_subbuffer_1);

			if (mipmap_width > 1)
			{
				mipmap_width /= 2;
			}

			if (mipmap_height > 1)
			{
				mipmap_height /= 2;
			}

			if (is_set_subbuffer_0)
			{
				is_set_subbuffer_0 = false;

				texture_subbuffer_0 = &mipmap_buffer_[0];
			}

			std::swap(texture_subbuffer_0, texture_subbuffer_1);
		}

		auto param = Ren3dTexture2dUpdateParam{};
		param.mipmap_level_ = i_mipmap;
		param.image_ = texture_subbuffer_0;

		texture_2d->update(param);
	}

	if (properties.is_generate_mipmaps_ &&
		properties.mipmap_count_ > 1 &&
		device_features.is_mipmap_available_)
	{
		texture_2d->generate_mipmaps();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::destroy_missing_sprite_texture() noexcept
{
	missing_sprite_texture_2d_item_.texture_2d_ = nullptr;
}

void HwTextureMgrImpl::create_missing_sprite_texture()
try
{
	destroy_missing_sprite_texture();

	const auto& raw_image = get_missing_sprite_image();
	const auto rgba_8_image = reinterpret_cast<const Rgba8*>(raw_image.data());

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = Ren3dPixelFormat::rgba_8_unorm;
	param.width_ = Sprite::dimension;
	param.height_ = Sprite::dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::Ren3dUtils::calculate_mipmap_count(Sprite::dimension, Sprite::dimension);
	param.rgba_8_pixels_ = rgba_8_image;

	auto texture_2d_item = create_texture(param);

	missing_sprite_texture_2d_item_.properties_ = texture_2d_item.properties_;
	missing_sprite_texture_2d_item_.texture_2d_ = std::move(texture_2d_item.texture_2d_);

	update_mipmaps(missing_sprite_texture_2d_item_.properties_, missing_sprite_texture_2d_item_.texture_2d_);
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::destroy_missing_wall_texture() noexcept
{
	missing_wall_texture_2d_item_.texture_2d_ = nullptr;
}

void HwTextureMgrImpl::create_missing_wall_texture()
try
{
	destroy_missing_wall_texture();

	const auto& raw_image = get_missing_wall_image();
	const auto rgba_8_image = reinterpret_cast<const Rgba8*>(raw_image.data());

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = Ren3dPixelFormat::rgba_8_unorm;
	param.width_ = wall_dimension;
	param.height_ = wall_dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::Ren3dUtils::calculate_mipmap_count(param.width_, param.height_);
	param.rgba_8_pixels_ = rgba_8_image;

	auto texture_2d_item = create_texture(param);

	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	missing_wall_texture_2d_item_ = std::move(texture_2d_item);
}
catch (...)
{
	fail_nested(__func__);
}

HwTextureMgrImpl::Texture2dItem HwTextureMgrImpl::create_from_external_image(
	int id,
	ImageKind kind)
try
{
	if (id < 0 || id > 99'999'999)
	{
		fail("Id out of range.");
	}

	using PathNameMaker = void (*)(
		int id,
		std::string& data_path,
		std::string& mod_path);

	auto path_name_maker = PathNameMaker{};

	switch (kind)
	{
		case ImageKind::sprite:
			path_name_maker = ca_make_sprite_resource_path_name;
			break;

		case ImageKind::wall:
			path_name_maker = ca_make_wall_resource_path_name;
			break;

		default:
			fail("Unsupported image kind.");
	}

	path_name_maker(id, image_data_path_, image_mod_path_);

	for (const auto& image_probe_item : image_probe_items_)
	{
		image_file_stream_.close();

		if (!image_file_stream_.is_open() && !image_mod_path_.empty())
		{
			file_system::replace_extension(image_mod_path_, image_probe_item.file_name_extension);
			image_file_stream_.open(image_mod_path_);
		}

		if (!image_file_stream_.is_open() && !image_data_path_.empty())
		{
			file_system::replace_extension(image_data_path_, image_probe_item.file_name_extension);
			image_file_stream_.open(image_data_path_);
		}

		if (!image_file_stream_.is_open())
		{
			continue;
		}

		const auto image_file_size = image_file_stream_.get_size();

		if (image_file_size <= 0)
		{
			continue;
		}

		if (image_buffer_.size() < static_cast<std::size_t>(image_file_size))
		{
			image_buffer_.resize(static_cast<std::size_t>(image_file_size));
		}

		const auto image_to_read_size = static_cast<int>(image_file_size);
		const auto image_read_result = image_file_stream_.read(image_buffer_.data(), image_to_read_size);

		if (image_read_result != image_to_read_size)
		{
			continue;
		}

		auto width = 0;
		auto height = 0;

		try
		{
			image_probe_item.image_decoder->decode(
				image_buffer_.data(),
				image_read_result,
				width,
				height,
				image_buffer_rgba8_
			);

			auto param = Texture2dProperties{};
			param.image_pixel_format_ = Ren3dPixelFormat::rgba_8_unorm;
			param.width_ = width;
			param.height_ = height;
			param.is_generate_mipmaps_ = true;
			param.mipmap_count_ = detail::Ren3dUtils::calculate_mipmap_count(param.width_, param.height_);
			param.rgba_8_pixels_ = image_buffer_rgba8_.data();

			auto texture_2d_item = create_texture(param);

			update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

			return texture_2d_item;
		}
		catch (...)
		{
			continue;
		}
	}

	return Texture2dItem{};
}
catch (...)
{
	fail_nested(__func__);
}

HwTextureMgrImpl::Texture2dItem HwTextureMgrImpl::wall_create_texture(
	const int wall_id)
try
{
	const auto indexed_pixels = bstone::globals::page_mgr->get(wall_id);

	if (!indexed_pixels)
	{
		fail("Null data.");
	}

	if (is_external_textures_enabled_)
	{
		auto external_texture_2d_item = create_from_external_image(wall_id, ImageKind::wall);

		if (external_texture_2d_item.texture_2d_)
		{
			return external_texture_2d_item;
		}
	}

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = Ren3dPixelFormat::rgba_8_unorm;
	param.width_ = wall_dimension;
	param.height_ = wall_dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::Ren3dUtils::calculate_mipmap_count(param.width_, param.height_);
	param.indexed_is_column_major_ = true;
	param.indexed_pixels_ = indexed_pixels;
	param.indexed_palette_ = &vid_hw_get_default_palette();

	auto texture_2d_item = create_texture(param);

	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	return texture_2d_item;
}
catch (...)
{
	fail_nested(__func__);
}

HwTextureMgrImpl::Texture2dItem HwTextureMgrImpl::sprite_create_texture(
	const int sprite_id)
try
{
	auto sprite = sprite_cache_->cache(sprite_id);

	if (!sprite)
	{
		fail(("Failed to cache a sprite #" + std::to_string(sprite_id) + ".").c_str());
	}

	if (!sprite->is_initialized())
	{
		fail(("Sprite #" + std::to_string(sprite_id) + " not initialized.").c_str());
	}

	if (is_external_textures_enabled_)
	{
		auto external_texture_2d_item = create_from_external_image(sprite_id, ImageKind::sprite);

		if (external_texture_2d_item.texture_2d_)
		{
			return external_texture_2d_item;
		}
	}

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = Ren3dPixelFormat::rgba_8_unorm;
	param.width_ = Sprite::dimension;
	param.height_ = Sprite::dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::Ren3dUtils::calculate_mipmap_count(param.width_, param.height_);
	param.indexed_sprite_ = sprite;
	param.indexed_palette_ = &vid_hw_get_default_palette();

	auto texture_2d_item = create_texture(param);

	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	return texture_2d_item;
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::initialize_internal(
	Ren3dPtr renderer,
	SpriteCachePtr cache_sprite)
try
{
	if (!renderer)
	{
		fail("Null renderer.");
	}

	if (!cache_sprite)
	{
		fail("Null sprite cache.");
	}

	if (!mt_task_manager_)
	{
		fail("Null task manager.");
	}

	renderer_ = renderer;
	sprite_cache_ = cache_sprite;

	create_missing_sprite_texture();
	create_missing_wall_texture();

	generation_id_ = first_generation_id;

	wall_map_.reserve(max_walls);
	sprite_map_.reserve(max_sprites);

	upscale_filter_factor_ = upscale_filter_clamp_factor(upscale_filter_kind_, upscale_filter_factor_);

	bmp_image_decoder_ = make_image_decoder(ImageDecoderType::bmp);
	png_image_decoder_ = make_image_decoder(ImageDecoderType::png);

	image_probe_items_ = ExternalImageProbeItems
	{
		ExternalImageProbeItem{".png", png_image_decoder_.get()},
		ExternalImageProbeItem{".bmp", bmp_image_decoder_.get()},
	};
}
catch (...)
{
	fail_nested(__func__);
}

void HwTextureMgrImpl::purge_cache(
	IdToTexture2dMap& map)
try
{
	for (auto map_it = map.begin(); map_it != map.end(); )
	{
		auto& texture_2d_item = map_it->second;

		if (texture_2d_item.generation_id_ != generation_id_)
		{
			texture_2d_item.texture_2d_ = nullptr;

			map_it = map.erase(map_it);
		}
		else
		{
			++map_it;
		}
	}
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dTexture2dPtr HwTextureMgrImpl::get_texture_2d(
	const ImageKind image_kind,
	const int id,
	const IdToTexture2dMap& map) const noexcept
{
	auto item_it = map.find(id);

	if (item_it == map.end())
	{
		switch (image_kind)
		{
		case ImageKind::sprite:
			return missing_sprite_texture_2d_item_.texture_2d_.get();

		case ImageKind::wall:
			return missing_wall_texture_2d_item_.texture_2d_.get();

		default:
			return nullptr;
		}
	}

	return item_it->second.texture_2d_.get();
}

void HwTextureMgrImpl::solid_1x1_destroy_all() noexcept
{
	for (int i = 0; i < static_cast<int>(HwTextureMgrSolid1x1Id::count_); ++i)
	{
		const auto id = static_cast<HwTextureMgrSolid1x1Id>(i);

		destroy_solid_1x1(id);
	}
}

int HwTextureMgrImpl::solid_1x1_try_get_index(
	const HwTextureMgrSolid1x1Id id) noexcept
{
	switch (id)
	{
		case HwTextureMgrSolid1x1Id::black:
		case HwTextureMgrSolid1x1Id::white:
		case HwTextureMgrSolid1x1Id::fade_2d:
		case HwTextureMgrSolid1x1Id::fade_3d:
		case HwTextureMgrSolid1x1Id::flooring:
		case HwTextureMgrSolid1x1Id::ceiling:
			return static_cast<int>(id);

		default:
			assert(!"Invalid solid 1x1 texture id.");
			return -1;
	}
}

int HwTextureMgrImpl::solid_1x1_get_index(
	const HwTextureMgrSolid1x1Id id)
try
{
	switch (id)
	{
		case HwTextureMgrSolid1x1Id::black:
		case HwTextureMgrSolid1x1Id::white:
		case HwTextureMgrSolid1x1Id::fade_2d:
		case HwTextureMgrSolid1x1Id::fade_3d:
		case HwTextureMgrSolid1x1Id::flooring:
		case HwTextureMgrSolid1x1Id::ceiling:
			return static_cast<int>(id);

		default:
			fail("Unsupported solid 1x1 texture kind.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

int HwTextureMgrImpl::solid_1x1_get_updateable_index(
	const HwTextureMgrSolid1x1Id id)
try
{
	switch (id)
	{
		case HwTextureMgrSolid1x1Id::fade_2d:
		case HwTextureMgrSolid1x1Id::fade_3d:
		case HwTextureMgrSolid1x1Id::flooring:
		case HwTextureMgrSolid1x1Id::ceiling:
			return static_cast<int>(id);

		default:
			fail("Unsupported solid 1x1 texture kind.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

Rgba8 HwTextureMgrImpl::solid_1x1_get_default_color(
	const HwTextureMgrSolid1x1Id id)
try
{
	switch (id)
	{
		case HwTextureMgrSolid1x1Id::black:
			return Rgba8{0x00, 0x00, 0x00, 0xFF};

		case HwTextureMgrSolid1x1Id::white:
			return Rgba8{0xFF, 0xFF, 0xFF, 0xFF};

		case HwTextureMgrSolid1x1Id::fade_2d:
		case HwTextureMgrSolid1x1Id::fade_3d:
			return Rgba8{0x00, 0x00, 0x00, 0xFF};

		case HwTextureMgrSolid1x1Id::flooring:
		case HwTextureMgrSolid1x1Id::ceiling:
			return Rgba8{0x00, 0x00, 0x00, 0xFF};

		default:
			fail("Invalid color id.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

int HwTextureMgrImpl::upscale_filter_get_min_factor_internal(
	const HwTextureMgrUpscaleFilterKind upscale_filter_kind)
try
{
	switch (upscale_filter_kind)
	{
		case HwTextureMgrUpscaleFilterKind::none:
			return 1;

		case HwTextureMgrUpscaleFilterKind::xbrz:
			return 2;

		default:
			fail("Unsupported upscale filter kind.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

int HwTextureMgrImpl::upscale_filter_get_max_factor_internal(
	const HwTextureMgrUpscaleFilterKind upscale_filter_kind)
try
{
	switch (upscale_filter_kind)
	{
		case HwTextureMgrUpscaleFilterKind::none:
			return 1;

		case HwTextureMgrUpscaleFilterKind::xbrz:
			return xbrz::SCALE_FACTOR_MAX;

		default:
			fail("Unsupported upscale filter kind.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

int HwTextureMgrImpl::upscale_filter_clamp_factor(
	const HwTextureMgrUpscaleFilterKind upscale_filter_kind,
	const int upscale_filter_factor)
try
{
	auto result = upscale_filter_factor;


	const auto min_result = upscale_filter_get_min_factor_internal(upscale_filter_kind);

	if (result < min_result)
	{
		result = min_result;
	}


	const auto max_result = upscale_filter_get_max_factor_internal(upscale_filter_kind);

	if (result > max_result)
	{
		result = max_result;
	}

	return result;
}
catch (...)
{
	fail_nested(__func__);
}

//
// HwTextureMgrImpl
// ==========================================================================


// ==========================================================================
// HwTextureMgrFactory
//

HwTextureMgrUPtr HwTextureMgrFactory::create(
	const Ren3dPtr renderer,
	const SpriteCachePtr cache_sprite,
	const MtTaskMgrPtr mt_task_manager)
{
	return std::make_unique<HwTextureMgrImpl>(renderer, cache_sprite, mt_task_manager);
}

//
// HwTextureMgrFactory
// ==========================================================================


} // bstone
