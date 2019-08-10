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
// Renderer's command manager (implementaion).
//


#include "bstone_precompiled.h"
#include "bstone_detail_renderer_command_manager.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// RendererCommandManager
//

RendererCommandManager::RendererCommandManager()
	:
	buffers_{}
{
	buffers_.reserve(reserved_buffer_count);
}

RendererCommandManager::~RendererCommandManager()
{
}

int RendererCommandManager::buffer_get_count() const
{
	return static_cast<int>(buffers_.size());
}

RendererCommandBufferPtr RendererCommandManager::buffer_add(
	const RendererCommandManagerBufferAddParam& param)
{
	if (!validate_param(param))
	{
		return nullptr;
	}

	auto buffer = RendererCommandBufferImplUPtr{new RendererCommandBuffer{}};

	buffer->initialize(param);

	buffers_.push_back(std::move(buffer));

	return buffers_.back().get();
}

void RendererCommandManager::buffer_remove(
	RendererCommandBufferPtr buffer)
{
	if (!buffer)
	{
		assert(!"Null buffer.");

		return;
	}

	std::remove_if(
		buffers_.begin(),
		buffers_.end(),
		[&](const auto& item)
		{
			return item.get() == buffer;
		}
	);
}

RendererCommandBufferPtr RendererCommandManager::buffer_get(
	const int index)
{
	if (index < 0 || index >= buffer_get_count())
	{
		return nullptr;
	}

	return buffers_[index].get();
}

bool RendererCommandManager::validate_param(
	const RendererCommandManagerBufferAddParam& param)
{
	if (param.initial_size_ < 0)
	{
		return false;
	}

	if (param.resize_delta_size_ < 0)
	{
		return false;
	}

	return true;
}

//
// RendererCommandManager
// ==========================================================================


} // detail
} // bstone
