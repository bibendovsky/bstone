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
// OpenGL extension registry (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_extension_registry.h"
#include "SDL_video.h"
#include "bstone_detail_ogl_renderer_utils.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglExtensionRegistryImpl
//

class OglExtensionRegistryImpl :
	public OglExtensionRegistry
{
public:
	OglExtensionRegistryImpl();

	OglExtensionRegistryImpl(
		const OglExtensionRegistryImpl& rhs) = delete;

	OglExtensionRegistryImpl(
		OglExtensionRegistryImpl&& rhs);

	~OglExtensionRegistryImpl() override;


	void extension_probe(
		const OglExtensionId extension_id) override;


	bool has_extension(
		const OglExtensionId extension_id) const override;


	bool initialize();
}; // OglExtensionRegistryImpl

using OglExtensionRegistryImplPtr = OglExtensionRegistryImpl*;
using OglExtensionRegistryImplUPtr = std::unique_ptr<OglExtensionRegistryImpl>;


OglExtensionRegistryImpl::OglExtensionRegistryImpl()
{
}

OglExtensionRegistryImpl::OglExtensionRegistryImpl(
	OglExtensionRegistryImpl&& rhs)
{
}

OglExtensionRegistryImpl::~OglExtensionRegistryImpl()
{
}

bool OglExtensionRegistryImpl::initialize()
{
	const auto ogl_context = ::SDL_GL_GetCurrentContext();

	if (ogl_context == nullptr)
	{
		return false;
	}

	auto profile_mask = 0;

	const auto sdl_result = ::SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile_mask);

	if (sdl_result != 0)
	{
		return false;
	}

	return true;
}

void OglExtensionRegistryImpl::extension_probe(
	const OglExtensionId extension_id)
{
}

bool OglExtensionRegistryImpl::has_extension(
	const OglExtensionId extension_id) const
{
	return false;
}

//
// OglExtensionRegistryImpl
// ==========================================================================


// ==========================================================================
// OglExtensionRegistry
//

OglExtensionRegistry::OglExtensionRegistry()
{
}

OglExtensionRegistry::~OglExtensionRegistry()
{
}

//
// OglExtensionRegistry
// ==========================================================================


// ==========================================================================
// OglExtensionRegistryFactory
//

OglExtensionRegistryUPtr OglExtensionRegistryFactory::create()
{
	auto result = OglExtensionRegistryImplUPtr{new OglExtensionRegistryImpl{}};

	if (!result->initialize())
	{
		return nullptr;
	}

	return result;
}

//
// OglExtensionRegistryFactory
// ==========================================================================


} // detail
} // bstone
