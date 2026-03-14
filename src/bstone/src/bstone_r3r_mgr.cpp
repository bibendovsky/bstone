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
#ifndef NDEBUG
#include "bstone_null_r3r.h"
#endif
#include "bstone_gl_r3r.h"
#include "bstone_vk_r3r.h"

namespace bstone {

namespace {

class R3rMgrImpl final : public R3rMgr
{
public:
	R3rMgrImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr);
	~R3rMgrImpl() override = default;

	R3r* make_renderer(const R3rInitParam& param) override;

private:
	sys::VideoMgr& video_mgr_;
	sys::WindowMgr& window_mgr_;
	R3rUPtr r3r_{};
};

// --------------------------------------------------------------------------

R3rMgrImpl::R3rMgrImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr)
	:
	video_mgr_{video_mgr},
	window_mgr_{window_mgr}
{}

R3r* R3rMgrImpl::make_renderer(const R3rInitParam& param)
try {
	r3r_ = nullptr;

	R3rUtils::validate_initialize_param(param);

	switch (param.renderer_type)
	{
#ifndef NDEBUG
		case R3rType::null:
			r3r_ = make_null_r3r(video_mgr_, window_mgr_, param);
			return r3r_.get();
#endif // NDEBUG

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

			r3r_ = make_gl_r3r(video_mgr_, window_mgr_, param);
			return r3r_.get();
#endif // BSTONE_R3R_TEST_NO_GL

		case R3rType::vulkan:
			r3r_ = make_vk_r3r(video_mgr_, window_mgr_, param);
			return r3r_.get();

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
