/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_audio_content_mgr.h"
#include <cassert>
#include <vector>
#include "id_ca.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_file_stream.h"


namespace bstone
{

AudioContentMgr::AudioContentMgr() noexcept = default;

AudioContentMgr::~AudioContentMgr() = default;

// ==========================================================================

class AudioContentMgrImpl final : public AudioContentMgr
{
public:
	AudioContentMgrImpl(PageMgr& page_mgr);
	~AudioContentMgrImpl() override;

	// ----------------------------------------------------------------------
	// AudioContentMgr

	AudioSfxType get_sfx_type() const noexcept override;
	void set_sfx_type(AudioSfxType sfx_type) override;

	void set_is_sfx_digitized(bool is_digitized) noexcept override;

	int get_chunk_count() const noexcept override;
	const AudioChunk& get_chunk(int chunk_number) const override;
	const AudioChunk& get_sfx_chunk(int chunk_number) const override;
	int get_sfx_priority(int chunk_number) const override;

	const AudioChunk& get_adlib_music_chunk(int chunk_number) const override;

	// AudioContentMgr
	// ----------------------------------------------------------------------


private:
	static constexpr auto max_sfx_sounds = NUMSOUNDS;

	static constexpr auto pc_speaker_sfx_chunk_base_index = 0;
	static constexpr auto adlib_sfx_chunk_base_index = 100;
	static constexpr auto digitized_sfx_chunk_base_index = 200;
	static constexpr auto adlib_music_chunk_base_index = 300;

	using AudiotData = std::vector<std::uint8_t>;
	using AudioChunks = std::vector<AudioChunk>;

	PageMgr& page_mgr_;

	AudioSfxType sfx_type_{};
	int sfx_chunk_base_index_{};
	bool is_sfx_digitized_{};
	AudiotData audiot_data_{};
	AudioChunks audio_chunks_{};

	void initialize();

	AudiotData load_audiot_data();
	AudioChunks make_audio_chunks(const AudiotData& audiot_data);
	void make_digitized_sfx(AudioChunks& audio_chunks);
}; // AudioContentMgrImpl

// ----------------------------------------------------------------------

AudioContentMgrImpl::AudioContentMgrImpl(PageMgr& page_mgr)
	:
	page_mgr_{page_mgr}
{
	initialize();
}

AudioContentMgrImpl::~AudioContentMgrImpl() = default;

AudioSfxType AudioContentMgrImpl::get_sfx_type() const noexcept
{
	return sfx_type_;
}

void AudioContentMgrImpl::set_sfx_type(AudioSfxType sfx_type)
{
	auto sfx_chunk_base_index = 0;

	switch (sfx_type)
	{
		case AudioSfxType::adlib:
			sfx_chunk_base_index = adlib_sfx_chunk_base_index;
			break;

		case AudioSfxType::pc_speaker:
			sfx_chunk_base_index = pc_speaker_sfx_chunk_base_index;
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported SFX type.");
	}

	sfx_chunk_base_index_ = sfx_chunk_base_index;
}

void AudioContentMgrImpl::set_is_sfx_digitized(bool is_digitized) noexcept
{
	is_sfx_digitized_ = is_digitized;
}

int AudioContentMgrImpl::get_chunk_count() const noexcept
{
	return static_cast<int>(audio_chunks_.size());
}

const AudioChunk& AudioContentMgrImpl::get_chunk(int chunk_number) const
{
	if (chunk_number < 0 || chunk_number >= get_chunk_count())
	{
		BSTONE_THROW_STATIC_SOURCE("Chunk number out of range.");
	}

	return audio_chunks_[chunk_number];
}

const AudioChunk& AudioContentMgrImpl::get_sfx_chunk(int chunk_number) const
{
	if (chunk_number < 0 || chunk_number >= max_sfx_sounds)
	{
		BSTONE_THROW_STATIC_SOURCE("SFX chunk number out of range.");
	}

	if (is_sfx_digitized_)
	{
		const auto& digitized_sfx_chunk = audio_chunks_[digitized_sfx_chunk_base_index + chunk_number];

		if (digitized_sfx_chunk.data)
		{
			return digitized_sfx_chunk;
		}
	}

	return audio_chunks_[sfx_chunk_base_index_ + chunk_number];
}

int AudioContentMgrImpl::get_sfx_priority(int chunk_number) const
{
	if (chunk_number < 0 || chunk_number >= max_sfx_sounds)
	{
		BSTONE_THROW_STATIC_SOURCE("SFX chunk number out of range.");
	}

	const auto& audio_chunk = audio_chunks_[sfx_chunk_base_index_ + chunk_number];
	const auto data_u16 = reinterpret_cast<const std::uint16_t*>(audio_chunk.data);
	const auto priority = static_cast<int>(endian::to_little(data_u16[2]));
	return priority;
}

const AudioChunk& AudioContentMgrImpl::get_adlib_music_chunk(int chunk_number) const
{
	const auto music_chunk_count = get_chunk_count() - adlib_music_chunk_base_index;

	if (chunk_number < 0 || chunk_number >= music_chunk_count)
	{
		BSTONE_THROW_STATIC_SOURCE("Music chunk number out of range.");
	}

	return audio_chunks_[adlib_music_chunk_base_index + chunk_number];
}

void AudioContentMgrImpl::initialize()
{
	auto audiot_data = load_audiot_data();
	auto audio_chunks = make_audio_chunks(audiot_data);
	make_digitized_sfx(audio_chunks);
	audiot_data_.swap(audiot_data);
	audio_chunks_.swap(audio_chunks);
	set_sfx_type(AudioSfxType::adlib);
	is_sfx_digitized_ = true;
}

AudioContentMgrImpl::AudiotData AudioContentMgrImpl::load_audiot_data()
{
	auto audiot_file = FileStream{};
	ca_open_resource(AssetsResourceType::audiot, audiot_file);
	const auto audiot_size = static_cast<int>(audiot_file.get_size());
	auto audiot_data = AudiotData{};
	audiot_data.resize(audiot_size);
	audiot_file.read_exactly(audiot_data.data(), audiot_size);
	return audiot_data;
}

AudioContentMgrImpl::AudioChunks AudioContentMgrImpl::make_audio_chunks(const AudiotData& audiot_data)
{
	constexpr auto audiohed_item_size = 4;
	auto audiohed_file = FileStream{};
	ca_open_resource(AssetsResourceType::audiohed, audiohed_file);
	const auto audiohed_size = static_cast<int>(audiohed_file.get_size());

	if ((audiohed_size % audiohed_item_size) != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid TOC file size.");
	}

	const auto audiohed_count = audiohed_size / audiohed_item_size;
	const auto audio_chunk_count = audiohed_count - 1;

	if (audio_chunk_count <= adlib_music_chunk_base_index)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid audio chunk count.");
	}

