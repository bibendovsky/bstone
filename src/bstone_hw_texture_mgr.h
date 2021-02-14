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
// Texture manager (hardware).
//


#ifndef BSTONE_HW_TEXTURE_MGR_INCLUDED
#define BSTONE_HW_TEXTURE_MGR_INCLUDED


#include <memory>

#include "bstone_ren_3d.h"
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

enum class HwTextureMgrUpscaleFilterKind
{
	none,
	xbrz,
}; // HwTextureMgrUpscaleFilterKind


class HwTextureMgr
{
protected:
	HwTextureMgr() = default;


public:
	virtual ~HwTextureMgr() = default;


	virtual int get_min_upscale_filter_degree(
		const HwTextureMgrUpscaleFilterKind upscale_filter_kind) const = 0;

	virtual int get_max_upscale_filter_degree(
		const HwTextureMgrUpscaleFilterKind upscale_filter_kind) const = 0;

	virtual HwTextureMgrUpscaleFilterKind get_upscale_filter_kind() const noexcept = 0;

	virtual int get_upscale_filter_degree() const noexcept = 0;

	virtual void set_upscale_filter(
		const HwTextureMgrUpscaleFilterKind upscale_filter_kind,
		const int upscale_filter_degree) = 0;

	virtual void enable_external_textures(
		bool is_enable) = 0;


	virtual void begin_cache() = 0;

	virtual void end_cache() = 0;

	virtual void purge_cache() = 0;


	virtual void cache_wall(
		const int id) = 0;

	virtual Ren3dTexture2dPtr get_wall(
		const int id) const = 0;


	virtual void cache_sprite(
		const int id) = 0;

	virtual Ren3dTexture2dPtr get_sprite(
		const int id) const = 0;


	virtual void destroy_ui() = 0;

	virtual void create_ui(
		const std::uint8_t* const indexed_pixels,
		const bool* const indexed_alphas,
		const Rgba8PaletteCPtr indexed_palette) = 0;

	virtual void update_ui() = 0;

	virtual Ren3dTexture2dPtr get_ui() const = 0;


	virtual void destroy_solid_1x1(
		const HwTextureMgrSolid1x1Id id) = 0;

	virtual void create_solid_1x1(
		const HwTextureMgrSolid1x1Id id) = 0;

	virtual void update_solid_1x1(
		const HwTextureMgrSolid1x1Id id,
		const Rgba8 color) = 0;

	virtual Ren3dTexture2dPtr get_solid_1x1(
		const HwTextureMgrSolid1x1Id id) const = 0;
}; // HwTextureMgr

using HwTextureMgrUPtr = std::unique_ptr<HwTextureMgr>;


class HwTextureMgrFactory
{
public:
	static HwTextureMgrUPtr create(
		const Ren3dPtr renderer_3d,
		const SpriteCachePtr sprite_cache,
		const MtTaskMgrPtr task_manager);
}; // HwTextureMgrFactory


} // bstone


#endif // !BSTONE_HW_TEXTURE_MGR_INCLUDED
