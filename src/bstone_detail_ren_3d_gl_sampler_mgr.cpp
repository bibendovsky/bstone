/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// OpenGL sampler object manager (implementation).
//


#include "bstone_detail_ren_3d_gl_sampler_mgr.h"

#include "bstone_exception.h"
#include "bstone_ren_3d_tests.h"

#include "bstone_detail_ren_3d_gl_sampler.h"
#include "bstone_detail_ren_3d_gl_context.h"
#include "bstone_detail_ren_3d_gl_texture_2d.h"
#include "bstone_detail_ren_3d_gl_texture_mgr.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ren3dGlSamplerMgrImplException
//

class Ren3dGlSamplerMgrImplException :
	public Exception
{
public:
	explicit Ren3dGlSamplerMgrImplException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_GL_SMPLR_MGR] "} + message}
	{
	}
}; // Ren3dGlSamplerMgrImplException

//
// Ren3dGlSamplerMgrImplException
// ==========================================================================


// ==========================================================================
// Ren3dGlSamplerMgrImplCreateException
//

class Ren3dGlSamplerMgrImplCreateException :
	public Exception
{
public:
	explicit Ren3dGlSamplerMgrImplCreateException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_GL_SMPLR_MGR_INIT] "} + message}
	{
	}
}; // Ren3dGlSamplerMgrImplCreateException

//
// Ren3dGlSamplerMgrImplCreateException
// ==========================================================================


// ==========================================================================
// Ren3dGlSamplerMgrImpl
//

class Ren3dGlSamplerMgrImpl final :
	public Ren3dGlSamplerMgr
{
public:
	Ren3dGlSamplerMgrImpl(
		const Ren3dGlContextPtr context);

	~Ren3dGlSamplerMgrImpl() override;


	Ren3dSamplerUPtr create(
		const Ren3dCreateSamplerParam& param) override;

	void notify_destroy(
		const Ren3dSamplerPtr sampler) noexcept override;

	void set(
		const Ren3dSamplerPtr sampler) override;

	const Ren3dSamplerState& get_current_state() const noexcept override;


private:
	const Ren3dGlContextPtr context_;
	const Ren3dDeviceFeatures& device_features_;

	Ren3dGlSamplerUPtr default_sampler_;
	Ren3dGlSamplerPtr current_sampler_;


	void initialize_default_sampler();

	void set();
}; // Ren3dGlSamplerMgrImpl

//
// Ren3dGlSamplerMgrImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlSamplerMgrImpl
//

Ren3dGlSamplerMgrImpl::Ren3dGlSamplerMgrImpl(
	const Ren3dGlContextPtr context)
	:
	context_{context},
	device_features_{context_->get_device_features()},
	default_sampler_{},
	current_sampler_{}
{
	if (!context_)
	{
		throw Ren3dGlSamplerMgrImplCreateException{"Null OpenGL state."};
	}

	initialize_default_sampler();

	current_sampler_ = default_sampler_.get();
}

Ren3dGlSamplerMgrImpl::~Ren3dGlSamplerMgrImpl() = default;

Ren3dSamplerUPtr Ren3dGlSamplerMgrImpl::create(
	const Ren3dCreateSamplerParam& param)
{
	return Ren3dGlSamplerFactory::create(context_, param);
}

void Ren3dGlSamplerMgrImpl::notify_destroy(
	const Ren3dSamplerPtr sampler) noexcept
{
	if (current_sampler_ == sampler)
	{
		if (device_features_.is_sampler_available_)
		{
			current_sampler_ = nullptr;
		}
		else
		{
			current_sampler_ = default_sampler_.get();
		}
	}
}

void Ren3dGlSamplerMgrImpl::set(
	const Ren3dSamplerPtr sampler)
{
	const auto new_sampler = (sampler ? sampler : default_sampler_.get());

	if (current_sampler_ == new_sampler)
	{
		return;
	}

	current_sampler_ = static_cast<Ren3dGlSamplerPtr>(new_sampler);
	set();
}

const Ren3dSamplerState& Ren3dGlSamplerMgrImpl::get_current_state() const noexcept
{
	return current_sampler_->get_state();
}

void Ren3dGlSamplerMgrImpl::initialize_default_sampler()
{
	const auto param = Ren3dCreateSamplerParam{};

	default_sampler_ = Ren3dGlSamplerFactory::create(context_, param);
}

void Ren3dGlSamplerMgrImpl::set()
{
	if (device_features_.is_sampler_available_)
	{
		current_sampler_->set();
	}
}

//
// Ren3dGlSamplerMgrImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlSamplerMgrFactory
//

Ren3dGlSamplerMgrUPtr Ren3dGlSamplerMgrFactory::create(
	const Ren3dGlContextPtr context)
{
	return std::make_unique<Ren3dGlSamplerMgrImpl>(context);
}

//
// Ren3dGlSamplerMgrFactory
// ==========================================================================


} // detail
} // bstone
