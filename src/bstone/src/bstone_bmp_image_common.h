/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

/*
BMP constants, etc.
*/

#ifndef BSTONE_BMP_IMAGE_COMMON_INCLUDED
#define BSTONE_BMP_IMAGE_COMMON_INCLUDED

namespace bstone {
namespace bmp {

// ==========================================================================
// Limits.

constexpr auto max_int32 = 0x7FFFFFFF;

// ==========================================================================
// V4/V5 Endpoints.

constexpr auto ciexyztriple_size = 36; // CIEXYZTRIPLE

// ==========================================================================
// Header sizes.

constexpr auto bitmapfileheader_size = 14; // BITMAPFILEHEADER
constexpr auto bitmapcoreheader_size = 12; // BITMAPCOREHEADER
constexpr auto bitmapinfoheader_size = 40; // BITMAPINFOHEADER
constexpr auto bitmapv4header_size = 108; // BITMAPV4HEADER
constexpr auto bitmapv5header_size = 124; // BITMAPV5HEADER

// ==========================================================================
// Bitmap types.

constexpr auto type_bm = 0x4D42U; // "BM".

// ==========================================================================
// Plane counts.

constexpr auto plane_count = 1;

// ==========================================================================
// Compression types.

constexpr auto bi_rgb = 0U;
constexpr auto bi_rle8 = 1U;
constexpr auto bi_rle4 = 2U;
constexpr auto bi_bitfields = 3U;

// ==========================================================================
// Color space types.

constexpr auto lcs_calibrated_rgb = 0U; // V4/V5
constexpr auto lcs_srgb = 0x73524742U; // V5 "sRGB".
constexpr auto lcs_windows_color_space = 0x57696E20U; // V5 "Win ".
constexpr auto profile_linked = 0x4C494E4BU; // V5 "LINK".
constexpr auto profile_embedded = 0x4D424544U; // V5 "MBED".

// ==========================================================================
// V5 Rendering intent types.

constexpr auto lcs_gm_business = 1U;
constexpr auto lcs_gm_graphics = 2U;
constexpr auto lcs_gm_images = 4U;
constexpr auto lcs_gm_abs_colorimetric = 8U;

// ==========================================================================
// Functions.

inline constexpr int calculate_stride(int width, int bit_depth) noexcept
{
	return (((width * bit_depth) + 31) / 32) * 4;
}

} // namespace bmp
} // namespace bstone

#endif // BSTONE_BMP_IMAGE_COMMON_INCLUDED
