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
// OpenGL vertex input manager (implementation).
//


#include "bstone_detail_ren_3d_gl_vertex_input_mgr.h"

#include "bstone_exception.h"
#include "bstone_ren_3d_tests.h"

#include "bstone_detail_ren_3d_gl_context.h"
#include "bstone_detail_ren_3d_gl_vertex_input.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ren3dGlVertexInputMgrException
//

class Ren3dGlVertexInputMgrException :
	public Exception
{
public:
	explicit Ren3dGlVertexInputMgrException(
		const char* message)
		:
		Exception{"REN_3D_GL_VTX_INP_MGR", message}
	{
	}
}; // Ren3dGlVertexInputMgrException

//
// Ren3dGlVertexInputMgrException
// ==========================================================================


// ==========================================================================
// Ren3dGlVertexInputMgrImpl
//

class Ren3dGlVertexInputMgrImpl final :
	public Ren3dGlVertexInputMgr
{
public:
	Ren3dGlVertexInputMgrImpl(
		const Ren3dGlContextPtr context);

	~Ren3dGlVertexInputMgrImpl() override;


	Ren3dGlContextPtr get_context() const noexcept override;


	Ren3dVertexInputUPtr create(
		const Ren3dCreateVertexInputParam& param) override;

	void set(
		const Ren3dVertexInputPtr vertex_input) override;

	void bind_default_vao() override;


private:
	const Ren3dGlContextPtr context_;

	Ren3dGlVertexInputUPtr default_vertex_input_;


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	void initialize_default_vertex_input();
}; // Ren3dGlVertexInputMgrImpl

//
// Ren3dGlVertexInputMgrImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlVertexInputMgrImpl
//

Ren3dGlVertexInputMgrImpl::Ren3dGlVertexInputMgrImpl(
	const Ren3dGlContextPtr context)
try
	:
	context_{context},
	default_vertex_input_{}
{
	if (!context_)
	{
		fail("Null context.");
	}

	initialize_default_vertex_input();
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dGlVertexInputMgrImpl::~Ren3dGlVertexInputMgrImpl() = default;

Ren3dGlContextPtr Ren3dGlVertexInputMgrImpl::get_context() const noexcept
{
	return context_;
}

Ren3dVertexInputUPtr Ren3dGlVertexInputMgrImpl::create(
	const Ren3dCreateVertexInputParam& param)
try
{
	return Ren3dGlVertexInputFactory::create(this, param);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlVertexInputMgrImpl::set(
	const Ren3dVertexInputPtr vertex_input)
try
{
	if (!vertex_input)
	{
		fail("Null vertex input.");
	}

	static_cast<Ren3dGlVertexInputPtr>(vertex_input)->bind();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlVertexInputMgrImpl::bind_default_vao()
try
{
	if (default_vertex_input_)
	{
		default_vertex_input_->bind_vao();
	}
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]]
void Ren3dGlVertexInputMgrImpl::fail(
	const char* message)
{
	throw Ren3dGlVertexInputMgrException{message};
}

[[noreturn]]
void Ren3dGlVertexInputMgrImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(Ren3dGlVertexInputMgrException{message});
}

void Ren3dGlVertexInputMgrImpl::initialize_default_vertex_input()
try
{
	const auto param = Ren3dCreateVertexInputParam{};
	default_vertex_input_ = Ren3dGlVertexInputFactory::create(this, param);;
	bind_default_vao();
}
catch (...)
{
	fail_nested(__func__);
}

//
// Ren3dGlVertexInputMgrImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlVaoMgrFactory
//

Ren3dGlVertexInputMgrUPtr Ren3dGlVertexInputMgrFactory::create(
	const Ren3dGlContextPtr context)
{
	return std::make_unique<Ren3dGlVertexInputMgrImpl>(context);
}

//
// Ren3dGlVaoMgrFactory
// ==========================================================================


} // detail
} // bstone
