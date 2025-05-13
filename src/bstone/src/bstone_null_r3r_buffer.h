/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Buffer

#ifndef BSTONE_NULL_R3R_BUFFER_INCLUDED
#define BSTONE_NULL_R3R_BUFFER_INCLUDED

#include <memory>
#include "bstone_r3r_buffer.h"

// ==========================================================================

namespace bstone {

using NullR3rBufferUPtr = std::unique_ptr<R3rBuffer>;

NullR3rBufferUPtr make_null_r3r_buffer(const R3rBufferInitParam& param);

} // namespace bstone

#endif // BSTONE_NULL_R3R_BUFFER_INCLUDED
