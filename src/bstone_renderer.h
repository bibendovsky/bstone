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
// Renderer.
//


#ifndef BSTONE_RENDERER_INCLUDED
#define BSTONE_RENDERER_INCLUDED


#include <array>
#include <memory>
#include <string>
#include <vector>
#include "bstone_renderer_color_32.h"
#include "bstone_sprite.h"


namespace bstone
{


enum class RendererKind :
	unsigned char
{
	none,
	opengl,
}; // RendererKind

enum class RendererPath :
	unsigned char
{
	none,

	autodetect,

	ogl_1_x,
}; // RendererPath

enum class RendererCommandId :
	unsigned char
{
	none,

	viewport_set,

	scissor_enable,
	scissor_set_box,

	culling_enable,

	depth_set_test,
	depth_set_write,

	blending_enable,
	blending_function,

	fog_enable,
	fog_set_color,
	fog_set_distances,

	matrix_set_model,
	matrix_set_view,
	matrix_set_model_view,
	matrix_set_projection,

	texture_set,
	sampler_set,

	vertex_input_set,

	draw_quads,
}; // RendererCommandId

enum class RendererCullingFace :
	unsigned char
{
	counter_clockwise,
	clockwise,
}; // RendererCullingFace

enum class RendererCullingMode :
	unsigned char
{
	back,
	front,
	both,
}; // RendererCullingMode

enum class RendererBlendingFactor :
	unsigned char
{
	zero,
	one,
	src_color,
	src_alpha,
	one_minus_src_alpha,
}; // RendererBlendingFactor

enum class RendererAaKind
{
	none,
	ms,
}; // RendererAaKind


class RendererInitializeWindowParam
{
public:
	bool is_visible_;
	bool is_borderless_;
	bool is_fullscreen_desktop_;
	bool is_positioned_;

	int x_;
	int y_;

	int width_;
	int height_;

	std::string title_utf8_;
}; // RendererInitializeWindowParam

class RendererInitializeParam
{
public:
	RendererPath renderer_path_;
	RendererInitializeWindowParam window_;

	RendererAaKind aa_kind_;
	int aa_value_;
}; // RendererInitializeParam


using RendererPalette = std::array<RendererColor32, 256>;
using RendererPalettePtr = RendererPalette*;
using RendererPaletteCPtr = const RendererPalette*;


// ==========================================================================
// RendererIndexBuffer
//

enum class RendererIbElementTypeId :
	unsigned char
{
	none,
	uint8,
	uint16,
	uint32,
}; // RendererIbElementTypeId

template<RendererIbElementTypeId TTypeId>
using RendererIbTypeT = std::conditional_t<
	TTypeId == RendererIbElementTypeId::uint8,
	std::uint8_t,
	std::conditional_t<
		TTypeId == RendererIbElementTypeId::uint16,
		std::uint16_t,
		std::conditional_t<
			TTypeId == RendererIbElementTypeId::uint32,
			std::uint32_t,
			void
		>
	>
>;

struct RendererIndexBufferCreateParam
{
	int byte_depth_;
	int size_;
}; // RendererIndexBufferCreateParam

struct RendererIndexBufferUpdateParam
{
	int offset_;
	int size_;
	const void* data_;
}; // RendererIndexBufferUpdateParam

class RendererIndexBuffer
{
protected:
	RendererIndexBuffer() = default;

	virtual ~RendererIndexBuffer() = default;


public:
	virtual void update(
		const RendererIndexBufferUpdateParam& param) = 0;
}; // RendererIndexBuffer

using RendererIndexBufferPtr = RendererIndexBuffer*;

//
// RendererIndexBuffer
// ==========================================================================


// ==========================================================================
// RendererVertexBuffer
//

struct RendererVertexBufferCreateParam
{
	int size_;
}; // RendererVertexBufferCreateParam

struct RendererVertexBufferUpdateParam
{
	int offset_;
	int size_;
	const void* data_;
}; // RendererVertexBufferUpdateParam

class RendererVertexBuffer
{
protected:
	RendererVertexBuffer() = default;

