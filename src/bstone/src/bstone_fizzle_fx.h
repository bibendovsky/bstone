/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
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
	FizzleFX() noexcept;

	FizzleFX(
		const FizzleFX& rhs) = delete;

	FizzleFX(
		FizzleFX&& rhs);

	FizzleFX& operator=(
		const FizzleFX& rhs) = delete;

	virtual ~FizzleFX();


	// Performs the effect.
	//
	// Returns true if aborted or false otherwise.
	bool present();


protected:
	// Initializes the effect.
	virtual void initialize(bool has_vanilla_appearence) = 0;

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

	// Returns true if the effect has vanilla appearence.
	virtual bool has_vanilla_appearence() const = 0;


private:
	[[noreturn]]
	static void fail_not_implemented();
}; // FizzleFX


} // bstone


#endif // !BSTONE_FIZZLE_FX_INCLUDED
