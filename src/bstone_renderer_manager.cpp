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
#include "bstone_detail_ogl_2_x_renderer.h"
#include "bstone_detail_ogl_renderer_utils.h"


namespace bstone
{


// ==========================================================================
// RendererManagerImpl
//

class RendererManagerImpl :
	public RendererManager
{
public:
	RendererManagerImpl();

	RendererManagerImpl(
		RendererManagerImpl&& rhs);

	~RendererManagerImpl() override;


	int get_renderer_count() const;

	RendererPtr get_renderer(
		const int index) const;


	void renderer_probe(
		const RendererKind& renderer_path) override;

	const RendererProbe& renderer_probe_get() const override;

	RendererPtr renderer_initialize(
		const RendererInitializeParam& param) override;


private:
	using Renderers = std::vector<RendererPtr>;


	int renderer_count_;

	RendererProbe renderer_probe_;
	Renderers renderers_;

	detail::Ogl2XRenderer ogl_2_x_renderer_;


	bool initialize();

	void uninitialize();
}; // RendererManagerImpl

//
// RendererManagerImpl
// ==========================================================================


// ==========================================================================
// RendererManagerImpl
//

RendererManagerImpl::RendererManagerImpl()
	:
	renderer_probe_{},
	renderers_{}
{
	initialize();
}

RendererManagerImpl::RendererManagerImpl(
	RendererManagerImpl&& rhs)
	:
	renderer_probe_{std::move(rhs.renderer_probe_)},
	renderers_{std::move(rhs.renderers_)}
{
}

RendererManagerImpl::~RendererManagerImpl()
{
	uninitialize();
}

bool RendererManagerImpl::initialize()
{
	detail::OglRendererUtils::load_library();

	renderer_count_ = 0;
	renderers_.resize(0);

	// OpenGL 2.x
	//
	++renderer_count_;
	renderers_.emplace_back(&ogl_2_x_renderer_);

	return true;
}

void RendererManagerImpl::uninitialize()
{
	for (auto renderer : renderers_)
	{
		renderer->uninitialize();
	}

	renderers_.clear();

	detail::OglRendererUtils::unload_library();
}

int RendererManagerImpl::get_renderer_count() const
{
	return renderer_count_;
}

RendererPtr RendererManagerImpl::get_renderer(
	const int index) const
{
	return renderers_[index];
}

void RendererManagerImpl::renderer_probe(
	const RendererKind& renderer_path)
{
	if (renderer_path == RendererKind::auto_detect)
	{
		// OpenGL.
		//

		// OpenGL 2.x.
		//
		if (ogl_2_x_renderer_.probe())
		{
			renderer_probe_ = ogl_2_x_renderer_.probe_get();
		}
	}
	else
	{
		// OpenGL.
		//

		// OpenGL 2.x.
		//
		if (renderer_path == RendererKind::ogl_2_x)
		{
			if (ogl_2_x_renderer_.probe())
			{
				renderer_probe_.kind_ = renderer_path;
			}
		}
	}
}

const RendererProbe& RendererManagerImpl::renderer_probe_get() const
{
	return renderer_probe_;
}

RendererPtr RendererManagerImpl::renderer_initialize(
	const RendererInitializeParam& param)
{
	detail::RendererUtils::validate_initialize_param(param);

	switch (param.renderer_kind_)
	{
	case RendererKind::ogl_2_x:
		if (ogl_2_x_renderer_.initialize(param))
		{
			return &ogl_2_x_renderer_;
		}

		break;

	default:
		break;
	}

	return nullptr;
}

//
// RendererManagerImpl
// ==========================================================================


// ==========================================================================
// RendererManagerFactory
//

RendererManagerUPtr RendererManagerFactory::create()
{
	return RendererManagerUPtr{new RendererManagerImpl{}};
}

//
// RendererManagerFactory
// ==========================================================================


} // bstone
