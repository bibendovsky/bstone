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
// OpenGL state implementation.
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_state.h"
#include "bstone_detail_ogl_buffer.h"
#include "bstone_detail_ogl_renderer_utils.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglState
//

OglState::OglState() = default;

OglState::~OglState() = default;

//
// OglState
// ==========================================================================


// ==========================================================================
// OglErrorState
//

class OglErrorState final :
	public OglState
{
public:
	std::string error_message_;


	OglErrorState() = default;

	~OglErrorState() override = default;


	const std::string& get_error_message() const noexcept override;

	virtual void initialize() override;

	virtual bool is_initialized() const noexcept override;


	void buffer_bind(
		const detail::OglBufferPtr buffer) override;

	void buffer_unbind(
		const RendererBufferKind target) override;
}; // OglErrorState

using OglErrorStatePtr = OglErrorState*;
using OglErrorStateUPtr = std::unique_ptr<OglErrorState>;

//
// OglErrorState
// ==========================================================================


// ==========================================================================
// Ogl2XState
//

class Ogl2XState :
	public OglState
{
public:
	Ogl2XState();

	Ogl2XState(
		const Ogl2XState& rhs) = delete;

	~Ogl2XState() override;


	const std::string& get_error_message() const noexcept override;

	void initialize() override;

	bool is_initialized() const noexcept override;


	void buffer_bind(
		const detail::OglBufferPtr buffer) override;

	void buffer_unbind(
		const RendererBufferKind target) override;


private:
	static constexpr int target_index_count = 2;
	static constexpr int index_buffer_target_index = 0;
	static constexpr int vertex_buffer_target_index = 1;


	using Targets = std::array<detail::OglBufferPtr, target_index_count>;


	bool is_initialized_;
	std::string error_message_;

	Targets targets_;


	bool get_target(
		const RendererBufferKind target_kind,
		GLenum& ogl_target,
		int& target_index);

	void bind_target(
		const int target_index,
		const GLenum ogl_target,
		const detail::OglBufferPtr buffer);
}; // Ogl2XState

using Ogl2XStateImplPtr = Ogl2XState*;
using Ogl2XStateImplUPtr = std::unique_ptr<Ogl2XState>;

//
// Ogl2XState
// ==========================================================================


// ==========================================================================
// OglErrorState
//

const std::string& OglErrorState::get_error_message() const noexcept
{
	return error_message_;
}

void OglErrorState::initialize()
{
}

bool OglErrorState::is_initialized() const noexcept
{
	return false;
}

void OglErrorState::buffer_bind(
	const detail::OglBufferPtr buffer)
{
	static_cast<void>(buffer);
}

void OglErrorState::buffer_unbind(
	const RendererBufferKind target)
{
	static_cast<void>(target);
}

//
// OglErrorState
// ==========================================================================


// ==========================================================================
// Ogl2XState
//

constexpr int Ogl2XState::target_index_count;
constexpr int Ogl2XState::index_buffer_target_index;
constexpr int Ogl2XState::vertex_buffer_target_index;


Ogl2XState::Ogl2XState()
	:
	is_initialized_{},
	error_message_{},
	targets_{}
{
}

Ogl2XState::~Ogl2XState()
{
}

const std::string& Ogl2XState::get_error_message() const noexcept
{
	return error_message_;
}

void Ogl2XState::initialize()
{
	::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	assert(!detail::OglRendererUtils::was_errors());

	::glBindBuffer(GL_ARRAY_BUFFER, 0);
	assert(!detail::OglRendererUtils::was_errors());

	is_initialized_ = true;
}

bool Ogl2XState::is_initialized() const noexcept
{
	return is_initialized_;
}

void Ogl2XState::buffer_bind(
	const detail::OglBufferPtr buffer)
{
	if (buffer == nullptr)
	{
		assert(!"Null buffer.");

		return;
	}

	auto ogl_target = GLenum{};
	auto target_index = 0;

	const auto target_kind = buffer->get_kind();

	if (!get_target(target_kind, ogl_target, target_index))
	{
		return;
	}

	bind_target(target_index, ogl_target, buffer);
}

void Ogl2XState::buffer_unbind(
	const RendererBufferKind target_kind)
{
	auto ogl_target = GLenum{};
	auto target_index = 0;

	if (!get_target(target_kind, ogl_target, target_index))
	{
		return;
	}

	bind_target(target_index, ogl_target, nullptr);
}

bool Ogl2XState::get_target(
	const RendererBufferKind target_kind,
	GLenum& ogl_target,
	int& target_index)
{
	switch (target_kind)
	{
		case RendererBufferKind::index:
			ogl_target = GL_ELEMENT_ARRAY_BUFFER;
			target_index = index_buffer_target_index;
			return true;

		case RendererBufferKind::vertex:
			ogl_target = GL_ARRAY_BUFFER;
			target_index = vertex_buffer_target_index;
			return true;

		default:
			assert(!"Unsupported target kind.");
			return false;
	}
}

void Ogl2XState::bind_target(
	const int target_index,
	const GLenum ogl_target,
	const detail::OglBufferPtr buffer)
{
	if (targets_[target_index] == buffer)
	{
		return;
	}

	const auto is_empty_name = (buffer == nullptr);

	if (!is_empty_name)
	{
		if (!buffer->is_initialized())
		{
			assert(!"Buffer not initialized.");

			return;
		}
	}

	targets_[target_index] = buffer;

	const auto ogl_name = (is_empty_name ? 0 : buffer->get_ogl_name());
	::glBindBuffer(ogl_target, ogl_name);
	assert(!detail::OglRendererUtils::was_errors());
}

//
// Ogl2XState
// ==========================================================================


// =========================================================================
// OglStateFactory
//

OglStateUPtr OglStateFactory::create(
	const RendererKind renderer_kind)
{
	switch (renderer_kind)
	{
		case RendererKind::ogl_2_x:
		{
			auto ogl_state = Ogl2XStateImplUPtr{new Ogl2XState{}};

			ogl_state->initialize();

			if (!ogl_state->is_initialized())
			{
				auto ogl_state_error = OglErrorStateUPtr{new OglErrorState{}};
				ogl_state_error->error_message_ = ogl_state->get_error_message();

				return ogl_state_error;
			}

			return ogl_state;
		}

		default:
			assert("Unsupported renderer kind.");

			return nullptr;
	}
}

//
// OglStateFactory
// =========================================================================


} // detail
} // bstone
