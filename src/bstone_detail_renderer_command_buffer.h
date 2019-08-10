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
// Renderer's command buffer (implementaion).
//


#ifndef BSTONE_DETAIL_RENDERER_COMMAND_BUFFER_INCLUDED
#define BSTONE_DETAIL_RENDERER_COMMAND_BUFFER_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// RendererCommandBuffer
//

class RendererCommandBuffer final :
	public bstone::RendererCommandBuffer
{
public:
	RendererCommandBuffer();

	RendererCommandBuffer(
		RendererCommandBuffer&& rhs);

	~RendererCommandBuffer() override;


	int get_command_count() const override;


	bool is_enabled() const override;

	void enable(
		const bool is_enabled) override;


	void write_begin() override;

	void write_end() override;

	RendererCommandViewport* write_viewport() override;

	RendererCommandScissor* write_scissor() override;
	RendererCommandScissorBox* write_scissor_box() override;

	RendererCommandCulling* write_culling() override;

	RendererCommandDepthTest* write_depth_test() override;
	RendererCommandDepthWrite* write_depth_write() override;

	RendererCommandBlending* write_blending() override;
	RendererCommandBlendingFunction* write_blending_function() override;

	RendererCommandTexture* write_texture() override;
	RendererCommandSampler* write_sampler() override;

	RendererCommandVertexInput* write_vertex_input() override;

	RendererCommandShaderStage* write_shader_stage() override;

	RendererCommandShaderVariableInt32* write_shader_variable_int32() override;
	RendererCommandShaderVariableFloat32* write_shader_variable_float32() override;
	RendererCommandShaderVariableVec2* write_shader_variable_vec2() override;
	RendererCommandShaderVariableVec4* write_shader_variable_vec4() override;
	RendererCommandShaderVariableMat4* write_shader_variable_mat4() override;
	RendererCommandShaderVariableSampler2d* write_shader_variable_sampler_2d() override;

	RendererCommandDrawQuads* write_draw_quads() override;


	void read_begin() override;

	void read_end() override;

	RendererCommandId read_command_id() override;

	const RendererCommandViewport* read_viewport() override;

	const RendererCommandScissor* read_scissor() override;
	const RendererCommandScissorBox* read_scissor_box() override;

	const RendererCommandCulling* read_culling() override;

	const RendererCommandDepthTest* read_depth_test() override;
	const RendererCommandDepthWrite* read_depth_write() override;

	const RendererCommandBlending* read_blending() override;
	const RendererCommandBlendingFunction* read_blending_function() override;

	const RendererCommandTexture* read_texture() override;
	const RendererCommandSampler* read_sampler() override;

	const RendererCommandVertexInput* read_vertex_input() override;

	const RendererCommandShaderStage* read_shader_stage() override;

	const RendererCommandShaderVariableInt32* read_shader_variable_int32() override;
	const RendererCommandShaderVariableFloat32* read_shader_variable_float32() override;
	const RendererCommandShaderVariableVec2* read_shader_variable_vec2() override;
	const RendererCommandShaderVariableVec4* read_shader_variable_vec4() override;
	const RendererCommandShaderVariableMat4* read_shader_variable_mat4() override;
	const RendererCommandShaderVariableSampler2d* read_shader_variable_sampler_2d() override;

	const RendererCommandDrawQuads* read_draw_quads() override;


	void initialize(
		const RendererCommandManagerBufferAddParam& param);


private:
	static constexpr int get_min_initial_size()
	{
		return 4096;
	}

	static constexpr int get_min_resize_delta_size()
	{
		return 4096;
	}

	static constexpr int get_command_id_size()
	{
		return static_cast<int>(sizeof(RendererCommandId));
	}


	using Data = std::vector<std::uint8_t>;

	bool is_enabled_;
	bool is_reading_;
	bool is_writing_;

	int size_;
	int write_offset_;
	int read_offset_;
	int resize_delta_size_;
	int command_count_;

	Data data_;


	void resize_if_necessary(
		const int dst_delta_size);

	template<typename T>
	T* write(
		const RendererCommandId command_id)
	{
		if (is_reading_ || !is_writing_)
		{
			assert(!"Invalid state.");

			return nullptr;
		}

		if (command_id == RendererCommandId::none)
		{
			assert(!"Invalid command id.");

			return nullptr;
		}

		const auto command_size = static_cast<int>(sizeof(T));

		const auto block_size = get_command_id_size() + command_size;

		resize_if_necessary(block_size);

		auto block = reinterpret_cast<RendererCommandId*>(&data_[write_offset_]);
		*block = command_id;

		write_offset_ += block_size;
		++command_count_;

		return reinterpret_cast<T*>(block + 1);
	}

	template<typename T>
	const T* read()
	{
		if (!is_reading_ || is_writing_)
		{
			assert(!"Invalid state.");

			return nullptr;
		}

		const auto command_size = static_cast<int>(sizeof(T));

		if ((size_ - read_offset_) < command_size)
		{
			return nullptr;
		}

		auto command = reinterpret_cast<const T*>(&data_[read_offset_]);

		read_offset_ += command_size;

		return command;
	}
}; // RendererCommandBuffer

using RendererCommandBufferPtr = RendererCommandBuffer*;
using RendererCommandBufferUPtr = std::unique_ptr<RendererCommandBuffer>;

//
// RendererCommandBuffer
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_RENDERER_COMMAND_BUFFER_INCLUDED
