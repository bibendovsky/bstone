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
#include "bstone_ogl_renderer.h"
#include "bstone_ogl_renderer_utils.h"


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


	int get_renderer_count() const override;

	RendererPtr get_renderer(
		const int index) const override;


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


private:
	using Renderers = std::vector<RendererPtr>;


	static const auto renderer_count = 1;

	static const auto ogl_index = 0;


	bool is_initialized_;
	std::string error_message_;

	Renderers renderers_;

	OglRenderer ogl_renderer_;
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
	renderers_{},
	ogl_renderer_{}
{
}

RendererManagerImpl::Impl::Impl(
	Impl&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	error_message_{std::move(rhs.error_message_)},
	renderers_{std::move(rhs.renderers_)},
	ogl_renderer_{std::move(rhs.ogl_renderer_)}
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

	if (!OglRendererUtils::load_library(error_message_))
	{
		return false;
	}

	renderers_.resize(renderer_count);
	renderers_[ogl_index] = &ogl_renderer_;

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

	OglRendererUtils::unload_library();
}

int RendererManagerImpl::Impl::get_renderer_count() const
{
	if (!is_initialized_)
	{
		return 0;
	}

	return renderer_count;
}

RendererPtr RendererManagerImpl::Impl::get_renderer(
	const int index) const
{
	if (!is_initialized_)
	{
		return nullptr;
	}

	if (index < 0 || index >= renderer_count)
	{
		return nullptr;
	}

	return renderers_[index];
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

int RendererManagerImpl::get_renderer_count() const
{
	auto& impl = get_impl();

	return impl.get_renderer_count();
}

RendererPtr RendererManagerImpl::get_renderer(
	const int index) const
{
	auto& impl = get_impl();

	return impl.get_renderer(index);
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
