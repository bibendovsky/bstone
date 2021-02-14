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
// SHA-1 implementation based on RFC 3174 sample code
// http://www.ietf.org/rfc/rfc3174.txt
//


#ifndef BSTONE_SHA1_INCLUDED
#define BSTONE_SHA1_INCLUDED


#include <cstdint>

#include <array>
#include <string>


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Sha1
//

class Sha1
{
public:
	static const int hash_size = 20;

	using Digest = std::array<std::uint8_t, hash_size>;


	Sha1();

	Sha1(
		const char (&sha1_string)[(hash_size * 2) + 1]);

	Sha1(
		const std::string& sha1_string);

	Sha1(
		const Digest& digest);

	Sha1(
		const Sha1& rhs);

	Sha1(
		Sha1&& rhs);

	Sha1& operator=(
		const Sha1& rhs);


	void reset();

	void process(
		const void* data,
		const int data_size);

	void finish();

	bool is_finished() const;

	bool is_valid() const;

	const Digest& get_digest() const;

	std::string to_string() const;


private:
	using Block = std::array<std::uint8_t, 64>;
	using Digest32 = std::array<std::uint32_t, hash_size / 4>;


	static const Digest32 initial_digest_32;


	// 512-bit message block.
	Block block_;

	// Message digest.
	Digest digest_;

	// Message digest as words.
	Digest32 digest32_;

	// Message length in bits.
	std::uint32_t length_low_;

	// Message length in bits.
	std::uint32_t length_high_;

	// Index into message block array.
	int block_index_;

	// Is the digest computed?
	bool is_finished_;

	// Is the digest invalid?
	bool is_invalid_;


	void ctor(
		const char* const sha1_string,
		const int sha1_string_length);

	void pad_message();

	void process_block();
}; // Sha1

//
// Sha1
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Sha1 operators
//

bool operator==(
	const bstone::Sha1& lhs,
	const bstone::Sha1& rhs);

bool operator!=(
	const bstone::Sha1& lhs,
	const bstone::Sha1& rhs);

bool operator==(
	const bstone::Sha1& lhs,
	const std::string& rhs);

bool operator!=(
	const bstone::Sha1& lhs,
	const std::string& rhs);

//
// Sha1 operators
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


#endif // BSTONE_SHA1_INCLUDED
