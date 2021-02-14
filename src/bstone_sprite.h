/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// A sprite.
//


#ifndef BSTONE_SPRITE_INCLUDED
#define BSTONE_SPRITE_INCLUDED


#include <cstdint>

#include <vector>


namespace bstone
{


class Sprite
{
public:
	// Sprite dimension.
	static constexpr auto dimension = 64;


	Sprite();

	Sprite(
		const Sprite& rhs);

	Sprite(
		Sprite&& rhs);

	Sprite& operator=(
		const Sprite& rhs);

	~Sprite();


	void initialize(
		const void* raw_data);

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
	const std::int16_t* get_column(
		const int index) const;

	// Returns a pointer to the columns.
	//
	// Note: Negative value means transparency.
	const std::int16_t* get_data() const;


private:
	using Image = std::vector<std::int16_t>;


	int left_;
	int right_;
	int top_;
	int bottom_;

	Image image_;
}; // Sprite

using SpritePtr = Sprite*;
using SpriteCPtr = const Sprite*;


} // bstone


#endif // !BSTONE_SPRITE_INCLUDED
