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
// OpenGL 1.x renderer.
//
// !!! Internal usage only !!!
//



#include "bstone_precompiled.h"
#include "bstone_ogl_1_x_renderer.h"
#include <cassert>
#include "bstone_ogl_renderer_utils.h"


namespace bstone
{


// ==========================================================================
// Ogl1XRenderer::IndexBuffer
//

void Ogl1XRenderer::IndexBuffer::update(
	const RendererIndexBufferUpdateParam& param)
{
	if (!RendererUtils::validate_index_buffer_update_param(param, error_message_))
	{
		error_message_ = "Invalid update parameter.";

		return;
	}

	if (param.offset_ >= count_)
	{
		error_message_ = "Offset out of range.";

		return;
	}

	if (param.count_ > count_)
	{
		error_message_ = "Count out of range.";

		return;
	}

	if ((param.offset_ + param.count_) > count_)
	{
		error_message_ = "Block out of range.";

		return;
	}


	const auto offset_in_bytes = param.offset_ * byte_depth_;
	const auto size_in_bytes = param.count_ * byte_depth_;

	std::uninitialized_copy_n(
		static_cast<const std::uint8_t*>(param.indices_),
		size_in_bytes,
		data_.begin() + offset_in_bytes
	);
}

bool Ogl1XRenderer::IndexBuffer::initialize(
	const RendererIndexBufferCreateParam& param)
{
	if (!RendererUtils::validate_index_buffer_create_param(param, error_message_))
	{
		return false;
	}

	auto byte_depth = 0;
	auto data_type = GLenum{};

	if (param.index_count_ <= 0x100)
	{
		byte_depth = 1;
		data_type = GL_UNSIGNED_BYTE;
	}
	else if (param.index_count_ <= 0x10'000)
	{
		byte_depth = 2;
		data_type = GL_UNSIGNED_SHORT;
	}
	else
	{
		byte_depth = 4;
		data_type = GL_UNSIGNED_INT;
	}

	const auto size_in_bytes = param.index_count_ * byte_depth;

	count_ = param.index_count_;
	byte_depth_ = byte_depth;
	size_in_bytes_ = size_in_bytes;
	data_type_ = data_type;
	data_.resize(size_in_bytes);

	return true;
}

//
// Ogl1XRenderer::IndexBuffer
// ==========================================================================


// ==========================================================================
// Ogl1XRenderer::VertexBuffer
//

void Ogl1XRenderer::VertexBuffer::update(
	const RendererVertexBufferUpdateParam& param)
{
	if (!RendererUtils::validate_vertex_buffer_update_param(param, error_message_))
	{
		error_message_ = "Invalid update parameter.";

		return;
	}

	if (param.offset_ >= count_)
	{
		error_message_ = "Offset out of range.";

		return;
	}

	if (param.count_ > count_)
	{
		error_message_ = "Count out of range.";

		return;
	}

	if ((param.offset_ + param.count_) > count_)
	{
		error_message_ = "Block out of range.";

		return;
	}

	std::uninitialized_copy_n(param.vertices_, param.count_, data_.begin() + param.offset_);
}

bool Ogl1XRenderer::VertexBuffer::initialize(
	const RendererVertexBufferCreateParam& param)
{
	if (!RendererUtils::validate_vertex_buffer_create_param(param, error_message_))
	{
		return false;
	}

	count_ = param.vertex_count_;
	data_.resize(param.vertex_count_);

	return true;
}

//
// Ogl1XRenderer::VertexBuffer
// ==========================================================================


// ==========================================================================
// Ogl1XRenderer::Texture2d
//

Ogl1XRenderer::Texture2d::Texture2d(
	Ogl1XRendererPtr renderer)
	:
	renderer_{renderer},
	error_message_{},
	is_npot_{},
	width_{},
	height_{},
	actual_width_{},
	actual_height_{},
	indexed_pixels_{},
	indexed_palette_{},
	indexed_alphas_{}
{
	assert(renderer_);
}

Ogl1XRenderer::Texture2d::~Texture2d()
{
	uninitialize_internal();
}

void Ogl1XRenderer::Texture2d::update(
	const RendererTexture2dUpdateParam& param)
{
	if (!RendererUtils::validate_texture_2d_update_param(param, error_message_))
	{
		return;
	}

	if (param.indexed_pixels_)
	{
		indexed_pixels_ = param.indexed_pixels_;
	}

	if (param.indexed_palette_)
	{
		indexed_palette_ = param.indexed_palette_;
	}

	if (param.indexed_alphas_)
	{
		indexed_alphas_ = param.indexed_alphas_;
	}

	::glBindTexture(GL_TEXTURE_2D, ogl_id_);
	assert(!OglRendererUtils::was_errors());

	update_internal(0);
}

bool Ogl1XRenderer::Texture2d::initialize(
	const RendererTexture2dCreateParam& param)
{
	if (!RendererUtils::validate_texture_2d_create_param(param, error_message_))
	{
		return false;
	}

	width_ = param.width_;
	height_ = param.height_;

	actual_width_ = RendererUtils::find_nearest_pot_value(param.width_);
	actual_height_ = RendererUtils::find_nearest_pot_value(param.height_);

	indexed_pixels_ = param.indexed_pixels_;
	indexed_palette_ = param.indexed_palette_;
	indexed_alphas_ = param.indexed_alphas_;

	is_npot_ = (width_ != actual_width_ || height_ != actual_height_);

	const auto internal_format = (indexed_alphas_ ? GL_RGBA8 : GL_RGB8);

	::glGenTextures(1, &ogl_id_);
	assert(!OglRendererUtils::was_errors());
	assert(ogl_id_ != 0);

	::glBindTexture(GL_TEXTURE_2D, ogl_id_);
	assert(!OglRendererUtils::was_errors());

	::glTexImage2D(
		GL_TEXTURE_2D, // target
		0, // level
		internal_format, // internal format
		actual_width_, // width
		actual_height_, // height
		0, // border
		GL_RGBA, // format
		GL_UNSIGNED_BYTE, // type
		nullptr // pixels
	);

	assert(!OglRendererUtils::was_errors());

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	assert(!OglRendererUtils::was_errors());

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	assert(!OglRendererUtils::was_errors());

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	assert(!OglRendererUtils::was_errors());

	::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	assert(!OglRendererUtils::was_errors());

	update_internal(0);

	return true;
}

void Ogl1XRenderer::Texture2d::uninitialize_internal()
{
	if (ogl_id_)
	{
		::glDeleteTextures(1, &ogl_id_);
		assert(!OglRendererUtils::was_errors());

		ogl_id_ = 0;
	}
}

void Ogl1XRenderer::Texture2d::update_internal(
	const int mipmap_level)
{
	assert(mipmap_level == 0);

	const auto has_alpha = (indexed_alphas_ != nullptr);
	const auto area = actual_width_ * actual_height_;

	auto& buffer = renderer_->texture_buffer_;
	const auto& palette = (indexed_palette_ ? *indexed_palette_ : renderer_->palette_);

	buffer.clear();
	buffer.resize(area);

	if (!is_npot_)
	{
		for (int i = 0; i < area; ++i)
		{
			renderer_->texture_buffer_[i] = palette[indexed_pixels_[i]];

			const auto is_transparent = (!has_alpha || (has_alpha && !indexed_alphas_[i]));

			if (is_transparent)
			{
				buffer[i].a_ = 0x00;
			}
		}
	}
	else
	{
		// Resample.
		//

		const auto src_du_d =
			static_cast<double>(width_) /
			static_cast<double>(actual_width_);

		const auto src_dv_d =
			static_cast<double>(height_) /
			static_cast<double>(actual_height_);

		auto src_v_d = 0.5 * src_dv_d;

		for (int h = 0; h < actual_height_; ++h)
		{
			const auto src_v = static_cast<int>(src_v_d);

			auto src_u_d = 0.5 * src_du_d;

			for (int w = 0; w < actual_width_; ++w)
			{
				const auto src_u = static_cast<int>(src_u_d);

				const auto src_index = (src_v * width_) + src_u;

				const auto dst_index = ((actual_height_ - 1 - h) * actual_width_) + w;

				buffer[dst_index] = palette[indexed_pixels_[src_index]];

				const auto is_transparent = (!has_alpha || (has_alpha && !indexed_alphas_[src_index]));

				if (is_transparent)
				{
					buffer[dst_index].a_ = 0x00;
				}

				src_u_d += src_du_d;
			}

			src_v_d += src_dv_d;
		}
	}

	::glTexSubImage2D(
		GL_TEXTURE_2D, // target
		0, // level
		0, // xoffset
		0, // yoffset
		actual_width_, // width
		actual_height_, // height
		GL_RGBA, // format
		GL_UNSIGNED_BYTE, // type
		buffer.data() // pixels
	);

	assert(!OglRendererUtils::was_errors());
}

//
// Ogl1XRenderer::Texture2d
// ==========================================================================


// ==========================================================================
// Ogl1XRenderer
//

Ogl1XRenderer::Ogl1XRenderer()
	:
	is_initialized_{},
	error_message_{},
	probe_renderer_path_{},
	sdl_window_{},
	sdl_gl_context_{},
	palette_{},
	two_d_projection_matrix_{},
	index_buffers_{},
	vertex_buffers_{},
	texture_buffer_{},
	textures_2d_{}
{
}

Ogl1XRenderer::Ogl1XRenderer(
	Ogl1XRenderer&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	error_message_{std::move(rhs.error_message_)},
	probe_renderer_path_{std::move(rhs.probe_renderer_path_)},
	sdl_window_{std::move(rhs.sdl_window_)},
	sdl_gl_context_{std::move(rhs.sdl_gl_context_)},
	palette_{std::move(rhs.palette_)},
	two_d_projection_matrix_{std::move(rhs.two_d_projection_matrix_)},
	index_buffers_{std::move(rhs.index_buffers_)},
	vertex_buffers_{std::move(rhs.vertex_buffers_)},
	texture_buffer_{std::move(rhs.texture_buffer_)},
	textures_2d_{std::move(rhs.textures_2d_)}
{
	rhs.is_initialized_ = false;
	rhs.sdl_window_ = nullptr;
	rhs.sdl_gl_context_ = nullptr;
}

Ogl1XRenderer::~Ogl1XRenderer()
{
	uninitialize_internal(true);
}

const std::string& Ogl1XRenderer::get_error_message() const
{
	return error_message_;
}

RendererKind Ogl1XRenderer::get_kind() const
{
	return RendererKind::opengl;
}

const std::string& Ogl1XRenderer::get_name() const
{
	static const auto result = std::string{"ogl1"};

	return result;
}

const std::string& Ogl1XRenderer::get_description() const
{
	static const auto result = std::string{"OpenGL 1.x"};

	return result;
}

bool Ogl1XRenderer::probe(
	const RendererPath renderer_path)
{
	return probe_or_initialize(true, renderer_path, RendererInitializeParam{});
}

RendererPath Ogl1XRenderer::get_probe_path() const
{
	return probe_renderer_path_;
}

bool Ogl1XRenderer::is_initialized() const
{
	return is_initialized_;
}

bool Ogl1XRenderer::initialize(
	const RendererInitializeParam& param)
{
	uninitialize_internal();

	return probe_or_initialize(false, RendererPath::none, param);
}

void Ogl1XRenderer::uninitialize()
{
	uninitialize_internal();
}

RendererPath Ogl1XRenderer::get_path() const
{
	if (!is_initialized_)
	{
		return RendererPath::none;
	}

	return RendererPath::ogl_1_x;
}

void Ogl1XRenderer::window_show(
	const bool is_visible)
{
	if (!is_initialized_)
	{
		return;
	}

	static_cast<void>(RendererUtils::show_window(sdl_window_, is_visible, error_message_));
}

void Ogl1XRenderer::color_buffer_set_clear_color(
	const RendererColor32& color)
{
	assert(is_initialized_);

	OglRendererUtils::set_color_buffer_clear_color(color);
}

void Ogl1XRenderer::clear_buffers()
{
	assert(is_initialized_);

	OglRendererUtils::clear_buffers();
}

void Ogl1XRenderer::present()
{
	assert(is_initialized_);

	OglRendererUtils::swap_window(sdl_window_);
}

void Ogl1XRenderer::palette_update(
	const RendererPalette& palette)
{
	if (palette_ == palette)
	{
		return;
	}

	palette_ = palette;

	auto param = RendererTexture2dUpdateParam{};
	param.indexed_palette_ = &palette_;

	for (auto& texture_2d : textures_2d_)
	{
		if (!texture_2d->indexed_palette_)
		{
			texture_2d->update(param);
		}
	}
}

void Ogl1XRenderer::set_2d_projection_matrix(
	const int width,
	const int height)
{
	assert(is_initialized_);
	assert(width > 0);
	assert(height > 0);

	const auto& new_matrix = OglRendererUtils::build_2d_projection_matrix(width, height);

	if (two_d_projection_matrix_ == new_matrix)
	{
		return;
	}

	two_d_projection_matrix_ = new_matrix;
}

RendererIndexBufferPtr Ogl1XRenderer::index_buffer_create(
	const RendererIndexBufferCreateParam& param)
{
	auto index_buffer = IndexBufferUPtr{new IndexBuffer{}};

	if (!index_buffer->initialize(param))
	{
		error_message_ = index_buffer->error_message_;

		return nullptr;
	}

	index_buffers_.push_back(std::move(index_buffer));

	return index_buffers_.back().get();
}

void Ogl1XRenderer::index_buffer_destroy(
	RendererIndexBufferPtr index_buffer)
{
	if (!index_buffer)
	{
		assert(!"Null index buffer.");
	}

	index_buffers_.remove_if(
		[=](const auto& item)
		{
			return item.get() == index_buffer;
		}
	);
}

RendererVertexBufferPtr Ogl1XRenderer::vertex_buffer_create(
	const RendererVertexBufferCreateParam& param)
{
	auto vertex_buffer = VertexBufferUPtr{new VertexBuffer{}};

	if (!vertex_buffer->initialize(param))
	{
		error_message_ = vertex_buffer->error_message_;

		return nullptr;
	}

	vertex_buffers_.push_back(std::move(vertex_buffer));

	return vertex_buffers_.back().get();
}

void Ogl1XRenderer::vertex_buffer_destroy(
	RendererVertexBufferPtr vertex_buffer)
{
	if (!vertex_buffer)
	{
		assert(!"Null vertex buffer.");
	}

	vertex_buffers_.remove_if(
		[=](const auto& item)
		{
			return item.get() == vertex_buffer;
		}
	);
}

void Ogl1XRenderer::execute_command_sets(
	const RendererCommandSets& command_sets)
{
	assert(!command_sets.empty());

	for (auto& command_set : command_sets)
	{
		const auto& commands = command_set.commands_;

		for (int i = 0; i < command_set.count_; ++i)
		{
			const auto& command = commands[i];

			switch (command.id_)
			{
			case RendererCommandId::set_2d:
				execute_command_set_2d(command.set_2d_);
				break;

			case RendererCommandId::draw_quads:
				execute_command_draw_quads(command.draw_quads_);
				break;

			default:
				assert(!"Unsupported command id.");
				break;
			}
		}
	}
}

bool Ogl1XRenderer::probe_or_initialize(
	const bool is_probe,
	const RendererPath probe_renderer_path,
	const RendererInitializeParam& param)
{
	const auto renderer_path = (is_probe ? probe_renderer_path : param.renderer_path_);

	if (renderer_path != RendererPath::ogl_1_x)
	{
		error_message_ = "Invalid renderer path value.";

		return false;
	}

	auto is_succeed = true;
	auto sdl_window = SdlWindowPtr{};
	auto sdl_gl_context = SdlGlContext{};

	if (is_succeed)
	{
		if (is_probe)
		{
			if (!OglRendererUtils::create_probe_window_and_context(sdl_window, sdl_gl_context, error_message_))
			{
				is_succeed = false;
			}
		}
		else
		{
			auto window_param = RendererUtilsCreateWindowParam{};
			window_param.is_opengl_ = true;
			window_param.window_ = param.window_;

			if (!OglRendererUtils::create_window_and_context(window_param, sdl_window, sdl_gl_context, error_message_))
			{
				is_succeed = false;
			}
		}
	}

	if (is_succeed)
	{
		if (!OglRendererUtils::resolve_symbols_1_1())
		{
			error_message_ = "Failed to load OpenGL 1.1 symbols.";

			is_succeed = false;
		}
	}

	if (!is_succeed)
	{
		OglRendererUtils::destroy_window_and_context(sdl_window, sdl_gl_context);

		return false;
	}

	is_initialized_ = true;
	sdl_window_ = sdl_window;
	sdl_gl_context_ = sdl_gl_context;

	if (!is_probe)
	{
		OglRendererUtils::clear_buffers();
		OglRendererUtils::swap_window(sdl_window);
	}

	return true;
}

RendererTexture2dPtr Ogl1XRenderer::texture_2d_create(
	const RendererTexture2dCreateParam& param)
{
	auto texture_2d = Texture2dUPtr{new Texture2d{this}};

	if (!texture_2d->initialize(param))
	{
		error_message_ = texture_2d->error_message_;

		return nullptr;
	}

	textures_2d_.push_back(std::move(texture_2d));

	return textures_2d_.back().get();
}

void Ogl1XRenderer::texture_2d_destroy(
	RendererTexture2dPtr texture_2d)
{
	if (!texture_2d)
	{
		return;
	}

	textures_2d_.remove_if(
		[=](const auto& item)
		{
			return item.get() == texture_2d;
		}
	);
}

void Ogl1XRenderer::uninitialize_internal(
	const bool is_dtor)
{
	if (sdl_gl_context_)
	{
		static_cast<void>(OglRendererUtils::make_context_current(sdl_window_, nullptr, error_message_));
	}

	OglRendererUtils::destroy_window_and_context(sdl_window_, sdl_gl_context_);

	if (!is_dtor)
	{
		palette_ = {};
		two_d_projection_matrix_ = {};
		index_buffers_.clear();
		vertex_buffers_.clear();
		texture_buffer_.clear();
	}

	index_buffers_.clear();
	vertex_buffers_.clear();
	textures_2d_.clear();
}

void Ogl1XRenderer::execute_command_set_2d(
	const RendererCommand::Set2d& command)
{
	static_cast<void>(command);


	// Enable 2D texturing.
	//
	::glEnable(GL_TEXTURE_2D);
	assert(!OglRendererUtils::was_errors());

	// Disable depth test.
	//
	::glDisable(GL_DEPTH_TEST);
	assert(!OglRendererUtils::was_errors());

	// Model-view.
	//
	::glMatrixMode(GL_MODELVIEW);
	assert(!OglRendererUtils::was_errors());

	::glLoadIdentity();
	assert(!OglRendererUtils::was_errors());

	// Projection.
	//
	::glMatrixMode(GL_PROJECTION);
	assert(!OglRendererUtils::was_errors());

	::glLoadMatrixf(two_d_projection_matrix_.get_data());
	assert(!OglRendererUtils::was_errors());
}

void Ogl1XRenderer::execute_command_draw_quads(
	const RendererCommand::DrawQuads& command)
{
	assert(command.count_ > 0);
	assert(command.index_offset_ >= 0);
	assert(command.texture_2d_);
	assert(command.index_buffer_);
	assert(command.vertex_buffer_);

	const auto triangles_per_quad = 2;
	const auto triangle_count = command.count_ * triangles_per_quad;

	const auto indices_per_triangle = 3;
	const auto indices_per_quad = triangles_per_quad * indices_per_triangle;
	const auto index_count = indices_per_quad * command.count_;

	auto& texture_2d = *reinterpret_cast<Texture2d*>(command.texture_2d_);
	auto& index_buffer = *reinterpret_cast<IndexBuffer*>(command.index_buffer_);
	auto& vertex_buffer = *reinterpret_cast<VertexBuffer*>(command.vertex_buffer_);

	assert(command.index_offset_ < index_buffer.count_);
	assert(command.count_ <= index_buffer.count_);
	assert((command.index_offset_ + command.count_) <= index_buffer.count_);

	const auto stride = static_cast<GLsizei>(sizeof(RendererVertex));
	const auto vertex_buffer_data = reinterpret_cast<const std::uint8_t*>(vertex_buffer.data_.data());

	::glBindTexture(GL_TEXTURE_2D, texture_2d.ogl_id_);
	assert(!OglRendererUtils::was_errors());

	// Diffuse.
	//
	::glEnableClientState(GL_COLOR_ARRAY);
	assert(!OglRendererUtils::was_errors());

	::glColorPointer(
		4, // size
		GL_UNSIGNED_BYTE, // type
		stride, // stride
		vertex_buffer_data + offsetof(RendererVertex, rgba_) // pointer
	);

	assert(!OglRendererUtils::was_errors());

	// Texture coordinates.
	//
	::glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	assert(!OglRendererUtils::was_errors());

	::glTexCoordPointer(
		2, // size
		GL_FLOAT, // type
		stride, // stride
		vertex_buffer_data + offsetof(RendererVertex, uv_) // pointer
	);

	assert(!OglRendererUtils::was_errors());

	// Position.
	//
	::glEnableClientState(GL_VERTEX_ARRAY);
	assert(!OglRendererUtils::was_errors());

	::glVertexPointer(
		3, // size
		GL_FLOAT, // type
		stride, // stride
		vertex_buffer_data + offsetof(RendererVertex, xyz_) // pointer
	);

	assert(!OglRendererUtils::was_errors());

	// Draw the quads.
	//

	const auto index_buffer_data =
		index_buffer.data_.data() +
		(index_buffer.byte_depth_ * command.index_offset_);

	::glDrawElements(
		GL_TRIANGLES, // mode
		index_count, // count
		index_buffer.data_type_, // type
		index_buffer_data // indices
	);

	assert(!OglRendererUtils::was_errors());

	// Disable the state.
	//
	::glBindTexture(GL_TEXTURE_2D, 0);
	assert(!OglRendererUtils::was_errors());

	::glDisableClientState(GL_VERTEX_ARRAY);
	assert(!OglRendererUtils::was_errors());

	::glDisableClientState(GL_COLOR_ARRAY);
	assert(!OglRendererUtils::was_errors());

	::glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	assert(!OglRendererUtils::was_errors());
}

//
// Ogl1XRenderer
// ==========================================================================


} // bstone