	using Audiohed = std::vector<std::int32_t>;
	auto audiohed_data = Audiohed{};
	audiohed_data.resize(audiohed_count);
	audiohed_file.read_exactly(audiohed_data.data(), audiohed_size);

	for (auto& audiohed_item : audiohed_data)
	{
		audiohed_item = endian::to_little(audiohed_item);
	}

	auto audio_chunks = AudioChunks{};
	audio_chunks.resize(audio_chunk_count);

	auto chunk_offset = 0;

	for (auto i = 0; i < audio_chunk_count; ++i)
	{
		const auto data = &audiot_data[chunk_offset];
		const auto data_size = audiohed_data[i + 1] - audiohed_data[i];
		auto& audio_chunk = audio_chunks[i];
		audio_chunk.data = data;
		audio_chunk.data_size = data_size;
		audio_chunk.index = i;
		chunk_offset += data_size;
	}

	for (auto i = 0; i < max_sfx_sounds; ++i)
	{
		auto& audio_chunk = audio_chunks[pc_speaker_sfx_chunk_base_index + i];
		audio_chunk.type = AudioChunkType::pc_speaker;
		audio_chunk.audio_index = i;
	}

	for (auto i = 0; i < max_sfx_sounds; ++i)
	{
		auto& audio_chunk = audio_chunks[adlib_sfx_chunk_base_index + i];
		audio_chunk.type = AudioChunkType::adlib_sfx;
		audio_chunk.audio_index = i;
	}

	for (auto i = 0; i < max_sfx_sounds; ++i)
	{
		auto& audio_chunk = audio_chunks[digitized_sfx_chunk_base_index + i];
		audio_chunk.data = nullptr;
		audio_chunk.data_size = 0;
		audio_chunk.type = AudioChunkType::digitized;
		audio_chunk.audio_index = i;
	}

	const auto adlib_music_count = audio_chunk_count - adlib_music_chunk_base_index;

	for (auto i = 0; i < adlib_music_count; ++i)
	{
		auto& audio_chunk = audio_chunks[adlib_music_chunk_base_index + i];
		audio_chunk.type = AudioChunkType::adlib_music;
		audio_chunk.audio_index = i;
	}

	return audio_chunks;
}

