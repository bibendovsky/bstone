/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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
// Ren3dGlSamplerMgrException
//

class Ren3dGlSamplerMgrException :
	public Exception
{
public:
	explicit Ren3dGlSamplerMgrException(
		const char* message) noexcept
		:
		Exception{"REN_3D_GL_SMPLR_MGR", message}
	{
	}
}; // Ren3dGlSamplerMgrException

//
// Ren3dGlSamplerMgrException
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


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


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
try
	:
	context_{context},
	device_features_{context_->get_device_features()},
	default_sampler_{},
	current_sampler_{}
{
	if (!context_)
	{
		fail("Null OpenGL state.");
	}

	initialize_default_sampler();

	current_sampler_ = default_sampler_.get();
}
catch (...)
{
	fail_nested(__func__);
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

[[noreturn]]
void Ren3dGlSamplerMgrImpl::fail(
	const char* message)
{
	throw Ren3dGlSamplerMgrException{message};
}

[[noreturn]]
void Ren3dGlSamplerMgrImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(Ren3dGlSamplerMgrException{message});
}

void Ren3dGlSamplerMgrImpl::initialize_default_sampler()
try
{
	const auto param = Ren3dCreateSamplerParam{};

	default_sampler_ = Ren3dGlSamplerFactory::create(context_, param);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlSamplerMgrImpl::set()
try
{
	if (device_features_.is_sampler_available_)
	{
		current_sampler_->set();
	}
}
catch (...)
{
	fail_nested(__func__);
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
