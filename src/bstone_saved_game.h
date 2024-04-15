/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Saved game stuff.

#ifndef BSTONE_SAVED_GAME_INCLUDED
#define BSTONE_SAVED_GAME_INCLUDED

#include <cstddef>
#include <cstdint>

#include "bstone_memory_stream.h"
#include "bstone_stream.h"
#include "bstone_four_cc.h"

namespace bstone {

constexpr auto sg_version = 11;

struct SgKnownFourCc
{
	static constexpr FourCc vers() { return FourCc{'V', 'E', 'R', 'S'}; }
	static constexpr FourCc desc() { return FourCc{'D', 'E', 'S', 'C'}; }
	static constexpr FourCc head() { return FourCc{'H', 'E', 'A', 'D'}; }
	static constexpr FourCc lvxx() { return FourCc{'L', 'V', 'X', 'X'}; }
};

struct SgChunkHeader
{
public:
	std::uint32_t id;
	std::int32_t size;

public:
	void serialize(Stream& stream) const;
	void deserialize(Stream& stream);
};

static_assert(
	sizeof(SgChunkHeader) == 8 &&
		offsetof(SgChunkHeader, id) == 0 &&
		offsetof(SgChunkHeader, size) == 4,
	"Invalid SgChunkHeader type.");

constexpr auto sg_chunk_header_size = static_cast<int>(sizeof(SgChunkHeader));

FourCc sg_make_numbered_four_cc(char ch_0, char ch_1, int number);
FourCc sg_make_level_four_cc(int number);
FourCc sg_make_overlay_four_cc(int number);

// Returns a position of chunk's data nn success or zero otherwise.
int sg_find_chunk(bstone::FourCc four_cc, bstone::Stream& stream);
void sg_delete_chunk(bstone::FourCc four_cc, bstone::MemoryStream& stream);

} // namespace bstone

#endif // BSTONE_SAVED_GAME_INCLUDED
