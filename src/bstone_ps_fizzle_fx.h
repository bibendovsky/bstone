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


#ifndef BSTONE_PS_FIZZLE_FX_INCLUDED
#define BSTONE_PS_FIZZLE_FX_INCLUDED


//
// Planet Strike fizzle effect.
//


#include "bstone_fizzle_fx.h"
#include "id_vl.h"


namespace bstone
{


class PsFizzleFX final :
	public FizzleFX
{
public:
	PsFizzleFX();

	PsFizzleFX(
		PsFizzleFX&& rhs);

	PsFizzleFX(
		const PsFizzleFX& rhs) = delete;

	PsFizzleFX& operator=(
		const PsFizzleFX& rhs) = delete;

	~PsFizzleFX() override;


	// Initializes the effect.
	void initialize() override;


protected:
	// Clean-ups the effect.
	void uninitialize() override;

	// Returns true if the effect can be interrupted.
	bool is_abortable() const override;

	// Returns true if the effect for vanilla renderer only.
	bool is_vanilla_only() const override;

	// Returns total frame count.
	int get_frame_count() const override;

	// Returns vertical offset of the effect beginning.
	int get_y() const override;

	// Returns total height of the effect.
	int get_height() const override;

	// Performs operation on one pixel.
	void plot(
		const int x,
		const int y) override;

	// Presents the final stage of the effect.
	void skip_to_the_end() override;


private:
	VgaBuffer buffer_;
}; // PsFizzleFX


} // bstone


#endif // !BSTONE_PS_FIZZLE_FX_INCLUDED
