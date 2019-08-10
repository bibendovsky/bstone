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


#ifndef BSTONE_DETAIL_RENDERER_COMMAND_MANAGER_INCLUDED
#define BSTONE_DETAIL_RENDERER_COMMAND_MANAGER_INCLUDED


#include "bstone_detail_renderer_command_buffer.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// RendererCommandManager
//

class RendererCommandManager final :
	public bstone::RendererCommandManager
{
public:
	RendererCommandManager();

	~RendererCommandManager() override;


	int buffer_get_count() const override;

	bstone::RendererCommandBufferPtr buffer_add(
		const RendererCommandManagerBufferAddParam& param) override;

	void buffer_remove(
		bstone::RendererCommandBufferPtr buffer) override;

	bstone::RendererCommandBufferPtr buffer_get(
		const int index) override;


private:
	static constexpr auto reserved_buffer_count = 8;


	using Buffers = std::vector<detail::RendererCommandBufferUPtr>;


	Buffers buffers_;


	static bool validate_param(
		const RendererCommandManagerBufferAddParam& param);
}; // RendererCommandManager

using RendererCommandManagerPtr = detail::RendererCommandManager*;
using RendererCommandManagerUPtr = std::unique_ptr<detail::RendererCommandManager>;

//
// RendererCommandManager
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_RENDERER_COMMAND_MANAGER_INCLUDED