	virtual ~RendererVertexBuffer() = default;


public:
	virtual void update(
		const RendererVertexBufferUpdateParam& param) = 0;
}; // RendererVertexBuffer

using RendererVertexBufferPtr = RendererVertexBuffer*;

//
// RendererVertexBuffer
// ==========================================================================


// ==========================================================================
// RendererVertexInput
//

enum class RendererVertexAttributeLocation :
	unsigned char
{
	none,
	position,
	color,
	texture_coordinates,
}; // RendererVertexAttributeLocation

enum class RendererVertexAttributeFormat :
	unsigned char
{
	none,
	r8g8b8a8_unorm,
	r32g32_sfloat,
	r32g32b32_sfloat,
}; // RendererVertexAttributeFormat

struct RendererVertexAttributeDescription
{
	RendererVertexAttributeLocation location_;
	RendererVertexAttributeFormat format_;
	RendererVertexBufferPtr vertex_buffer_;
	int offset_;
	int stride_;
}; // RendererVertexAttributeDescription

using RendererVertexAttributeDescriptions = std::vector<RendererVertexAttributeDescription>;

struct RendererVertexInputCreateParam
{
	RendererIndexBufferPtr index_buffer_;
	RendererVertexAttributeDescriptions attribute_descriptions_;
}; // RendererVertexInputCreateParam


class RendererVertexInput
{
protected:
	RendererVertexInput() = default;

	virtual ~RendererVertexInput() = default;
}; // RendererVertexInput

using RendererVertexInputPtr = RendererVertexInput*;

//
// RendererVertexInput
// ==========================================================================


// ==========================================================================
// RendererTexture2d
//

enum class RendererFilterKind :
	unsigned char
{
	nearest,
	linear,
}; // RendererFilterKind

enum class RendererMipmapMode :
	unsigned char
{
	none,
	nearest,
	linear,
}; // RendererMipmapMode

enum class RendererAddressMode :
	unsigned char
{
	clamp,
	repeat,
}; // RendererAddressMode

struct RendererSamplerState
{
	RendererFilterKind mag_filter_;
	RendererFilterKind min_filter_;

	RendererMipmapMode mipmap_mode_;

	RendererAddressMode address_mode_u_;
	RendererAddressMode address_mode_v_;

	int anisotropy_;
}; // RendererSamplerState

enum class RendererPixelFormat
{
	none,
	r8g8b8_unorm,
	r8g8b8a8_unorm,
}; // RendererPixelFormat

struct RendererTexture2dCreateParam
{
	RendererPixelFormat storage_pixel_format_;

	int width_;
	int height_;

	int mipmap_count_;
}; // RendererTexture2dCreateParam

struct RendererTexture2dUpdateParam
{
	int mipmap_level_;

	RendererColor32CPtr rgba_pixels_;
}; // RendererTexture2dUpdateParam

class RendererTexture2d
{
protected:
	RendererTexture2d() = default;

	virtual ~RendererTexture2d() = default;


public:
	virtual void update(
		const RendererTexture2dUpdateParam& param) = 0;

	virtual void generate_mipmaps() = 0;
}; // RendererTexture2d

using RendererTexture2dPtr = RendererTexture2d*;


struct RendererSamplerCreateParam
{
	RendererSamplerState state_;
}; // RendererSamplerCreateParam

struct RendererSamplerUpdateParam
{
	RendererSamplerState state_;
}; // RendererSamplerUpdateParam

class RendererSampler
{
protected:
	RendererSampler() = default;

	virtual ~RendererSampler() = default;


public:
	static constexpr int anisotropy_min = 1;


	virtual void update(
		const RendererSamplerUpdateParam& param) = 0;
}; // RendererSampler

using RendererSamplerPtr = RendererSampler*;

//
// RendererTexture2d
// ==========================================================================


struct RendererCommandViewport
{
	int x_;
	int y_;
	int width_;
	int height_;

