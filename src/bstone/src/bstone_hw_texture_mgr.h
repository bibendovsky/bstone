/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Texture manager (hardware).
//


#ifndef BSTONE_HW_TEXTURE_MGR_INCLUDED
#define BSTONE_HW_TEXTURE_MGR_INCLUDED


#include <memory>

#include "bstone_r3r.h"
#include "bstone_rgb_palette.h"


namespace bstone
{


class SpriteCache;
using SpriteCachePtr = SpriteCache*;

class MtTaskMgr;
using MtTaskMgrPtr = MtTaskMgr*;


enum class HwTextureMgrSolid1x1Id
{
	black,
	white,
	fade_2d,
	fade_3d,
	flooring,
	ceiling,

	// Item count.
	count_,
}; // HwTextureMgrSolid1x1Id

enum class HwTextureMgrUpscaleFilterType
{
	none,
	xbrz,
}; // HwTextureMgrUpscaleFilterType


class HwTextureMgr
{
public:
	HwTextureMgr() noexcept;

	virtual ~HwTextureMgr();


	virtual int get_min_upscale_filter_degree(
		const HwTextureMgrUpscaleFilterType upscale_filter_type) const = 0;

	virtual int get_max_upscale_filter_degree(
		const HwTextureMgrUpscaleFilterType upscale_filter_type) const = 0;

	virtual HwTextureMgrUpscaleFilterType get_upscale_filter_type() const noexcept = 0;

	virtual int get_upscale_filter_degree() const noexcept = 0;

	virtual void set_upscale_filter(
		const HwTextureMgrUpscaleFilterType upscale_filter_type,
		const int upscale_filter_degree) = 0;

	virtual void enable_external_textures(
		bool is_enable) = 0;


	virtual void begin_cache() = 0;

	virtual void end_cache() = 0;

	virtual void purge_cache() = 0;


	virtual void cache_wall(
		const int id) = 0;

	virtual R3rR2Texture* get_wall(
		const int id) const = 0;


	virtual void cache_sprite(
		const int id) = 0;

	virtual R3rR2Texture* get_sprite(
		const int id) const = 0;


	virtual void destroy_ui() = 0;

	virtual void create_ui(
		const std::uint8_t* const indexed_pixels,
		const bool* const indexed_alphas,
		const Rgba8PaletteCPtr indexed_palette) = 0;

	virtual void update_ui() = 0;

	virtual R3rR2Texture* get_ui() const noexcept = 0;


	virtual void try_destroy_solid_1x1(
		const HwTextureMgrSolid1x1Id id) noexcept = 0;

	virtual void destroy_solid_1x1(
		const HwTextureMgrSolid1x1Id id) = 0;

	virtual void create_solid_1x1(
		const HwTextureMgrSolid1x1Id id) = 0;

	virtual void update_solid_1x1(
		const HwTextureMgrSolid1x1Id id,
		const Rgba8 color) = 0;

	virtual R3rR2Texture* get_solid_1x1(
		const HwTextureMgrSolid1x1Id id) const = 0;
}; // HwTextureMgr

using HwTextureMgrUPtr = std::unique_ptr<HwTextureMgr>;


HwTextureMgrUPtr make_hw_texture_mgr(
	R3r* renderer_3d,
	const SpriteCachePtr sprite_cache,
	const MtTaskMgrPtr task_manager);


} // bstone


#endif // !BSTONE_HW_TEXTURE_MGR_INCLUDED
