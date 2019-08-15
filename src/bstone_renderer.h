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
#include "bstone_rgb_color_model.h"
#include "bstone_sprite.h"


namespace bstone
{


enum class RendererKind :
	unsigned char
{
	none,

	auto_detect,
	software,

	ogl_2_x,
}; // RendererKind

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

	texture_set,
	sampler_set,

	vertex_input_set,

	shader_stage,

	shader_variable_int32,
	shader_variable_float32,
	shader_variable_vec2,
	shader_variable_vec4,
	shader_variable_mat4,
	shader_variable_sampler2d,

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

enum class RendererBufferKind :
	unsigned char
{
	none,
	index,
	vertex,
}; // RendererBufferKind

enum class RendererBufferUsageKind :
	unsigned char
{
	none,

	// Target - drawing; updates - continuous.
	stream_draw,

	// Target - drawing; updates - seldom.
	static_draw,

	// Target - drawing; updates - often.
	dynamic_draw,
}; // RendererBufferUsageKind


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
	RendererKind renderer_kind_;
	RendererInitializeWindowParam window_;

	int downscale_width_;
	int downscale_height_;

	RendererAaKind aa_kind_;
	int aa_value_;

	bool is_vsync_;
}; // RendererInitializeParam


// ==========================================================================
// RendererBuffer
//

struct RendererBufferUpdateParam
{
	int offset_;
	int size_;
	const void* data_;
}; // RendererBufferUpdateParam

class RendererBuffer
{
protected:
	RendererBuffer();

	virtual ~RendererBuffer();


public:
	virtual RendererBufferKind get_kind() const noexcept = 0;

	virtual RendererBufferUsageKind get_usage_kind() const noexcept = 0;

	virtual int get_size() const noexcept = 0;

	virtual void update(
		const RendererBufferUpdateParam& param) = 0;

	virtual void bind(
		const bool is_bind) = 0;
}; // RendererBuffer

using RendererBufferPtr = RendererBuffer*;

//
// RendererBuffer
// ==========================================================================


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
	RendererBufferUsageKind usage_kind_;
	int byte_depth_;
	int size_;
}; // RendererIndexBufferCreateParam

class RendererIndexBuffer :
	public RendererBuffer
{
protected:
	RendererIndexBuffer();

	~RendererIndexBuffer() override;


public:
	virtual int get_byte_depth() const noexcept = 0;
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
	RendererBufferUsageKind usage_kind_;
	int size_;
}; // RendererVertexBufferCreateParam

class RendererVertexBuffer :
	public RendererBuffer
{
protected:
	RendererVertexBuffer();

	~RendererVertexBuffer() override;
}; // RendererVertexBuffer

using RendererVertexBufferPtr = RendererVertexBuffer*;

//
// RendererVertexBuffer
// ==========================================================================


// ==========================================================================
// RendererVertexInput
//

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
	bool is_default_;
	int location_;
	RendererVertexAttributeFormat format_;
	RendererVertexBufferPtr vertex_buffer_;
	int offset_;
	int stride_;
	glm::vec4 default_value_;
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


	virtual RendererIndexBufferPtr get_index_buffer() const noexcept = 0;
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

enum class RendererTextureAxis :
	unsigned char
{
	u,
	v,
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

	R8g8b8a8CPtr rgba_pixels_;
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

	virtual const RendererSamplerState& get_state() const noexcept = 0;
}; // RendererSampler

using RendererSamplerPtr = RendererSampler*;

//
// RendererTexture2d
// ==========================================================================


// ==========================================================================
// Shader
//

class RendererShaderVariable
{
protected:
	RendererShaderVariable();

	virtual ~RendererShaderVariable();

public:
	enum class Kind
	{
		none,
		attribute,
		sampler,
		uniform,
	}; // Kind

	enum class TypeId
	{
		none,
		int32,
		float32,
		vec2,
		vec3,
		vec4,
		mat4,
		sampler2d,
	}; // TypeId


	virtual Kind get_kind() const = 0;

	virtual TypeId get_type_id() const = 0;

	virtual int get_index() const = 0;

	virtual const std::string& get_name() const = 0;

	virtual int get_input_index() const = 0;
}; // RendererShaderVariable

using RendererShaderVariablePtr = RendererShaderVariable*;
using RendererShaderVariableCPtr = const RendererShaderVariable*;


class RendererShaderVariableInt32 :
	public virtual RendererShaderVariable
{
protected:
	RendererShaderVariableInt32();

	virtual ~RendererShaderVariableInt32();

public:
	virtual void set_value(
		const std::int32_t value) = 0;
}; // RendererShaderVariableInt32

using RendererShaderVariableInt32Ptr = RendererShaderVariableInt32*;
using RendererShaderVariableInt32CPtr = const RendererShaderVariableInt32*;


