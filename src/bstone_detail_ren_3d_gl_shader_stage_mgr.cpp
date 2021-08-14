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
// OpenGL shader stage manager (implementation).
//


#include "bstone_detail_ren_3d_gl_shader_stage_mgr.h"

#include "bstone_exception.h"
#include "bstone_ren_3d_tests.h"

#include "bstone_detail_ren_3d_gl_context.h"
#include "bstone_detail_ren_3d_gl_error.h"
#include "bstone_detail_ren_3d_gl_shader_stage.h"


namespace bstone
{
namespace detail
{


// ==========================================================================

class Ren3dGlShaderStageMgrException :
	public Exception
{
public:
	explicit Ren3dGlShaderStageMgrException(
		const char* message)
		:
		Exception{"REN_3D_GL_SHADER_STAGE_MGR", message}
	{
	}
}; // Ren3dGlShaderStageMgrException

// ==========================================================================


// ==========================================================================
// Ren3dGlShaderStageMgrImpl
//

class Ren3dGlShaderStageMgrImpl final :
	public Ren3dGlShaderStageMgr
{
public:
	Ren3dGlShaderStageMgrImpl(
		const Ren3dGlContextPtr context) noexcept;

	~Ren3dGlShaderStageMgrImpl() override;


	Ren3dGlContextPtr get_context() const noexcept override;


	Ren3dShaderStageUPtr create(
		const Ren3dCreateShaderStageParam& param) override;

	void set(
		const Ren3dShaderStagePtr shader_stage) override;


private:
	const Ren3dGlContextPtr context_;


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);
}; // Ren3dGlShaderStageMgrImpl

//
// Ren3dGlShaderStageMgrImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderStageMgrImpl
//

Ren3dGlShaderStageMgrImpl::Ren3dGlShaderStageMgrImpl(
	const Ren3dGlContextPtr context) noexcept
	:
	context_{context}
{
}

Ren3dGlShaderStageMgrImpl::~Ren3dGlShaderStageMgrImpl() = default;

Ren3dGlContextPtr Ren3dGlShaderStageMgrImpl::get_context() const noexcept
{
	return context_;
}

Ren3dShaderStageUPtr Ren3dGlShaderStageMgrImpl::create(
	const Ren3dCreateShaderStageParam& param)
try
{
	return Ren3dGlShaderStageFactory::create(this, param);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlShaderStageMgrImpl::set(
	const Ren3dShaderStagePtr shader_stage)
try
{
	if (shader_stage)
	{
		static_cast<Ren3dGlShaderStagePtr>(shader_stage)->set();
	}
	else
	{
		glUseProgram(0);
		Ren3dGlError::ensure_debug();
	}
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]]
void Ren3dGlShaderStageMgrImpl::fail(
	const char* message)
{
	throw Ren3dGlShaderStageMgrException{message};
}

[[noreturn]]
void Ren3dGlShaderStageMgrImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(Ren3dGlShaderStageMgrException{message});
}

//
// Ren3dGlShaderStageMgrImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderStageMgrFactory
//

Ren3dGlShaderStageMgrUPtr Ren3dGlShaderStageMgrFactory::create(
	const Ren3dGlContextPtr context)
{
	return std::make_unique<Ren3dGlShaderStageMgrImpl>(context);
}

//
// Ren3dGlShaderStageMgrFactory
// ==========================================================================


} // detail
} // bstone
