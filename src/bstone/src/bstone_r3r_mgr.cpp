/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Manager

#include <vector>
#include "bstone_exception.h"
#include "bstone_r3r_mgr.h"
#include "bstone_r3r_tests.h"
#include "bstone_r3r_utils.h"
#include "bstone_gl_r3r.h"

namespace bstone {

R3rMgr::R3rMgr() noexcept = default;

R3rMgr::~R3rMgr() = default;

R3r* R3rMgr::renderer_initialize(const R3rInitParam& param)
try {
	return do_renderer_initialize(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

namespace {

class R3rMgrImpl final : public R3rMgr
{
public:
	R3rMgrImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr) noexcept;
	~R3rMgrImpl() override;

private:
	R3r* do_renderer_initialize(const R3rInitParam& param) override;

private:
	sys::VideoMgr& video_mgr_;
	sys::WindowMgr& window_mgr_;
	R3rUPtr gl_renderer_{};
};

// --------------------------------------------------------------------------

R3rMgrImpl::~R3rMgrImpl() = default;

R3rMgrImpl::R3rMgrImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr) noexcept
	:
	video_mgr_{video_mgr},
	window_mgr_{window_mgr}
{}

R3r* R3rMgrImpl::do_renderer_initialize(const R3rInitParam& param)
try {
	gl_renderer_ = nullptr;

	R3rUtils::validate_initialize_param(param);

	switch (param.renderer_type)
	{
#ifndef BSTONE_R3R_TEST_NO_GL

#ifndef BSTONE_R3R_TEST_NO_GL_2_0
		case R3rType::gl_2_0:
#endif

#ifndef BSTONE_R3R_TEST_NO_GL_3_2_C
		case R3rType::gl_3_2_core:
#endif

#ifndef BSTONE_R3R_TEST_NO_GLES_2_0
		case R3rType::gles_2_0:
#endif

			gl_renderer_ = make_gl_r3r(video_mgr_, window_mgr_, param);
			return gl_renderer_.get();
#endif // BSTONE_R3R_TEST_NO_GL

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported renderer type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

// ==========================================================================

R3rMgrUPtr make_r3r_mgr(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr)
try {
	return std::make_unique<R3rMgrImpl>(video_mgr, window_mgr);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
