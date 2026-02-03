/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// A sprite.

#ifndef BSTONE_SPRITE_INCLUDED
#define BSTONE_SPRITE_INCLUDED

#include <cstdint>
#include <vector>

namespace bstone {

class Sprite
{
public:
	static constexpr int dimension = 64;

	void initialize(const void* raw_data, int raw_data_size);
	void uninitialize();

	// Returns true if sprite is initialized.
	bool is_initialized() const;

	// Return an offset by x of the left edge.
	int get_left() const;

	// Return an offset by x of the right edge.
	int get_right() const;

	// Return an offset by y of the top edge.
	int get_top() const;

	// Return an offset by y of the bottm edge.
	int get_bottom() const;

	// Returns a width of the sprite.
	int get_width() const;

	// Returns a height of the sprite.
	int get_height() const;

	// Returns a pointer to the column data.
	//
	// Note: Negative value means transparency.
	const std::int16_t* get_column(int index) const;

	// Returns a pointer to the columns.
	//
	// Note: Negative value means transparency.
	const std::int16_t* get_data() const;

private:
	using Image = std::vector<std::int16_t>;

	int left_{};
	int right_{};
	int top_{};
	int bottom_{};
	Image image_{};
};

} // namespace bstone

#endif // BSTONE_SPRITE_INCLUDED
