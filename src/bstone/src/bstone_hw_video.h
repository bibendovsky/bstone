/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Hardware accelerated video (HW).

#ifndef BSTONE_HW_VIDEO_INCLUDED
#define BSTONE_HW_VIDEO_INCLUDED

#include "bstone_video.h"

namespace bstone {

VideoUPtr make_hw_video();

} // bstone

#endif // BSTONE_HW_VIDEO_INCLUDED
