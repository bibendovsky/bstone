/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Image extractor

#ifndef BSTONE_IMAGE_EXTRACTOR_INCLUDED
#define BSTONE_IMAGE_EXTRACTOR_INCLUDED

#include <memory>
#include <string>

namespace bstone {

class ImageExtractor
{
public:
	ImageExtractor() = default;
	virtual ~ImageExtractor() = default;

	virtual void extract_vga_palette(const std::string& destination_dir) = 0;
	virtual void extract_walls(const std::string& destination_dir) = 0;
	virtual void extract_sprites(const std::string& destination_dir) = 0;
};

// ======================================

using ImageExtractorUPtr = std::unique_ptr<ImageExtractor>;

ImageExtractorUPtr make_image_extractor();

} // namespace bstone

#endif // BSTONE_IMAGE_EXTRACTOR_INCLUDED
