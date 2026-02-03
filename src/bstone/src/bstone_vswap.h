/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// VSWAP file

#ifndef BSTONE_VSWAP_INCLUDED
#define BSTONE_VSWAP_INCLUDED

#include <cstdint>
#include <memory>

namespace bstone {

class Vswap
{
public:
	Vswap() = default;
	virtual ~Vswap() = default;

	virtual int get_chunk_count() const = 0;
	virtual int get_wall_count() const = 0;
	virtual int get_sprite_count() const = 0;
	virtual int get_audio_count() const = 0;
	virtual int get_chunk_size(int index) const = 0;
	virtual int get_wall_size(int index) const = 0;
	virtual int get_sprite_size(int index) const = 0;
	virtual int get_audio_size(int index) const = 0;
	virtual const std::uint8_t* get_chunk_data(int index) const = 0;
	virtual const std::uint8_t* get_wall_data(int index) const = 0;
	virtual const std::uint8_t* get_sprite_data(int index) const = 0;
	virtual const std::uint8_t* get_audio_data(int index) const = 0;
};

// =====================================

using VswapUPtr = std::unique_ptr<Vswap>;

VswapUPtr make_vswap();

} // namespace bstone

#endif // BSTONE_VSWAP_INCLUDED
