/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <algorithm>
#include "gfxv.h"
#include "id_ca.h"
#include "jm_lzh.h"
#include "bstone_binary_reader.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_fs_utils.h"
#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_memory_stream.h"
#include "bstone_static_ro_memory_stream.h"
#include "bstone_text_extractor.h"

namespace bstone {

TextExtractor::TextExtractor()
	:
	text_numbers_{}
{
	initialize_text();
}

void TextExtractor::extract_text(const std::string& dst_dir)
{
	globals::logger->log_information(("File count: " + std::to_string(text_numbers_.size())).c_str());

	for (const auto& text_number : text_numbers_)
	{
		extract_text(dst_dir, text_number);
	}
}

[[noreturn]] void TextExtractor::fail(int number, const char* message)
{
	const auto error_message = std::string{} + "[Text #" + std::to_string(number) + "] " + message;
	BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
}

void TextExtractor::initialize_text()
{
	const auto& assets_info = get_assets_info();
	const auto is_compressed = assets_info.is_aog_sw_v2_x() || assets_info.is_ps();

	text_numbers_.reserve(50);

	text_numbers_.emplace_back(TextNumber{false, INFORMANT_HINTS});
	text_numbers_.emplace_back(TextNumber{false, NICE_SCIE_HINTS});
	text_numbers_.emplace_back(TextNumber{false, MEAN_SCIE_HINTS});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W1});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I1});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W2});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I2});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W3});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I3});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W4});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I4});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W5});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I5});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_W6});
	text_numbers_.emplace_back(TextNumber{false, BRIEF_I6});
	text_numbers_.emplace_back(TextNumber{false, LEVEL_DESCS});
	text_numbers_.emplace_back(TextNumber{is_compressed, POWERBALLTEXT});
	text_numbers_.emplace_back(TextNumber{is_compressed, TICSTEXT});
	text_numbers_.emplace_back(TextNumber{is_compressed, MUSICTEXT});
	text_numbers_.emplace_back(TextNumber{is_compressed, RADARTEXT});
	text_numbers_.emplace_back(TextNumber{false, HELPTEXT});
	text_numbers_.emplace_back(TextNumber{false, SAGATEXT});
	text_numbers_.emplace_back(TextNumber{false, LOSETEXT});
	text_numbers_.emplace_back(TextNumber{false, ORDERTEXT});
	text_numbers_.emplace_back(TextNumber{false, CREDITSTEXT});
	text_numbers_.emplace_back(TextNumber{false, MUSTBE386TEXT});
	text_numbers_.emplace_back(TextNumber{false, QUICK_INFO1_TEXT});
	text_numbers_.emplace_back(TextNumber{false, QUICK_INFO2_TEXT});
	text_numbers_.emplace_back(TextNumber{false, BADINFO_TEXT});
	text_numbers_.emplace_back(TextNumber{false, CALJOY1_TEXT});
	text_numbers_.emplace_back(TextNumber{false, CALJOY2_TEXT});
	text_numbers_.emplace_back(TextNumber{false, READTHIS_TEXT});
	text_numbers_.emplace_back(TextNumber{false, ELEVMSG0_TEXT});
	text_numbers_.emplace_back(TextNumber{false, ELEVMSG1_TEXT});
	text_numbers_.emplace_back(TextNumber{false, ELEVMSG4_TEXT});
	text_numbers_.emplace_back(TextNumber{false, ELEVMSG5_TEXT});
	text_numbers_.emplace_back(TextNumber{false, FLOORMSG_TEXT});
	text_numbers_.emplace_back(TextNumber{false, YOUWIN_TEXT});
	text_numbers_.emplace_back(TextNumber{false, CHANGEVIEW_TEXT});
	text_numbers_.emplace_back(TextNumber{false, BADCHECKSUMTEXT});
	text_numbers_.emplace_back(TextNumber{false, DIZ_ERR_TEXT});
	text_numbers_.emplace_back(TextNumber{false, BADLEVELSTEXT});
	text_numbers_.emplace_back(TextNumber{false, BADSAVEGAME_TEXT});

	std::sort(
		text_numbers_.begin(),
		text_numbers_.end(),
		[](const auto& lhs, const auto& rhs)
		{
			return lhs.number_ < rhs.number_;
		});

	const auto non_zero_number_it = std::find_if(
		text_numbers_.begin(),
		text_numbers_.end(),
		[](const auto item)
		{
			return item.number_ != 0;
		});

	if (non_zero_number_it == text_numbers_.end())
	{
		BSTONE_THROW_STATIC_SOURCE("Empty list.");
	}

	text_numbers_.erase(text_numbers_.begin(), non_zero_number_it);
}

CompHeader_t TextExtractor::deserialize_header(int number, const std::uint8_t* data)
{
	auto stream = StaticRoMemoryStream{data, CompHeader_t::class_size};
	auto reader = BinaryReader{stream};
	auto result = CompHeader_t{};

	reader.get_stream().read_exactly(result.NameId, 4);
	result.OriginalLen = endian::to_little(reader.read_u32());
	result.CompType = static_cast<ct_TYPES>(endian::to_little(reader.read_u16()));
	result.CompressLen = endian::to_little(reader.read_u32());

	const auto four_cc = std::string{result.NameId, 4};

	if (four_cc != JAMP)
	{
		fail(number, "Unsupported FOURCC.");
	}

	return result;
}

void TextExtractor::extract_text(const std::string& dst_dir, const TextNumber& text_number)
{
	const auto number = text_number.number_;

	CA_CacheGrChunk(static_cast<std::int16_t>(number));

	auto text_data = grsegs[number].data();
	auto text_size = grsegs_sizes_[number];

	if (text_number.is_compressed_)
	{
		constexpr auto header_size = CompHeader_t::class_size;

		if (text_size < header_size)
		{
			fail(number, "Header too small.");
		}

		constexpr auto max_uncompressed_size = 4'096;

		const auto compressed_header = deserialize_header(number, text_data);
		const auto pure_data_size = text_size - header_size;

		if (compressed_header.CompressLen > static_cast<std::uint32_t>(pure_data_size) ||
			compressed_header.OriginalLen > max_uncompressed_size)
		{
			fail(number, "Length(s) out of range.");
		}

		if (compressed_header.CompType != ct_LZH)
		{
			fail(number, "Expected LZH compression type.");
		}

		buffer_.resize(compressed_header.OriginalLen);

		const auto decoded_size = LZH_Decompress(
			text_data + header_size,
			buffer_.data(),
			compressed_header.OriginalLen,
			compressed_header.CompressLen);

		buffer_.resize(decoded_size);

		text_data = buffer_.data();
		text_size = decoded_size;
	}
	else
	{
		text_data = grsegs[number].data();
	}

	const auto& number_string = ca_make_padded_asset_number_string(number);

	const auto& file_name = fs_utils::append_path(
		dst_dir,
		"text_" + number_string + ".txt"
	);

	auto file_stream = FileStream{
		file_name.c_str(),
		FileOpenFlags::create | FileOpenFlags::truncate | FileOpenFlags::write,
		FileShareMode::exclusive};

	file_stream.write_exactly(text_data, text_size);
}

} // namespace bstone
