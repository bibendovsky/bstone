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
#include <string>
#include <vector>
#include "bstone_renderer_color_32.h"
#include "bstone_sprite.h"


namespace bstone
{


enum class RendererKind
{
	none,
	opengl,
}; // RendererKind

enum class RendererPath
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

enum class RendererCullingFace
{
	counter_clockwise,
	clockwise,
}; // RendererCullingFace

enum class RendererCullingMode
{
	back,
	front,
	both,
}; // RendererCullingMode


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
}; // RendererInitializeParam


using RendererPalette = std::array<RendererColor32, 256>;


// ==========================================================================
// RendererIndexBuffer
//

enum class RendererIbElementTypeId
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

enum class RendererVertexAttributeLocation
{
	none,
	position,
	color,
	texture_coordinates,
}; // RendererVertexAttributeLocation

enum class RendererVertexAttributeFormat
{
	none,
	r8g8b8a8_uint,
	r32g32_float,
	r32g32b32_float,
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

enum class RendererFilterKind
{
	nearest,
	linear,
}; // RendererFilterKind

enum class RendererMipmapMode
{
	none,
	nearest,
	linear,
}; // RendererMipmapMode

enum class RendererAddressMode
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
}; // RendererSamplerState

struct RendererTexture2dCreateParam
{
	bool has_rgba_alpha_;
	bool is_generate_mipmaps_;

	int width_;
	int height_;

	bool indexed_is_column_major_;
	const std::uint8_t* indexed_pixels_;
	const RendererPalette* indexed_palette_;
	const bool* indexed_alphas_;

	SpriteCPtr indexed_sprite_;

	const RendererColor32* rgba_pixels_;
}; // RendererTexture2dCreateParam

struct RendererTexture2dUpdateParam
{
	const std::uint8_t* indexed_pixels_;
	const RendererPalette* indexed_palette_;
	const bool* indexed_alphas_;

	SpriteCPtr indexed_sprite_;

	const RendererColor32* rgba_pixels_;
}; // RendererTexture2dUpdateParam

class RendererTexture2d
{
protected:
	RendererTexture2d() = default;

	virtual ~RendererTexture2d() = default;


public:
	virtual void update(
		const RendererTexture2dUpdateParam& param) = 0;
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
	virtual void update(
		const RendererSamplerUpdateParam& param) = 0;
}; // RendererSampler

using RendererSamplerPtr = RendererSampler*;

//
// RendererTexture2d
// ==========================================================================


struct RendererDepthStateFlags
{
	using Value = unsigned char;


	Value is_enabled_ : 1;
	Value is_writable_ : 1;
}; // RendererDepthStateFlags

struct RendererCommandViewportSet
{
	int x_;
	int y_;
	int width_;
	int height_;

	float min_depth_;
	float max_depth_;
}; // ViewportSet

struct RendererCommandScissorEnable
{
	bool is_enabled_;
}; // ScissorEnable

struct RendererCommandScissorSetBox
{
	int x_;
	int y_;
	int width_;
	int height_;
}; // ScissorSetBox

struct RendererCommandCullingEnabled
{
	bool is_enabled_;
}; // CullingEnabled

struct RendererCommandBlendingEnable
{
	bool is_enabled_;
}; // EnableBlending

struct RendererCommandDepthSetTest
{
	bool is_enabled_;
}; // DepthSetTest

struct RendererCommandDepthSetWrite
{
	bool is_enabled_;
}; // DepthSetWrite

struct RendererCommandFogEnable
{
	bool is_enabled_;
}; // FogEnable

struct RendererCommandFogSetColor
{
	glm::vec4 color_;
}; // FogSetColor

struct RendererCommandFogSetDistances
{
	float start_;
	float end_;
}; // FogSetDistances

struct RendererCommandMatrixSetModel
{
	glm::mat4 model_;
}; // MatrixSetModel

struct RendererCommandMatrixSetView
{
	glm::mat4 view_;
}; // MatrixSetView