class RendererShaderVariableFloat32 :
	public virtual RendererShaderVariable
{
protected:
	RendererShaderVariableFloat32();

	virtual ~RendererShaderVariableFloat32();

public:
	virtual void set_value(
		const float value) = 0;
}; // RendererShaderVariableFloat32

using RendererShaderVariableFloat32Ptr = RendererShaderVariableFloat32*;
using RendererShaderVariableFloat32CPtr = const RendererShaderVariableFloat32*;


class RendererShaderVariableVec2 :
	public virtual RendererShaderVariable
{
protected:
	RendererShaderVariableVec2();

	virtual ~RendererShaderVariableVec2();

public:
	virtual void set_value(
		const glm::vec2& value) = 0;
}; // RendererShaderVariableVec2

using RendererShaderVariableVec2Ptr = RendererShaderVariableVec2*;
using RendererShaderVariableVec2CPtr = const RendererShaderVariableVec2*;


class RendererShaderVariableVec4 :
	public virtual RendererShaderVariable
{
protected:
	RendererShaderVariableVec4();

	virtual ~RendererShaderVariableVec4();

public:
	virtual void set_value(
		const glm::vec4& value) = 0;
}; // RendererShaderVariableVec4

using RendererShaderVariableVec4Ptr = RendererShaderVariableVec4*;
using RendererShaderVariableVec4CPtr = const RendererShaderVariableVec4*;


class RendererShaderVariableMat4 :
	public virtual RendererShaderVariable
{
protected:
	RendererShaderVariableMat4();

	virtual ~RendererShaderVariableMat4();

public:
	virtual void set_value(
		const glm::mat4& value) = 0;
}; // RendererShaderVariableMat4

using RendererShaderVariableMat4Ptr = RendererShaderVariableMat4*;
using RendererShaderVariableMat4CPtr = const RendererShaderVariableMat4*;


using RendererShaderVariableSampler2d = RendererShaderVariableInt32;
using RendererShaderVariableSampler2dPtr = RendererShaderVariableSampler2d*;
using RendererShaderVariableSampler2dCPtr = const RendererShaderVariableSampler2d*;


class RendererShader
{
protected:
	RendererShader();

	virtual ~RendererShader();


public:
	enum class Kind
	{
		none,
		fragment,
		vertex,
	}; // Kind

	struct Source
	{
		const void* data_;
		int size_;
	}; // Source

	struct CreateParam
	{
		Kind kind_;
		Source source_;
	}; // CreateParam


	virtual Kind get_kind() const = 0;
}; // RendererShader

using RendererShaderPtr = RendererShader*;


class RendererShaderStage
{
protected:
	RendererShaderStage();

	virtual ~RendererShaderStage();


public:
	struct InputBinding
	{
		int index_;
		std::string name_;
	}; // InputBinding

	using InputBindings = std::vector<InputBinding>;


	struct CreateParam
	{
		RendererShaderPtr fragment_shader_;
		RendererShaderPtr vertex_shader_;
		InputBindings input_bindings_;
	}; // CreateParam


	virtual void set_current() = 0;

	virtual RendererShaderVariablePtr find_variable(
		const std::string& name) = 0;

	virtual RendererShaderVariableInt32Ptr find_variable_int32(
		const std::string& name) = 0;

	virtual RendererShaderVariableFloat32Ptr find_variable_float32(
		const std::string& name) = 0;

	virtual RendererShaderVariableVec2Ptr find_variable_vec2(
		const std::string& name) = 0;

	virtual RendererShaderVariableVec4Ptr find_variable_vec4(
		const std::string& name) = 0;

	virtual RendererShaderVariableMat4Ptr find_variable_mat4(
		const std::string& name) = 0;

	virtual RendererShaderVariableSampler2dPtr find_variable_sampler_2d(
		const std::string& name) = 0;
}; // RendererShaderStage

using RendererShaderStagePtr = RendererShaderStage*;

//
// Shader
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

struct RendererCommandShaderStage
{
	RendererShaderStagePtr shader_stage_;
}; // RendererCommandShaderStage

struct RendererCommandShaderVariableInt32
{
	RendererShaderVariableInt32Ptr variable_;
	std::int32_t value_;
};

struct RendererCommandShaderVariableFloat32
{
	RendererShaderVariableFloat32Ptr variable_;
	float value_;
};

struct RendererCommandShaderVariableVec2
{
	RendererShaderVariableVec2Ptr variable_;
	glm::vec2 value_;
};

struct RendererCommandShaderVariableVec4
{
	RendererShaderVariableVec4Ptr variable_;
	glm::vec4 value_;
};

struct RendererCommandShaderVariableMat4
{
	RendererShaderVariableMat4Ptr variable_;
	glm::mat4 value_;
};