	float min_depth_;
	float max_depth_;
}; // ViewportSet

struct RendererCommandScissor
{
	bool is_enabled_;
}; // ScissorEnable

struct RendererCommandScissorBox
{
	int x_;
	int y_;
	int width_;
	int height_;
}; // ScissorSetBox

struct RendererCommandCulling
{
	bool is_enabled_;
}; // CullingEnabled

struct RendererCommandBlending
{
	bool is_enabled_;
}; // EnableBlending

struct RendererCommandBlendingFunction
{
	RendererBlendingFactor src_factor_;
	RendererBlendingFactor dst_factor_;
}; // RendererCommandBlendingFunction

struct RendererCommandDepthTest
{
	bool is_enabled_;
}; // DepthSetTest

struct RendererCommandDepthWrite
{
	bool is_enabled_;
}; // DepthSetWrite

struct RendererCommandFog
{
	bool is_enabled_;
}; // FogEnable

struct RendererCommandFogColor
{
	glm::vec4 color_;
}; // FogSetColor

struct RendererCommandFogDistances
{
	float start_;
	float end_;
}; // FogSetDistances

struct RendererCommandMatrixModel
{
	glm::mat4 model_;
}; // MatrixSetModel

struct RendererCommandMatrixView
{
	glm::mat4 view_;
}; // MatrixSetView

struct RendererCommandMatrixModelView
{
	glm::mat4 model_;
	glm::mat4 view_;
}; // MatrixSetModelView

struct RendererCommandMatrixProjection
{
	glm::mat4 projection_;
}; // MatrixSetProjection

struct RendererCommandTexture
{
	RendererTexture2dPtr texture_2d_;
}; // TextureSet

struct RendererCommandSampler
{
	RendererSamplerPtr sampler_;
}; // SamplerSet

struct RendererCommandVertexInput
{
	RendererVertexInputPtr vertex_input_;
}; // VertexInputSet

struct RendererCommandDrawQuads
{
	int count_;
	int index_offset_;
}; // DrawQuads


// ==========================================================================
// RendererCommandBuffer
//

class RendererCommandBuffer
{
protected:
	RendererCommandBuffer() = default;

	virtual ~RendererCommandBuffer() = default;


public:
	virtual int get_command_count() const = 0;


	virtual bool is_enabled() const = 0;

	virtual void enable(
		const bool is_enabled) = 0;


	virtual void write_begin() = 0;

	virtual void write_end() = 0;

	virtual RendererCommandViewport* write_viewport() = 0;

	virtual RendererCommandScissor* write_scissor() = 0;
	virtual RendererCommandScissorBox* write_scissor_box() = 0;

	virtual RendererCommandCulling* write_culling() = 0;

	virtual RendererCommandDepthTest* write_depth_test() = 0;
	virtual RendererCommandDepthWrite* write_depth_write() = 0;

	virtual RendererCommandBlending* write_blending() = 0;
	virtual RendererCommandBlendingFunction* write_blending_function() = 0;

	virtual RendererCommandFog* write_fog() = 0;
	virtual RendererCommandFogColor* write_fog_color() = 0;
	virtual RendererCommandFogDistances* write_fog_distances() = 0;

	virtual RendererCommandMatrixModel* write_matrix_model() = 0;
	virtual RendererCommandMatrixView* write_matrix_view() = 0;
	virtual RendererCommandMatrixModelView* write_matrix_model_view() = 0;
	virtual RendererCommandMatrixProjection* write_matrix_projection() = 0;

	virtual RendererCommandTexture* write_texture() = 0;
	virtual RendererCommandSampler* write_sampler() = 0;

	virtual RendererCommandVertexInput* write_vertex_input() = 0;

	virtual RendererCommandDrawQuads* write_draw_quads() = 0;


	virtual void read_begin() = 0;

	virtual void read_end() = 0;

	virtual RendererCommandId read_command_id() = 0;

	virtual const RendererCommandViewport* read_viewport() = 0;

	virtual const RendererCommandScissor* read_scissor() = 0;
	virtual const RendererCommandScissorBox* read_scissor_box() = 0;

	virtual const RendererCommandCulling* read_culling() = 0;

	virtual const RendererCommandDepthTest* read_depth_test() = 0;
	virtual const RendererCommandDepthWrite* read_depth_write() = 0;

	virtual const RendererCommandBlending* read_blending() = 0;
	virtual const RendererCommandBlendingFunction* read_blending_function() = 0;

	virtual const RendererCommandFog* read_fog() = 0;
	virtual const RendererCommandFogColor* read_fog_color() = 0;
	virtual const RendererCommandFogDistances* read_fog_distances() = 0;

	virtual const RendererCommandMatrixModel* read_matrix_model() = 0;
	virtual const RendererCommandMatrixView* read_matrix_view() = 0;
	virtual const RendererCommandMatrixModelView* read_matrix_model_view() = 0;
	virtual const RendererCommandMatrixProjection* read_matrix_projection() = 0;

	virtual const RendererCommandTexture* read_texture() = 0;
	virtual const RendererCommandSampler* read_sampler() = 0;

	virtual const RendererCommandVertexInput* read_vertex_input() = 0;

