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
// Renderer texture manager.
//


#ifndef BSTONE_HW_TEXTURE_MANAGER_INCLUDED
#define BSTONE_HW_TEXTURE_MANAGER_INCLUDED


#include <memory>
#include "bstone_rgb_palette.h"


namespace bstone
{


class Renderer;
using RendererPtr = Renderer*;

class RendererTexture2d;
using RendererTexture2dPtr = RendererTexture2d*;

class SpriteCache;
using SpriteCachePtr = SpriteCache*;

class MtTaskManager;
using MtTaskManagerPtr = MtTaskManager*;


enum class HwTextureManagerSolid1x1Id
{
	black,
	white,
	fade_2d,
	fade_3d,
	flooring,
	ceiling,

	// Item count.
	count_,
}; // HwTextureManagerSolid1x1Id

enum class HwTextureManagerUpscaleFilterKind
{
	none,
	xbrz,
}; // HwTextureManagerUpscaleFilterKind


class HwTextureManager
{
protected:
	HwTextureManager() = default;


public:
	virtual ~HwTextureManager() = default;


	virtual int upscale_filter_get_min_factor(
		const HwTextureManagerUpscaleFilterKind upscale_filter_kind) const = 0;

	virtual int upscale_filter_get_max_factor(
		const HwTextureManagerUpscaleFilterKind upscale_filter_kind) const = 0;

	virtual HwTextureManagerUpscaleFilterKind upscale_filter_get_kind() const noexcept = 0;

	virtual int upscale_filter_get_factor() const noexcept = 0;

	virtual void upscale_filter_set(
		const HwTextureManagerUpscaleFilterKind upscale_filter_kind,
		const int upscale_filter_factor) = 0;


	virtual void cache_begin() = 0;

	virtual void cache_end() = 0;

	virtual void cache_purge() = 0;


	virtual void wall_cache(
		const int id) = 0;

	virtual RendererTexture2dPtr wall_get(
		const int id) const = 0;


	virtual void sprite_cache(
		const int id) = 0;

	virtual RendererTexture2dPtr sprite_get(
		const int id) const = 0;


	virtual void ui_destroy() = 0;

	virtual void ui_create(
		const std::uint8_t* const indexed_pixels,
		const bool* const indexed_alphas,
		const R8g8b8a8PaletteCPtr indexed_palette) = 0;

	virtual void ui_update() = 0;

	virtual RendererTexture2dPtr ui_get() const = 0;


	virtual void solid_1x1_destroy(
		const HwTextureManagerSolid1x1Id id) = 0;

	virtual void solid_1x1_create(
		const HwTextureManagerSolid1x1Id id) = 0;

	virtual void solid_1x1_update(
		const HwTextureManagerSolid1x1Id id,
		const R8g8b8a8 color) = 0;

	virtual RendererTexture2dPtr solid_1x1_get(
		const HwTextureManagerSolid1x1Id id) const = 0;


	virtual void device_on_reset() = 0;
}; // HwTextureManager

using HwTextureManagerPtr = HwTextureManager*;
using HwTextureManagerUPtr = std::unique_ptr<HwTextureManager>;


class HwTextureManagerFactory
{
public:
	static HwTextureManagerUPtr create(
		const RendererPtr renderer,
		const SpriteCachePtr sprite_cache,
		const MtTaskManagerPtr mt_task_manager);
}; // HwTextureManagerFactory


} // bstone


#endif // !BSTONE_HW_TEXTURE_MANAGER_INCLUDED
