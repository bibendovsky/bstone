/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Former "D3_DASM2.ASM".

#include <cstdint>

constexpr auto planepics_size = 8192;

extern std::uint8_t planepics[planepics_size]; // 4k of ceiling, 4k of floor
