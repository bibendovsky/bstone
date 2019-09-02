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
// OpenGL command executor (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_command_executor.h"

#include "bstone_exception.h"
#include "bstone_renderer.h"

#include "bstone_detail_ogl_context.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_sampler_manager.h"
#include "bstone_detail_ogl_texture_manager.h"
#include "bstone_detail_ogl_vertex_input.h"
#include "bstone_detail_ogl_vertex_input_manager.h"
#include "bstone_detail_renderer_command_manager.h"


namespace bstone
{
namespace detail
{


// =========================================================================
// OglCommandExecutor
//

OglCommandExecutor::OglCommandExecutor() = default;

OglCommandExecutor::~OglCommandExecutor() = default;

//
// OglCommandExecutor
// =========================================================================


// =========================================================================
// GenericOglCommandExecutor
//

class GenericOglCommandExecutor final :
	public OglCommandExecutor
{
public:
	GenericOglCommandExecutor(
		const OglContextPtr ogl_context);

	~GenericOglCommandExecutor() override;


	void execute(
		const bstone::RendererCommandManagerPtr command_manager) override;


private:
	const OglContextPtr ogl_context_;


	void initialize();


	void command_execute_culling(
		const RendererCommandCulling& command);

	void command_execute_depth_test(
		const RendererCommandDepthTest& command);

	void command_execute_depth_write(
		const RendererCommandDepthWrite& command);

	void command_execute_blending(
		const RendererCommandBlending& command);

	void command_execute_blending_func(
		const RendererCommandBlendingFunc& command);

	void command_execute_viewport(
		const RendererCommandViewport& command);

	void command_execute_scissor(
		const RendererCommandScissor& command);

	void command_execute_scissor_box(
		const RendererCommandScissorBox& command);

	void command_execute_texture(
		const RendererCommandTexture& command);

	void command_execute_sampler(
		const RendererCommandSampler& command);

	void command_execute_vertex_input(
		const RendererCommandVertexInput& command);

	void command_execute_shader_stage(
		const RendererCommandShaderStage& command);

	void command_execute_shader_var_int32(
		const RendererCommandShaderVarInt32& command);

	void command_execute_shader_var_float32(
		const RendererCommandShaderVarFloat32& command);

	void command_execute_shader_var_vec2(
		const RendererCommandShaderVarVec2& command);

	void command_execute_shader_var_vec4(
		const RendererCommandShaderVarVec4& command);

	void command_execute_shader_var_mat4(
		const RendererCommandShaderVarMat4& command);

	void command_execute_shader_var_sampler_2d(
		const RendererCommandShaderVarSampler2d& command);

	void command_execute_draw_quads(
		const RendererCommandDrawQuads& command);
}; // GenericOglCommandExecutor


using GenericOglCommandExecutorUPtr = std::unique_ptr<GenericOglCommandExecutor>;

//
// GenericOglCommandExecutor
// =========================================================================


// =========================================================================
// GenericOglCommandExecutor
//

GenericOglCommandExecutor::GenericOglCommandExecutor(
	const OglContextPtr ogl_context)
	:
	ogl_context_{ogl_context}
{
	initialize();
}

GenericOglCommandExecutor::~GenericOglCommandExecutor() = default;

void GenericOglCommandExecutor::execute(
	const bstone::RendererCommandManagerPtr command_manager)
{
	if (!command_manager)
	{
		throw Exception{"Null command manager."};
	}

	const auto buffer_count = command_manager->buffer_get_count();

	for (int i = 0; i < buffer_count; ++i)
	{
		auto command_buffer = command_manager->buffer_get(i);

		if (!command_buffer->is_enabled())
		{
			continue;
		}

		const auto command_count = command_buffer->get_command_count();

		command_buffer->read_begin();

		for (int j = 0; j < command_count; ++j)
		{
			const auto command_id = command_buffer->read_command_id();

			switch (command_id)
			{
			case RendererCommandId::culling:
				command_execute_culling(*command_buffer->read_culling());
				break;

			case RendererCommandId::depth_set_test:
				command_execute_depth_test(*command_buffer->read_depth_test());
				break;

			case RendererCommandId::depth_set_write:
				command_execute_depth_write(*command_buffer->read_depth_write());
				break;

			case RendererCommandId::viewport:
				command_execute_viewport(*command_buffer->read_viewport());
				break;

			case RendererCommandId::scissor:
				command_execute_scissor(*command_buffer->read_scissor());
				break;

			case RendererCommandId::scissor_set_box:
				command_execute_scissor_box(*command_buffer->read_scissor_box());
				break;

			case RendererCommandId::blending:
				command_execute_blending(*command_buffer->read_blending());
				break;

			case RendererCommandId::blending_func:
				command_execute_blending_func(*command_buffer->read_blending_func());
				break;

			case RendererCommandId::texture:
				command_execute_texture(*command_buffer->read_texture());
				break;

			case RendererCommandId::sampler:
				command_execute_sampler(*command_buffer->read_sampler());
				break;

			case RendererCommandId::vertex_input:
				command_execute_vertex_input(*command_buffer->read_vertex_input());
				break;

			case RendererCommandId::shader_stage:
				command_execute_shader_stage(*command_buffer->read_shader_stage());
				break;

			case RendererCommandId::shader_var_int32:
				command_execute_shader_var_int32(*command_buffer->read_shader_var_int32());
				break;

			case RendererCommandId::shader_var_float32:
				command_execute_shader_var_float32(*command_buffer->read_shader_var_float32());
				break;

			case RendererCommandId::shader_var_vec2:
				command_execute_shader_var_vec2(*command_buffer->read_shader_var_vec2());
				break;

			case RendererCommandId::shader_var_vec4:
				command_execute_shader_var_vec4(*command_buffer->read_shader_var_vec4());
				break;

			case RendererCommandId::shader_var_mat4:
				command_execute_shader_var_mat4(*command_buffer->read_shader_var_mat4());
				break;

			case RendererCommandId::shader_var_sampler2d:
				command_execute_shader_var_sampler_2d(*command_buffer->read_shader_var_sampler_2d());
				break;

			case RendererCommandId::draw_quads:
				command_execute_draw_quads(*command_buffer->read_draw_quads());
				break;

			default:
				throw Exception{"Unsupported command id."};
			}
		}

		command_buffer->read_end();
	}
}

void GenericOglCommandExecutor::initialize()
{
	if (!ogl_context_)
	{
		throw Exception{"Null OpenGL context."};
	}
}

void GenericOglCommandExecutor::command_execute_culling(
	const RendererCommandCulling& command)
{
	ogl_context_->culling_enable(command.is_enable_);
}

void GenericOglCommandExecutor::command_execute_depth_test(
	const RendererCommandDepthTest& command)
{
	ogl_context_->depth_test_enable(command.is_enable_);
}

void GenericOglCommandExecutor::command_execute_depth_write(
	const RendererCommandDepthWrite& command)
{
	ogl_context_->depth_write_enable(command.is_enable_);
}

void GenericOglCommandExecutor::command_execute_viewport(
	const RendererCommandViewport& command)
{
	ogl_context_->viewport_set(command.viewport_);
}

void GenericOglCommandExecutor::command_execute_blending(
	const RendererCommandBlending& command)
{
	ogl_context_->blending_enable(command.is_enable_);
}

void GenericOglCommandExecutor::command_execute_blending_func(
	const RendererCommandBlendingFunc& command)
{
	ogl_context_->blending_set_func(command.blending_func_);
}

void GenericOglCommandExecutor::command_execute_scissor(
	const RendererCommandScissor& command)
{
	ogl_context_->scissor_enable(command.is_enable_);
}

void GenericOglCommandExecutor::command_execute_scissor_box(
	const RendererCommandScissorBox& command)
{
	ogl_context_->scissor_set_box(command.scissor_box_);
}

void GenericOglCommandExecutor::command_execute_texture(
	const RendererCommandTexture& command)
{
	ogl_context_->texture_get_manager()->set(command.texture_2d_);
}

void GenericOglCommandExecutor::command_execute_sampler(
	const RendererCommandSampler& command)
{
	ogl_context_->sampler_get_manager()->set(command.sampler_);
}

void GenericOglCommandExecutor::command_execute_vertex_input(
	const RendererCommandVertexInput& command)
{
	auto vertex_input = static_cast<OglVertexInputPtr>(command.vertex_input_);

	ogl_context_->vertex_input_get_manager()->set(vertex_input);
}

void GenericOglCommandExecutor::command_execute_shader_stage(
	const RendererCommandShaderStage& command)
{
	if (!command.shader_stage_)
	{
		throw Exception{"Null shader stage."};
	}

	command.shader_stage_->set();
}

void GenericOglCommandExecutor::command_execute_shader_var_int32(
	const RendererCommandShaderVarInt32& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void GenericOglCommandExecutor::command_execute_shader_var_float32(
	const RendererCommandShaderVarFloat32& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void GenericOglCommandExecutor::command_execute_shader_var_vec2(
	const RendererCommandShaderVarVec2& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void GenericOglCommandExecutor::command_execute_shader_var_vec4(
	const RendererCommandShaderVarVec4& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void GenericOglCommandExecutor::command_execute_shader_var_mat4(
	const RendererCommandShaderVarMat4& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void GenericOglCommandExecutor::command_execute_shader_var_sampler_2d(
	const RendererCommandShaderVarSampler2d& command)
{
	if (!command.var_)
	{
		throw Exception{"Null variable."};
	}

	command.var_->set_value(command.value_);
}

void GenericOglCommandExecutor::command_execute_draw_quads(
	const RendererCommandDrawQuads& command)
{
	assert(command.count_ > 0);
	assert(command.index_offset_ >= 0);

	const auto triangles_per_quad = 2;
	const auto triangle_count = command.count_ * triangles_per_quad;

	const auto indices_per_triangle = 3;
	const auto indices_per_quad = triangles_per_quad * indices_per_triangle;
	const auto index_count = indices_per_quad * command.count_;

	auto index_buffer = ogl_context_->vertex_input_get_manager()->get_current_index_buffer();

	if (!index_buffer)
	{
		throw Exception{"Null index buffer."};
	}

	const auto index_byte_depth = index_buffer->get_byte_depth();
	const auto max_index_count = index_buffer->get_size() / index_byte_depth;

	const auto index_byte_offset = command.index_offset_ * index_byte_depth;
	assert(command.index_offset_ < max_index_count);
	assert(command.count_ <= max_index_count);
	assert((command.index_offset_ + command.count_) <= max_index_count);


	// Draw the quads.
	//
	const auto index_buffer_data = reinterpret_cast<const void*>(static_cast<std::intptr_t>(index_byte_offset));

	const auto ogl_element_type = OglRendererUtils::index_buffer_get_element_type_by_byte_depth(
		index_buffer->get_byte_depth());

	index_buffer->set(true);

	::glDrawElements(
		GL_TRIANGLES, // mode
		index_count, // count
		ogl_element_type, // type
		index_buffer_data // indices
	);

	assert(!OglRendererUtils::was_errors());
}

//
// GenericOglCommandExecutor
// =========================================================================


// =========================================================================
// OglCommandExecutorFactory
//

OglCommandExecutorUPtr OglCommandExecutorFactory::create(
	const OglContextPtr ogl_context)
{
	return std::make_unique<GenericOglCommandExecutor>(ogl_context);
}

//
// OglCommandExecutorFactory
// =========================================================================


} // detail
} // bstone
