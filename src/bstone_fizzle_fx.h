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


#ifndef BSTONE_FIZZLE_FX_INCLUDED
#define BSTONE_FIZZLE_FX_INCLUDED


//
// Fizzle effect (base class).
//


namespace bstone
{


class FizzleFX
{
public:
	FizzleFX() = default;

	FizzleFX(
		const FizzleFX& rhs) = delete;

	FizzleFX(
		FizzleFX&& rhs) = default;

	FizzleFX& operator=(
		const FizzleFX& rhs) = delete;

	virtual ~FizzleFX() = default;


	// Performs the effect.
	//
	// Returns true if aborted or false otherwise.
	bool present(
		const bool trigger_fade);


protected:
	// Initializes the effect.
	virtual void initialize() = 0;

	// Clean-ups the effect.
	virtual void uninitialize() = 0;

	// Returns true if the effect can be interrupted.
	virtual bool is_abortable() const = 0;

	// Returns true if the effect for vanilla renderer only.
	virtual bool is_vanilla_only() const = 0;

	// Returns total frame count.
	virtual int get_frame_count() const = 0;

	// Returns vertical offset of the effect beginning.
	virtual int get_y() const = 0;

	// Returns total height of the effect.
	virtual int get_height() const = 0;

	// Performs operation on one pixel.
	virtual void plot(
		const int x,
		const int y) = 0;

	// Presents the final stage of the effect.
	virtual void skip_to_the_end() = 0;
}; // FizzleFX


} // bstone


#endif // !BSTONE_FIZZLE_FX_INCLUDED
