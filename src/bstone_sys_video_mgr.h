/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_VIDEO_MGR_INCLUDED)
#define BSTONE_SYS_VIDEO_MGR_INCLUDED

#include <memory>

namespace bstone {
namespace sys {

class VideoMgr
{
public:
	VideoMgr() = default;
	virtual ~VideoMgr() = default;
};

// ==========================================================================

using VideoMgrUPtr = std::unique_ptr<VideoMgr>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_VIDEO_MGR_INCLUDED