struct RendererCommandShaderVariableSampler2d
{
	RendererShaderVariableSampler2dPtr variable_;
	std::int32_t value_;
};

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

	virtual RendererCommandTexture* write_texture() = 0;
	virtual RendererCommandSampler* write_sampler() = 0;

	virtual RendererCommandVertexInput* write_vertex_input() = 0;

	virtual RendererCommandShaderStage* write_shader_stage() = 0;

	virtual RendererCommandShaderVariableInt32* write_shader_variable_int32() = 0;
	virtual RendererCommandShaderVariableFloat32* write_shader_variable_float32() = 0;
	virtual RendererCommandShaderVariableVec2* write_shader_variable_vec2() = 0;
	virtual RendererCommandShaderVariableVec4* write_shader_variable_vec4() = 0;
	virtual RendererCommandShaderVariableMat4* write_shader_variable_mat4() = 0;
	virtual RendererCommandShaderVariableSampler2d* write_shader_variable_sampler_2d() = 0;

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

	virtual const RendererCommandTexture* read_texture() = 0;
	virtual const RendererCommandSampler* read_sampler() = 0;

	virtual const RendererCommandVertexInput* read_vertex_input() = 0;

	virtual const RendererCommandShaderStage* read_shader_stage() = 0;

	virtual const RendererCommandShaderVariableInt32* read_shader_variable_int32() = 0;
	virtual const RendererCommandShaderVariableFloat32* read_shader_variable_float32() = 0;
	virtual const RendererCommandShaderVariableVec2* read_shader_variable_vec2() = 0;
	virtual const RendererCommandShaderVariableVec4* read_shader_variable_vec4() = 0;
	virtual const RendererCommandShaderVariableMat4* read_shader_variable_mat4() = 0;
	virtual const RendererCommandShaderVariableSampler2d* read_shader_variable_sampler_2d() = 0;

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

	bool vsync_is_available_;
	bool vsync_is_requires_restart_;

	int max_texture_dimension_;

	int max_viewport_width_;
	int max_viewport_height_;

	bool anisotropy_is_available_;
	int anisotropy_min_value_;
	int anisotropy_max_value_;

	bool npot_is_available_;

	bool mipmap_is_available_;

	bool framebuffer_is_available_;

	bool sampler_is_available_;

	int msaa_min_value_;
	int msaa_max_value_;

	int vertex_input_max_locations_;
}; // RendererDeviceFeatures

//
// RendererDeviceFeatures
// ==========================================================================


// ==========================================================================
// RendererDeviceInfo
//

struct RendererDeviceInfo
{
	std::string name_;
	std::string vendor_;
	std::string version_;
}; // RendererDeviceInfo

//
// RendererDeviceInfo
// ==========================================================================


// ==========================================================================
// RendererProbe
//

struct RendererProbe
{
	RendererKind kind_;
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
	virtual const std::string& get_error_message() const = 0;


	virtual RendererKind get_kind() const = 0;

	virtual const std::string& get_name() const = 0;

	virtual const std::string& get_description() const = 0;


	virtual bool probe() = 0;

	virtual const RendererProbe& probe_get() const = 0;


	virtual bool is_initialized() const = 0;

	virtual bool initialize(
		const RendererInitializeParam& param) = 0;

	virtual void uninitialize() = 0;


	virtual const RendererDeviceFeatures& device_get_features() const = 0;

	virtual const RendererDeviceInfo& device_get_info() const = 0;

	virtual bool device_is_lost() const = 0;

	virtual bool device_is_ready_to_reset() const = 0;

	virtual void device_reset() = 0;


	virtual void window_show(
		const bool is_visible) = 0;


	virtual const glm::mat4& csc_get_texture() const = 0;

	virtual const glm::mat4& csc_get_projection() const = 0;


	virtual bool vsync_get() const = 0;

	virtual bool vsync_set(
		const bool is_enabled) = 0;


	virtual bool downscale_set(
		const int width,
		const int height,
		const RendererFilterKind blit_filter) = 0;


	virtual bool aa_set(
		const RendererAaKind aa_kind,
		const int aa_value) = 0;


	virtual void color_buffer_set_clear_color(
		const R8g8b8a8& color) = 0;

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


	virtual RendererShaderPtr shader_create(
		const RendererShader::CreateParam& param) = 0;

	virtual void shader_destroy(
		const RendererShaderPtr shader) = 0;


	virtual RendererShaderStagePtr shader_stage_create(
		const RendererShaderStage::CreateParam& param) = 0;

	virtual void shader_stage_destroy(
		const RendererShaderStagePtr shader) = 0;


	virtual void execute_commands(
		const RendererCommandManagerPtr command_manager) = 0;
}; // Renderer

using RendererPtr = Renderer*;


} // bstone


#endif // !BSTONE_RENDERER_INCLUDED
