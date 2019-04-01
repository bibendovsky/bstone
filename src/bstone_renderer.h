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
// Renderer interface.
//


#ifndef BSTONE_RENDERER_INCLUDED
#define BSTONE_RENDERER_INCLUDED


#include <array>
#include <string>
#include <vector>
#include "glm/glm.hpp"
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

enum class RendererCommandId
{
	none,

	viewport_set,

	scissor_enable,
	scissor_set_box,

	culling_set,

	depth_set_test,
	depth_set_write,

	blending_set,

	matrix_set_model,
	matrix_set_view,
	matrix_set_model_view,
	matrix_set_projection,

	set_2d,
	set_3d,

	draw_quads,
}; // RendererCommandId


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


class RendererColor32
{
public:
	std::uint8_t r_;
	std::uint8_t g_;
	std::uint8_t b_;
	std::uint8_t a_;


	constexpr RendererColor32()
		:
		r_{},
		g_{},
		b_{},
		a_{}
	{
	}

	constexpr RendererColor32(
		const std::uint8_t r,
		const std::uint8_t g,
		const std::uint8_t b,
		const std::uint8_t a)
		:
		r_{r},
		g_{g},
		b_{b},
		a_{a}
	{
	}


	std::uint8_t* get_data()
	{
		return reinterpret_cast<std::uint8_t*>(this);
	}

	const std::uint8_t* get_data() const
	{
		return reinterpret_cast<const std::uint8_t*>(this);
	}


	constexpr std::uint32_t to_u32() const
	{
		return (r_ << 24) | (g_ << 16) | (b_ << 8) | a_;
	}

	constexpr bool operator==(
		const RendererColor32& rhs) const
	{
		return r_ == rhs.r_ && g_ == rhs.g_ && b_ == rhs.b_ && a_ == rhs.a_;
	}
}; // RendererColor32

using RendererColor32Ptr = RendererColor32*;
using RendererColor32CPtr = const RendererColor32*;

using RendererPalette = std::array<RendererColor32, 256>;

class RendererVertex
{
public:
	glm::vec3 xyz_;
	RendererColor32 rgba_;
	glm::vec2 uv_;
}; // RendererVertex


// ==========================================================================
// RendererIndexBuffer
//

struct RendererIndexBufferCreateParam
{
	int index_count_;
}; // RendererIndexBufferCreateParam

struct RendererIndexBufferUpdateParam
{
	int offset_;
	int count_;
	const std::uint16_t* indices_;
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
	int vertex_count_;
}; // RendererVertexBufferCreateParam

struct RendererVertexBufferUpdateParam
{
	int offset_;
	int count_;
	const RendererVertex* vertices_;
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
// RendererTexture2d
//

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

//
// RendererTexture2d
// ==========================================================================


struct RendererDepthStateFlags
{
	using Value = unsigned char;


	Value is_enabled_ : 1;
	Value is_writable_ : 1;
}; // RendererDepthStateFlags

struct RendererCommand
{
	struct ViewportSet
	{
		int x_;
		int y_;
		int width_;
		int height_;

		float min_depth_;
		float max_depth_;
	}; // ViewportSet

	struct ScissorEnable
	{
		bool is_enabled_;
	}; // ScissorEnable

	struct ScissorSetBox
	{
		int x_;
		int y_;
		int width_;
		int height_;
	}; // ScissorSetBox

	struct CullingSet
	{
		bool is_enabled_;
	}; // CullingSet

	struct MatrixSetModel
	{
		glm::mat4 model_;
	}; // MatrixSetModel

	struct MatrixSetView
	{
		glm::mat4 view_;
	}; // MatrixSetView

	struct MatrixSetModelView
	{
		glm::mat4 model_;
		glm::mat4 view_;
	}; // MatrixSetModelView

	struct MatrixSetProjection
	{
		glm::mat4 projection_;
	}; // MatrixSetProjection

	struct Set2d
	{
	}; // Set2d

	struct Set3d
	{
	}; // Set3d

	struct BlendingSet
	{
		bool is_enabled_;
	}; // EnableBlending

	struct DepthSetTest
	{
		bool is_enabled_;
	}; // DepthSetTest

	struct DepthSetWrite
	{
		bool is_enabled_;
	}; // DepthSetWrite

	struct DrawQuads
	{
		int count_;
		int index_offset_;
		RendererTexture2dPtr texture_2d_;
		RendererIndexBufferPtr index_buffer_;
		RendererVertexBufferPtr vertex_buffer_;
	}; // DrawQuads


	RendererCommandId id_;

	union
	{
		ViewportSet viewport_set_;

		ScissorEnable scissor_enable_;
		ScissorSetBox scissor_set_box_;

		CullingSet culling_set_;

		DepthSetTest depth_set_test_;
		DepthSetWrite depth_set_write_;

		BlendingSet blending_set_;

		MatrixSetModel matrix_set_model_;
		MatrixSetView matrix_set_view_;
		MatrixSetModelView matrix_set_model_view_;
		MatrixSetProjection matrix_set_projection_;

		Set2d set_2d_;
		Set3d set_3d_;

		DrawQuads draw_quads_;
	}; // union
}; // RendererCommand

struct RendererCommandSet
{
	using Commands = std::vector<RendererCommand>;


	int count_;
	Commands commands_;
}; // RendererCommandSet

using RendererCommandSets = std::vector<RendererCommandSet>;


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


	virtual void set_2d_projection_matrix(
		const int width,
		const int height) = 0;

	virtual void set_3d_view_matrix(
		const int angle_deg,
		const glm::vec3& position) = 0;

	virtual void set_3d_projection_matrix(
		const int width,
		const int height,
		const int vfov_deg,
		const float near_distance,
		const float far_distance) = 0;


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


	virtual void execute_command_sets(
		const RendererCommandSets& command_sets) = 0;
}; // Renderer

using RendererPtr = Renderer*;


} // bstone


#endif // !BSTONE_RENDERER_INCLUDED
