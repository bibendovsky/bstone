/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
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
		PsFizzleFX&& rhs) noexcept;

	PsFizzleFX(
		const PsFizzleFX& rhs) = delete;

	PsFizzleFX& operator=(
		const PsFizzleFX& rhs) = delete;

	~PsFizzleFX() override;


	// Initializes the effect.
	void initialize(bool has_vanilla_appearence) override;


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

	bool has_vanilla_appearence() const override;


private:
	bool has_vanilla_appearence_{};
	VgaBuffer buffer_;
}; // PsFizzleFX


} // bstone


#endif // !BSTONE_PS_FIZZLE_FX_INCLUDED