struct RendererCommandMatrixSetModelView
{
	glm::mat4 model_;
	glm::mat4 view_;
}; // MatrixSetModelView

struct RendererCommandMatrixSetProjection
{
	glm::mat4 projection_;
}; // MatrixSetProjection

struct RendererCommandTextureSet
{
	RendererTexture2dPtr texture_2d_;
}; // TextureSet

struct RendererCommandSamplerSet
{
	RendererSamplerPtr sampler_;
}; // SamplerSet

struct RendererCommandVertexInputSet
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

	virtual RendererCommandViewportSet* write_viewport_set() = 0;

	virtual RendererCommandScissorEnable* write_scissor_enable() = 0;
	virtual RendererCommandScissorSetBox* write_scissor_set_box() = 0;

	virtual RendererCommandCullingEnabled* write_culling_enable() = 0;

	virtual RendererCommandDepthSetTest* write_depth_set_test() = 0;
	virtual RendererCommandDepthSetWrite* write_depth_set_write() = 0;

	virtual RendererCommandBlendingEnable* write_blending_enable() = 0;

	virtual RendererCommandFogEnable* write_fog_enable() = 0;
	virtual RendererCommandFogSetColor* write_fog_set_color() = 0;
	virtual RendererCommandFogSetDistances* write_fog_set_distances() = 0;

	virtual RendererCommandMatrixSetModel* write_matrix_set_model() = 0;
	virtual RendererCommandMatrixSetView* write_matrix_set_view() = 0;
	virtual RendererCommandMatrixSetModelView* write_matrix_set_model_view() = 0;
	virtual RendererCommandMatrixSetProjection* write_matrix_set_projection() = 0;

	virtual RendererCommandTextureSet* write_texture_set() = 0;
	virtual RendererCommandSamplerSet* write_sampler_set() = 0;

	virtual RendererCommandVertexInputSet* write_vertex_input_set() = 0;

	virtual RendererCommandDrawQuads* write_draw_quads() = 0;


	virtual void read_begin() = 0;

	virtual void read_end() = 0;

	virtual RendererCommandId read_command_id() = 0;

	virtual const RendererCommandViewportSet* read_viewport_set() = 0;

	virtual const RendererCommandScissorEnable* read_scissor_enable() = 0;
	virtual const RendererCommandScissorSetBox* read_scissor_set_box() = 0;

	virtual const RendererCommandCullingEnabled* read_culling_enable() = 0;

	virtual const RendererCommandDepthSetTest* read_depth_set_test() = 0;
	virtual const RendererCommandDepthSetWrite* read_depth_set_write() = 0;

	virtual const RendererCommandBlendingEnable* read_blending_enable() = 0;

	virtual const RendererCommandFogEnable* read_fog_enable() = 0;
	virtual const RendererCommandFogSetColor* read_fog_set_color() = 0;
	virtual const RendererCommandFogSetDistances* read_fog_set_distances() = 0;

	virtual const RendererCommandMatrixSetModel* read_matrix_set_model() = 0;
	virtual const RendererCommandMatrixSetView* read_matrix_set_view() = 0;
	virtual const RendererCommandMatrixSetModelView* read_matrix_set_model_view() = 0;
	virtual const RendererCommandMatrixSetProjection* read_matrix_set_projection() = 0;

	virtual const RendererCommandTextureSet* read_texture_set() = 0;
	virtual const RendererCommandSamplerSet* read_sampler_set() = 0;

	virtual const RendererCommandVertexInputSet* read_vertex_input_set() = 0;

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

	virtual RendererPath get_probe_path() const = 0;


	virtual bool is_initialized() const = 0;

	virtual bool initialize(
		const RendererInitializeParam& param) = 0;

	virtual void uninitialize() = 0;


	virtual RendererPath get_path() const = 0;


	virtual void window_show(
		const bool is_visible) = 0;


	virtual void color_buffer_set_clear_color(
		const RendererColor32& color) = 0;

	virtual void clear_buffers() = 0;

	virtual void present() = 0;


	virtual void palette_update(
		const RendererPalette& palette) = 0;


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
