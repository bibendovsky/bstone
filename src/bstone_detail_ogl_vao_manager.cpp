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
// OpenGL vertex array object manager (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_vao_manager.h"

#include <stack>

#include "bstone_exception.h"
#include "bstone_ogl_api.h"
#include "bstone_uptr_resource_list.h"

#include "bstone_detail_ogl_device_features.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_vao.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglVaoManager
//

OglVaoManager::OglVaoManager() = default;

OglVaoManager::~OglVaoManager() = default;

//
// OglVaoManager
// ==========================================================================


// ==========================================================================
// GenericOglVaoManager
//

class GenericOglVaoManager :
	public OglVaoManager
{
public:
	GenericOglVaoManager(
		const OglContextPtr ogl_context,
		const RendererDeviceFeatures& device_features,
		const OglDeviceFeatures& ogl_device_features);

	~GenericOglVaoManager() override;


	const RendererDeviceFeatures& get_device_features() const noexcept override;

	const OglDeviceFeatures& get_ogl_device_features() const noexcept override;


	OglVaoPtr create() override;

	void destroy(
		const OglVaoPtr vao) override;

	void push_current_set_default() override;

	void pop() override;

	void bind(
		const OglVaoPtr vao) override;


	bool set_current_index_buffer(
		const RendererBufferPtr index_buffer) override;


	void enable_location(
		const int location,
		const bool is_enable) override;


private:
	const OglContextPtr ogl_context_;
	const RendererDeviceFeatures& device_features_;
	const OglDeviceFeatures& ogl_device_features_;


	OglVaoPtr vao_current_;
	OglVaoUPtr vao_default_;

	using VaoStack = std::stack<OglVaoPtr>;
	VaoStack vao_stack_;

	using Vaos = UPtrResourceList<OglVao, OglVaoFactory, Exception>;
	Vaos vaos_;


	void initialize_default_vao();

	void bind();

	void initialize();
}; // GenericOglVaoManager

using GenericOglVaoManagerPtr = GenericOglVaoManager*;
using GenericOglVaoManagerUPtr = std::unique_ptr<GenericOglVaoManager>;

//
// GenericOglVaoManager
// ==========================================================================


// ==========================================================================
// GenericOglVaoManager
//

GenericOglVaoManager::GenericOglVaoManager(
	const OglContextPtr ogl_context,
	const RendererDeviceFeatures& device_features,
	const OglDeviceFeatures& ogl_device_features)
	:
	ogl_context_{ogl_context},
	device_features_{device_features},
	ogl_device_features_{ogl_device_features},
	vao_current_{},
	vao_default_{},
	vao_stack_{},
	vaos_{}
{
	initialize();
}

GenericOglVaoManager::~GenericOglVaoManager() = default;

const RendererDeviceFeatures& GenericOglVaoManager::get_device_features() const noexcept
{
	return device_features_;
}

const OglDeviceFeatures& GenericOglVaoManager::get_ogl_device_features() const noexcept
{
	return ogl_device_features_;
}

OglVaoPtr GenericOglVaoManager::create()
{
	if (!ogl_device_features_.vao_is_available_)
	{
		return vao_default_.get();
	}

	return vaos_.add(this);
}

void GenericOglVaoManager::destroy(
	const OglVaoPtr vao)
{
	if (!ogl_device_features_.vao_is_available_)
	{
		if (vao != vao_default_.get())
		{
			throw Exception{"Expected default VAO."};
		}

		return;
	}

	if (!vao)
	{
		throw Exception{"Null vertex array."};
	}

	if (vao_current_ == vao)
	{
		vao_current_ = vao_default_.get();
		bind();
	}

	vaos_.remove(vao);
}

void GenericOglVaoManager::push_current_set_default()
{
	vao_stack_.emplace(vao_current_);

	bind(vao_default_.get());
}

void GenericOglVaoManager::pop()
{
	if (vao_stack_.empty())
	{
		throw Exception{"Empty stack."};
	}

	const auto vao = vao_stack_.top();
	vao_stack_.pop();

	bind(vao);
}

void GenericOglVaoManager::bind(
	const OglVaoPtr vao)
{
	if (!vao)
	{
		throw Exception{"Null VAO."};
	}

	if (vao_current_ == vao)
	{
		return;
	}

	vao_current_ = vao;
	bind();
}

bool GenericOglVaoManager::set_current_index_buffer(
	const RendererBufferPtr index_buffer)
{
	return vao_current_->set_current_index_buffer(index_buffer);
}

void GenericOglVaoManager::enable_location(
	const int location,
	const bool is_enable)
{
	vao_current_->enable_location(location, is_enable);
}

void GenericOglVaoManager::initialize_default_vao()
{
	vao_default_ = OglVaoFactory::create(this);

	vao_current_ = vao_default_.get();
	bind();
}

void GenericOglVaoManager::bind()
{
	vao_current_->bind();
}

void GenericOglVaoManager::initialize()
{
	if (!ogl_context_)
	{
		throw Exception{"Null OpenGL state."};
	}

	initialize_default_vao();
}

//
// GenericOglVaoManager
// ==========================================================================


// ==========================================================================
// OglVaoManagerFactory
//

OglVaoManagerUPtr OglVaoManagerFactory::create(
	const OglContextPtr ogl_context,
	const RendererDeviceFeatures& device_features,
	const OglDeviceFeatures& ogl_device_features)
{
	return std::make_unique<GenericOglVaoManager>(
		ogl_context,
		device_features,
		ogl_device_features
	);
}

//
// OglVaoManagerFactory
// ==========================================================================


} // detail
} // bstone
