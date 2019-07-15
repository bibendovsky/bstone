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
#include "bstone_detail_ogl_1_x_renderer.h"
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
	RendererManagerImpl() = default;

	~RendererManagerImpl() override = default;


	bool is_initialized() const override;

	const std::string& get_error_message() const override;

	bool initialize() override;

	void uninitialize() override;


	bool renderer_probe(
		const RendererPath& renderer_path) override;

	const RendererProbe& renderer_probe_get() const override;

	RendererPtr renderer_initialize(
		const RendererInitializeParam& param) override;


private:
	class Impl;


	static Impl& get_impl();
}; // RendererManager

//
// RendererManagerImpl
// ==========================================================================


// ==========================================================================
// RendererManagerImpl::Impl
//

class RendererManagerImpl::Impl
{
public:
	Impl();

	Impl(
		Impl&& rhs);

	~Impl();


	bool is_initialized() const;

	const std::string& get_error_message() const;

	bool initialize();

	void uninitialize();


	int get_renderer_count() const;

	RendererPtr get_renderer(
		const int index) const;


	bool renderer_probe(
		const RendererPath& renderer_path);

	const RendererProbe& renderer_probe_get() const;

	RendererPtr renderer_initialize(
		const RendererInitializeParam& param);


private:
	using Renderers = std::vector<RendererPtr>;


	bool is_initialized_;
	std::string error_message_;

	int renderer_count_;

	RendererProbe renderer_probe_;
	Renderers renderers_;

	detail::Ogl1XRenderer ogl_1_x_renderer_;
	detail::Ogl2XRenderer ogl_2_x_renderer_;
}; // RendererManagerImpl::Impl

//
// RendererManagerImpl::Impl
// ==========================================================================


// ==========================================================================
// RendererManagerImpl::Impl
//

RendererManagerImpl::Impl::Impl()
	:
	is_initialized_{},
	error_message_{},
	renderer_probe_{},
	renderers_{},
	ogl_1_x_renderer_{}
{
}

RendererManagerImpl::Impl::Impl(
	Impl&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	error_message_{std::move(rhs.error_message_)},
	renderer_probe_{std::move(rhs.renderer_probe_)},
	renderers_{std::move(rhs.renderers_)},
	ogl_1_x_renderer_{std::move(rhs.ogl_1_x_renderer_)}
{
	rhs.is_initialized_ = false;
}

RendererManagerImpl::Impl::~Impl()
{
}

bool RendererManagerImpl::Impl::is_initialized() const
{
	return is_initialized_;
}

const std::string& RendererManagerImpl::Impl::get_error_message() const
{
	return error_message_;
}

bool RendererManagerImpl::Impl::initialize()
{
	if (is_initialized_)
	{
		return true;
	}

	auto ogl_renderer_utils = detail::OglRendererUtils{};

	if (!ogl_renderer_utils.load_library())
	{
		return false;
	}

	is_initialized_ = true;

	renderer_count_ = 0;
	renderers_.resize(0);

	// OpenGL 1.x
	//
	++renderer_count_;
	renderers_.emplace_back(&ogl_1_x_renderer_);

	// OpenGL 2.x
	//
	++renderer_count_;
	renderers_.emplace_back(&ogl_2_x_renderer_);

	return true;
}

void RendererManagerImpl::Impl::uninitialize()
{
	is_initialized_ = false;

	for (auto renderer : renderers_)
	{
		renderer->uninitialize();
	}

	renderers_.clear();

	detail::OglRendererUtils::unload_library();
}

int RendererManagerImpl::Impl::get_renderer_count() const
{
	if (!is_initialized_)
	{
		return 0;
	}

	return renderer_count_;
}

RendererPtr RendererManagerImpl::Impl::get_renderer(
	const int index) const
{
	if (!is_initialized_)
	{
		return nullptr;
	}

	if (index < 0 || index >= renderer_count_)
	{
		return nullptr;
	}

	return renderers_[index];
}

bool RendererManagerImpl::Impl::renderer_probe(
	const RendererPath& renderer_path)
{
	if (!is_initialized_)
	{
		return false;
	}

	if (renderer_path == RendererPath::auto_detect)
	{
		// OpenGL.
		//

		// OpenGL 2.x.
		//
		if (ogl_2_x_renderer_.probe())
		{
			renderer_probe_ = ogl_2_x_renderer_.probe_get();

			return true;
		}

		// OpenGL 1.x.
		//
		if (ogl_1_x_renderer_.probe())
		{
			renderer_probe_ = ogl_1_x_renderer_.probe_get();

			return true;
		}
	}
	else
	{
		// OpenGL.
		//

		// OpenGL 2.x.
		//
		if (renderer_path == RendererPath::ogl_2_x)
		{
			if (ogl_2_x_renderer_.probe())
			{
				renderer_probe_.path_ = renderer_path;

				return true;
			}
		}

		// OpenGL 1.x.
		//
		if (renderer_path == RendererPath::ogl_1_x)
		{
			if (ogl_1_x_renderer_.probe())
			{
				renderer_probe_.path_ = renderer_path;

				return true;
			}
		}
	}

	return false;
}

const RendererProbe& RendererManagerImpl::Impl::renderer_probe_get() const
{
	return renderer_probe_;
}

RendererPtr RendererManagerImpl::Impl::renderer_initialize(
	const RendererInitializeParam& param)
{
	auto renderer_utils = detail::RendererUtils{};

	if (!renderer_utils.validate_initialize_param(param))
	{
		return nullptr;
	}

	switch (param.renderer_path_)
	{
	case RendererPath::ogl_1_x:
		if (ogl_1_x_renderer_.initialize(param))
		{
			return &ogl_1_x_renderer_;
		}

		break;

	case RendererPath::ogl_2_x:
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
// RendererManagerImpl::Impl
// ==========================================================================


// ==========================================================================
// RendererManagerImpl
//

bool RendererManagerImpl::is_initialized() const
{
	auto& impl = get_impl();

	return impl.is_initialized();
}

const std::string& RendererManagerImpl::get_error_message() const
{
	auto& impl = get_impl();

	return impl.get_error_message();
}

bool RendererManagerImpl::initialize()
{
	auto& impl = get_impl();

	return impl.initialize();
}

void RendererManagerImpl::uninitialize()
{
	auto& impl = get_impl();

	impl.uninitialize();
}

bool RendererManagerImpl::renderer_probe(
	const RendererPath& renderer_path)
{
	auto& impl = get_impl();

	return impl.renderer_probe(renderer_path);
}

const RendererProbe& RendererManagerImpl::renderer_probe_get() const
{
	auto& impl = get_impl();

	return impl.renderer_probe_get();
}

RendererPtr RendererManagerImpl::renderer_initialize(
	const RendererInitializeParam& param)
{
	auto& impl = get_impl();

	return impl.renderer_initialize(param);
}

RendererManagerImpl::Impl& RendererManagerImpl::get_impl()
{
	static auto result = Impl{};

	return result;
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
