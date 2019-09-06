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
// Renderer manager (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_renderer_manager.h"

#include <vector>

#include "bstone_exception.h"
#include "bstone_renderer_tests.h"

#include "bstone_detail_ogl_renderer.h"
#include "bstone_detail_ogl_renderer_utils.h"


namespace bstone
{


// ==========================================================================
// RendererManagerImpl
//

class GenericRendererManager :
	public RendererManager
{
public:
	GenericRendererManager();

	GenericRendererManager(
		GenericRendererManager&& rhs);

	~GenericRendererManager() override;


	RendererPtr renderer_initialize(
		const RendererCreateParam& param) override;


private:
	detail::OglRendererUPtr ogl_renderer_;


	void initialize();

	void uninitialize();

	void uninitialize_renderers();
}; // RendererManagerImpl

//
// RendererManagerImpl
// ==========================================================================


// ==========================================================================
// RendererManagerImpl
//

GenericRendererManager::GenericRendererManager()
	:
	ogl_renderer_{}
{
	initialize();
}

GenericRendererManager::GenericRendererManager(
	GenericRendererManager&& rhs)
	:
	ogl_renderer_{std::move(rhs.ogl_renderer_)}
{
}

GenericRendererManager::~GenericRendererManager()
{
	uninitialize();
}

void GenericRendererManager::initialize()
{
	detail::OglRendererUtils::load_library();
}

void GenericRendererManager::uninitialize()
{
	uninitialize_renderers();

	detail::OglRendererUtils::unload_library();
}

void GenericRendererManager::uninitialize_renderers()
{
	ogl_renderer_ = nullptr;
}

RendererPtr GenericRendererManager::renderer_initialize(
	const RendererCreateParam& param)
{
	uninitialize_renderers();

	detail::RendererUtils::validate_initialize_param(param);

	auto new_param = param;

	const auto is_auto_detect = (param.renderer_kind_ == RendererKind::auto_detect);

	using RendererKindList = std::vector<RendererKind>;

	auto renderer_kind_list = RendererKindList{};

	if (is_auto_detect)
	{
#ifndef BSTONE_RENDERER_HW_TEST_NO_OGL
		renderer_kind_list =
		{
#ifndef BSTONE_RENDERER_HW_TEST_NO_OGL_3_2_CORE
			RendererKind::ogl_3_2_core,
#endif // !BSTONE_RENDERER_HW_TEST_NO_OGL_3_2_CORE

#ifndef BSTONE_RENDERER_HW_TEST_NO_OGL_2
			RendererKind::ogl_2,
#endif // !BSTONE_RENDERER_HW_TEST_NO_OGL_2
		};
#endif // BSTONE_RENDERER_HW_TEST_NO_OGL
	}
	else
	{
		renderer_kind_list = {param.renderer_kind_};
	}

	for (const auto renderer_kind : renderer_kind_list)
	{
		new_param.renderer_kind_ = renderer_kind;

		try
		{
			switch (renderer_kind)
			{
#ifndef BSTONE_RENDERER_HW_TEST_NO_OGL

#ifndef BSTONE_RENDERER_HW_TEST_NO_OGL_2
				case RendererKind::ogl_2:
#endif // !BSTONE_RENDERER_HW_TEST_NO_OGL_2

#ifndef BSTONE_RENDERER_HW_TEST_NO_OGL_3_2_CORE
				case RendererKind::ogl_3_2_core:
#endif // !BSTONE_RENDERER_HW_TEST_NO_OGL_3_2_CORE

					ogl_renderer_ = std::make_unique<detail::OglRenderer>(new_param);

					return ogl_renderer_.get();
#endif // BSTONE_RENDERER_HW_TEST_NO_OGL

				default:
					throw Exception{"Unsupported renderer kind."};

			}
		}
		catch (const Exception&)
		{
			if (!is_auto_detect)
			{
				throw;
			}
		}
	}

	throw Exception{"No hardware renderer detected."};
}

//
// RendererManagerImpl
// ==========================================================================


// ==========================================================================
// RendererManagerFactory
//

RendererManagerUPtr RendererManagerFactory::create()
{
	return std::make_unique<GenericRendererManager>();
}

//
// RendererManagerFactory
// ==========================================================================


} // bstone
