/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_char_conv.h"

namespace bstone {
namespace detail {

const char* FromCharsErrorMessages::number_overflow = "Number overflow.";
const char* FromCharsErrorMessages::invalid_character = "Invalid character.";
const char* FromCharsErrorMessages::digit_out_of_range = "Digit out of base.";
const char* FromCharsErrorMessages::negative_unsigned = "Negative value for unsigned type";
const char* FromCharsErrorMessages::base_out_of_range = "Base out of range.";
const char* FromCharsErrorMessages::unexpected_end_of_chars = "Unexpected end of characters.";
const char* FromCharsErrorMessages::unable_to_detect_a_base = "Unable to detect a base.";

} // namespace detail
} // namespace bstone
