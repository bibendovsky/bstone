/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

//
// 3D renderer manager (implementation).
//

#include <vector>
#include "bstone_exception.h"
#include "bstone_ren_3d_mgr.h"
#include "bstone_ren_3d_tests.h"
#include "bstone_detail_ren_3d_gl.h"

namespace bstone {

namespace {

class Ren3dMgrImpl final : public Ren3dMgr
{
public:
	Ren3dMgrImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr) noexcept;
	~Ren3dMgrImpl() override = default;

	Ren3dPtr renderer_initialize(const Ren3dCreateParam& param) override;

private:
	sys::VideoMgr& video_mgr_;
	sys::WindowMgr& window_mgr_;
	detail::Ren3dGlUPtr gl_renderer_{};
};

// --------------------------------------------------------------------------

Ren3dMgrImpl::Ren3dMgrImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr) noexcept
	:
	video_mgr_{video_mgr},
	window_mgr_{window_mgr}
{}

Ren3dPtr Ren3dMgrImpl::renderer_initialize(const Ren3dCreateParam& param)
try
{
	gl_renderer_ = nullptr;

	detail::Ren3dUtils::validate_initialize_param(param);

	switch (param.renderer_kind_)
	{
#if !defined(BSTONE_REN_3D_TEST_NO_GL)

#if !defined(BSTONE_REN_3D_TEST_NO_GL_2_0)
		case Ren3dKind::gl_2_0:
#endif

#if !defined(BSTONE_REN_3D_TEST_NO_GL_3_2_C)
		case Ren3dKind::gl_3_2_core:
#endif

#if !defined(BSTONE_REN_3D_TEST_NO_GLES_2_0)
		case Ren3dKind::gles_2_0:
#endif

			gl_renderer_ = std::make_unique<detail::Ren3dGl>(video_mgr_, window_mgr_, param);
			return gl_renderer_.get();
#endif // BSTONE_REN_3D_TEST_NO_GL

		default:
			BSTONE_STATIC_THROW("Unsupported renderer type.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace

// ==========================================================================

Ren3dMgrUPtr Ren3dMgrFactory::create(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr)
{
	return std::make_unique<Ren3dMgrImpl>(video_mgr, window_mgr);
}

} // namespace bstone
