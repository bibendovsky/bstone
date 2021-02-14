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
// 3D renderer manager (implementation).
//


#include "bstone_ren_3d_mgr.h"

#include <vector>

#include "bstone_exception.h"
#include "bstone_ren_3d_tests.h"

#include "bstone_detail_ren_3d_gl.h"
#include "bstone_detail_ren_3d_gl_utils.h"


namespace bstone
{


// ==========================================================================
// Ren3dMgrImplException
//

class Ren3dMgrImplException :
	public Exception
{
public:
	explicit Ren3dMgrImplException(
		const char* const message)
		:
		Exception{std::string{"[REN_3D_MGR] "} + message}
	{
	}
}; // Ren3dMgrImplException

//
// Ren3dMgrImplException
// ==========================================================================


// ==========================================================================
// Ren3dMgrImpl
//

class Ren3dMgrImpl :
	public Ren3dMgr
{
public:
	Ren3dMgrImpl();

	Ren3dMgrImpl(
		Ren3dMgrImpl&& rhs);

	~Ren3dMgrImpl() override;


	Ren3dPtr renderer_initialize(
		const Ren3dCreateParam& param) override;


private:
	detail::Ren3dGlUPtr gl_renderer_;


	void initialize();

	void uninitialize();

	void uninitialize_renderers();
}; // Ren3dMgrImpl

//
// Ren3dMgrImpl
// ==========================================================================


// ==========================================================================
// Ren3dMgrImpl
//

Ren3dMgrImpl::Ren3dMgrImpl()
	:
	gl_renderer_{}
{
	initialize();
}

Ren3dMgrImpl::Ren3dMgrImpl(
	Ren3dMgrImpl&& rhs)
	:
	gl_renderer_{std::move(rhs.gl_renderer_)}
{
}

Ren3dMgrImpl::~Ren3dMgrImpl()
{
	uninitialize();
}

void Ren3dMgrImpl::initialize()
{
	detail::Ren3dGlUtils::load_library();
}

void Ren3dMgrImpl::uninitialize()
{
	uninitialize_renderers();

	detail::Ren3dGlUtils::unload_library();
}

void Ren3dMgrImpl::uninitialize_renderers()
{
	gl_renderer_ = nullptr;
}

Ren3dPtr Ren3dMgrImpl::renderer_initialize(
	const Ren3dCreateParam& param)
{
	uninitialize_renderers();

	detail::Ren3dUtils::validate_initialize_param(param);

	switch (param.renderer_kind_)
	{
#ifndef BSTONE_REN_3D_TEST_NO_GL

#ifndef BSTONE_REN_3D_TEST_NO_GL_2_0
		case Ren3dKind::gl_2_0:
#endif // !BSTONE_REN_3D_TEST_NO_GL_2_0

#ifndef BSTONE_REN_3D_TEST_NO_GL_3_2_C
		case Ren3dKind::gl_3_2_core:
#endif // !BSTONE_REN_3D_TEST_NO_GL_3_2_C

#ifndef BSTONE_REN_3D_TEST_NO_GLES_2_0
		case Ren3dKind::gles_2_0:
#endif // !BSTONE_REN_3D_TEST_NO_GLES_2_0

			gl_renderer_ = std::make_unique<detail::Ren3dGl>(param);

			return gl_renderer_.get();
#endif // BSTONE_REN_3D_TEST_NO_GL

		default:
			throw Ren3dMgrImplException{"Unsupported renderer kind."};
	}
}

//
// Ren3dMgrImpl
// ==========================================================================


// ==========================================================================
// Ren3dMgrFactory
//

Ren3dMgrUPtr Ren3dMgrFactory::create()
{
	return std::make_unique<Ren3dMgrImpl>();
}

//
// Ren3dMgrFactory
// ==========================================================================


} // bstone