void AudioContentMgrImpl::make_digitized_sfx(AudioChunks& audio_chunks)
{
	struct DigitizedMapItem
	{
		int sfx_index;
		int digitized_info_index;
	}; // DigitizedMapItem

	constexpr DigitizedMapItem digitized_map[] =
	{
		DigitizedMapItem{ATKIONCANNONSND, 0},
		DigitizedMapItem{ATKCHARGEDSND, 1},
		DigitizedMapItem{ATKBURSTRIFLESND, 2},
		DigitizedMapItem{ATKGRENADESND, 46},

		DigitizedMapItem{OPENDOORSND, 3},
		DigitizedMapItem{CLOSEDOORSND, 4},
		DigitizedMapItem{HTECHDOOROPENSND, 5},
		DigitizedMapItem{HTECHDOORCLOSESND, 6},

		DigitizedMapItem{INFORMANTDEATHSND, 7},
		DigitizedMapItem{SCIENTISTHALTSND, 19},
		DigitizedMapItem{SCIENTISTDEATHSND, 20},

		DigitizedMapItem{GOLDSTERNHALTSND, 8},
		DigitizedMapItem{GOLDSTERNLAUGHSND, 24},

		DigitizedMapItem{HALTSND, 9}, // Rent-A-Cop 1st sighting
		DigitizedMapItem{RENTDEATH1SND, 10}, // Rent-A-Cop Death

		DigitizedMapItem{EXPLODE1SND, 11},

		DigitizedMapItem{GGUARDHALTSND, 12},
		DigitizedMapItem{GGUARDDEATHSND, 17},

		DigitizedMapItem{PROHALTSND, 16},
		DigitizedMapItem{PROGUARDDEATHSND, 13},

		DigitizedMapItem{BLUEBOYDEATHSND, 18},
		DigitizedMapItem{BLUEBOYHALTSND, 51},

		DigitizedMapItem{SWATHALTSND, 22},
		DigitizedMapItem{SWATDIESND, 47},

		DigitizedMapItem{SCANHALTSND, 15},
		DigitizedMapItem{SCANDEATHSND, 23},

		DigitizedMapItem{PODHATCHSND, 26},
		DigitizedMapItem{PODHALTSND, 50},
		DigitizedMapItem{PODDEATHSND, 49},

		DigitizedMapItem{ELECTSHOTSND, 27},

		DigitizedMapItem{DOGBOYHALTSND, 14},
		DigitizedMapItem{DOGBOYDEATHSND, 21},
		DigitizedMapItem{ELECARCDAMAGESND, 25},
		DigitizedMapItem{ELECAPPEARSND, 28},
		DigitizedMapItem{ELECDIESND, 29},

		DigitizedMapItem{INFORMDEATH2SND, 39}, // Informant Death #2
		DigitizedMapItem{RENTDEATH2SND, 34}, // Rent-A-Cop Death #2
		DigitizedMapItem{PRODEATH2SND, 42}, // PRO Death #2
		DigitizedMapItem{SWATDEATH2SND, 48}, // SWAT Death #2
		DigitizedMapItem{SCIDEATH2SND, 53}, // Gen. Sci Death #2

		DigitizedMapItem{LIQUIDDIESND, 30},

		DigitizedMapItem{GURNEYSND, 31},
		DigitizedMapItem{GURNEYDEATHSND, 41},

		DigitizedMapItem{WARPINSND, 32},
		DigitizedMapItem{WARPOUTSND, 33},

		DigitizedMapItem{EXPLODE2SND, 35},

		DigitizedMapItem{LCANHALTSND, 36},
		DigitizedMapItem{LCANDEATHSND, 37},

		// RENTDEATH3SND, 38, // Rent-A-Cop Death #3
		DigitizedMapItem{INFORMDEATH3SND, 40}, // Informant Death #3
		DigitizedMapItem{PRODEATH3SND, 43}, // PRO Death #3
		DigitizedMapItem{SWATDEATH3SND, 52}, // Swat Guard #3
		DigitizedMapItem{SCIDEATH3SND, 54}, // Gen. Sci Death #3

		DigitizedMapItem{LCANBREAKSND, 44},
		DigitizedMapItem{SCANBREAKSND, 45},
		DigitizedMapItem{CLAWATTACKSND, 56},
		DigitizedMapItem{SPITATTACKSND, 55},
		DigitizedMapItem{PUNCHATTACKSND, 57},
	}; // digitized_map

	struct DigitizedInfo
	{
		std::uint16_t page_number;
		std::uint16_t data_size;
	}; // DigitizedInfo

	static_assert(sizeof(DigitizedInfo) == 4, "Unsupported structure size.");

	const auto digitized_infos = reinterpret_cast<const DigitizedInfo*>(page_mgr_.get_last_audio());

	for (const auto& digitized_map_item : digitized_map)
	{
		const auto& digitized_info = digitized_infos[digitized_map_item.digitized_info_index];
		const auto page_number = static_cast<int>(bstone::endian::to_little(digitized_info.page_number));
		const auto data_size = static_cast<int>(bstone::endian::to_little(digitized_info.data_size));
		const auto data = page_mgr_.get_audio(page_number);
		const auto digitized_sfx_chunk_index = digitized_sfx_chunk_base_index + digitized_map_item.sfx_index;

		auto& audio_chunk = audio_chunks[digitized_sfx_chunk_index];
		audio_chunk.data = data;
		audio_chunk.data_size = data_size;
	}
}

// ==========================================================================

AudioContentMgrUPtr make_audio_content_mgr(PageMgr& page_mgr)
{
	return std::make_unique<AudioContentMgrImpl>(page_mgr);
}

} // bstone