	virtual const RendererCommandDrawQuads* read_draw_quads() = 0;
}; // RendererCommandBuffer

using RendererCommandBufferPtr = RendererCommandBuffer*;

//
// RendererCommandBuffer
// ==========================================================================


// ==========================================================================
// RendererCommandManager
//

struct RendererCommandManagerBufferAddParam
{
	int initial_size_;
	int resize_delta_size_;
}; // RendererCommandManagerBufferAddParam

class RendererCommandManager
{
protected:
	RendererCommandManager() = default;


public:
	virtual ~RendererCommandManager() = default;


	virtual int buffer_get_count() const = 0;

	virtual RendererCommandBufferPtr buffer_add(
		const RendererCommandManagerBufferAddParam& param) = 0;

	virtual void buffer_remove(
		RendererCommandBufferPtr set) = 0;

	virtual RendererCommandBufferPtr buffer_get(
		const int index) = 0;
}; // RendererCommandManager

using RendererCommandManagerPtr = RendererCommandManager*;
using RendererCommandManagerUPtr = std::unique_ptr<RendererCommandManager>;

//
// RendererCommandManager
// ==========================================================================


// ==========================================================================
// RendererCommandManagerFactory
//

struct RendererCommandManagerFactory
{
	static RendererCommandManagerUPtr create();
}; // RendererCommandManagerFactory

//
// RendererCommandManagerFactory
// ==========================================================================


// ==========================================================================
// RendererDeviceFeatures
//

struct RendererDeviceFeatures
{
	bool is_losable_;

	int max_texture_dimension_;

	int max_viewport_width_;
	int max_viewport_height_;

	bool anisotropy_is_available_;
	int anisotropy_min_value_;
	int anisotropy_max_value_;

	bool npot_is_available_;

	bool mipmap_is_available_;

	bool framebuffer_is_available_;

	int msaa_min_value_;
	int msaa_max_value_;
}; // RendererDeviceFeatures

//
// RendererDeviceFeatures
// ==========================================================================

// ==========================================================================
// RendererProbe
//

struct RendererProbe
{
	RendererPath path_;
	RendererDeviceFeatures device_features_;
}; // RendererProbe

//
// RendererProbe
// ==========================================================================


class Renderer
{
protected:
	Renderer() = default;

	virtual ~Renderer() = default;


public:
	static constexpr int min_vfov_deg = 10;
	static constexpr int max_vfov_deg = 170;


	virtual const std::string& get_error_message() const = 0;


	virtual RendererKind get_kind() const = 0;

	virtual const std::string& get_name() const = 0;

	virtual const std::string& get_description() const = 0;


	virtual bool probe(
		const RendererPath renderer_path) = 0;

	virtual const RendererProbe& probe_get() const = 0;


	virtual bool is_initialized() const = 0;

	virtual bool initialize(
		const RendererInitializeParam& param) = 0;

	virtual void uninitialize() = 0;


	virtual RendererPath get_path() const = 0;


	virtual const RendererDeviceFeatures& device_get_features() const = 0;

	virtual bool device_is_lost() const = 0;

	virtual bool device_is_ready_to_reset() const = 0;

	virtual void device_reset() = 0;


	virtual void window_show(
		const bool is_visible) = 0;


	virtual void color_buffer_set_clear_color(
		const RendererColor32& color) = 0;

	virtual void clear_buffers() = 0;

	virtual void present() = 0;


	virtual RendererIndexBufferPtr index_buffer_create(
		const RendererIndexBufferCreateParam& param) = 0;

	virtual void index_buffer_destroy(
		RendererIndexBufferPtr index_buffer) = 0;


	virtual RendererVertexBufferPtr vertex_buffer_create(
		const RendererVertexBufferCreateParam& param) = 0;

	virtual void vertex_buffer_destroy(
		RendererVertexBufferPtr vertex_buffer) = 0;


	virtual RendererTexture2dPtr texture_2d_create(
		const RendererTexture2dCreateParam& param) = 0;

	virtual void texture_2d_destroy(
		RendererTexture2dPtr texture_2d) = 0;


	virtual RendererSamplerPtr sampler_create(
		const RendererSamplerCreateParam& param) = 0;

	virtual void sampler_destroy(
		RendererSamplerPtr sampler) = 0;


	virtual RendererVertexInputPtr vertex_input_create(
		const RendererVertexInputCreateParam& param) = 0;

	virtual void vertex_input_destroy(
		RendererVertexInputPtr vertex_input) = 0;


	virtual void execute_commands(
		const RendererCommandManagerPtr command_manager) = 0;
}; // Renderer

using RendererPtr = Renderer*;


} // bstone


#endif // !BSTONE_RENDERER_INCLUDED
