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


#include "bstone_audio_content_mgr.h"

#include <vector>

#include "id_ca.h"
#include "id_pm.h"

#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_file_stream.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

AudioContentMgr::AudioContentMgr() noexcept = default;

AudioContentMgr::~AudioContentMgr() = default;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class AudioContentMgrException :
	public Exception
{
public:
	explicit AudioContentMgrException(
		const char* message)
		:
		Exception{std::string{} + "[AUDIO_CONTENT_MGR] " + message}
	{
	}
}; // AudioContentMgrException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class AudioContentMgrImpl :
	public AudioContentMgr
{
public:
	AudioContentMgrImpl();


	// ----------------------------------------------------------------------
	// AudioContentMgr

	AudioSfxType get_sfx_type() const noexcept override;

	void set_sfx_type(
		AudioSfxType sfx_type) override;


	void set_is_sfx_digitized(
		bool is_enabled) noexcept override;


	int get_chunk_count() const noexcept override;

	const AudioChunk& get_chunk(
		int chunk_number) const override;


	const AudioChunk& get_sfx_chunk(
		int sfx_chunk_number) const override;

	int get_sfx_priority(
		int sfx_chunk_number) const override;


	const AudioChunk& get_adlib_music_chunk(
		int adlib_music_chunk_number) const override;

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


	AudioSfxType sfx_type_{};
	int sfx_chunk_base_index_{};
	bool is_sfx_digitized_{};
	AudiotData audiot_data_{};
	AudioChunks audio_chunks_{};


	void initialize();

	AudiotData load_audiot_data();

	AudioChunks make_audio_chunks(
		const AudiotData& audiot_data);

	void make_digitized_sfx(
		AudioChunks& audio_chunks);
}; // AudioContentMgrImpl

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

AudioContentMgrImpl::AudioContentMgrImpl()
{
	initialize();
}

AudioSfxType AudioContentMgrImpl::get_sfx_type() const noexcept
{
	return sfx_type_;
}

void AudioContentMgrImpl::set_sfx_type(
	AudioSfxType sfx_type)
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
			throw AudioContentMgrException{"Unsupported SFX type."};
	}

	sfx_chunk_base_index_ = sfx_chunk_base_index;
}

void AudioContentMgrImpl::set_is_sfx_digitized(
	bool is_enabled) noexcept
{
	is_sfx_digitized_ = is_enabled;
}

int AudioContentMgrImpl::get_chunk_count() const noexcept
{
	return static_cast<int>(audio_chunks_.size());
}

const AudioChunk& AudioContentMgrImpl::get_chunk(
	int chunk_number) const
{
	if (chunk_number < 0 || chunk_number >= get_chunk_count())
	{
		throw AudioContentMgrException{"Chunk number out of range."};
	}

	return audio_chunks_[chunk_number];
}

const AudioChunk& AudioContentMgrImpl::get_sfx_chunk(
	int sfx_chunk_number) const
{
	if (sfx_chunk_number < 0 || sfx_chunk_number >= max_sfx_sounds)
	{
		throw AudioContentMgrException{"SFX chunk number out of range."};
	}

	if (is_sfx_digitized_)
	{
		const auto& digitized_sfx_chunk = audio_chunks_[digitized_sfx_chunk_base_index + sfx_chunk_number];

		if (digitized_sfx_chunk.data)
		{
			return digitized_sfx_chunk;
		}
	}

	return audio_chunks_[sfx_chunk_base_index_ + sfx_chunk_number];
}

int AudioContentMgrImpl::get_sfx_priority(
	int sfx_chunk_number) const
{
	if (sfx_chunk_number < 0 || sfx_chunk_number >= max_sfx_sounds)
	{
		throw AudioContentMgrException{"SFX chunk number out of range."};
	}

	const auto& audio_chunk = audio_chunks_[sfx_chunk_base_index_ + sfx_chunk_number];
	const auto data_u16 = reinterpret_cast<const std::uint16_t*>(audio_chunk.data);
	const auto priority = static_cast<int>(Endian::little(data_u16[2]));

	return priority;
}

const AudioChunk& AudioContentMgrImpl::get_adlib_music_chunk(
	int adlib_music_chunk_id) const
{
	const auto music_chunk_count = get_chunk_count() - adlib_music_chunk_base_index;

	if (adlib_music_chunk_id < 0 || adlib_music_chunk_id >= music_chunk_count)
	{
		throw AudioContentMgrException{"Music chunk number out of range."};
	}

	return audio_chunks_[adlib_music_chunk_base_index + adlib_music_chunk_id];
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

	const auto read_audiot_size = audiot_file.read(audiot_data.data(), audiot_size);

	if (read_audiot_size != audiot_size)
	{
		throw AudioContentMgrException{"Failed to read audio data file."};
	}

	return audiot_data;
}

AudioContentMgrImpl::AudioChunks AudioContentMgrImpl::make_audio_chunks(
	const AudiotData& audiot_data)
{
	constexpr auto audiohed_item_size = 4;

	auto audiohed_file = FileStream{};
	ca_open_resource(AssetsResourceType::audiohed, audiohed_file);
	const auto audiohed_size = static_cast<int>(audiohed_file.get_size());

	if ((audiohed_size % audiohed_item_size) != 0)
	{
		throw AudioContentMgrException{"Invalid audio TOC file size."};
	}

	const auto audiohed_count = audiohed_size / audiohed_item_size;
	const auto audio_chunk_count = audiohed_count - 1;

	if (audio_chunk_count <= adlib_music_chunk_base_index)
	{
		throw AudioContentMgrException{"Invalid audio chunk count."};
	}

	using Audiohed = std::vector<std::int32_t>;
	auto audiohed_data = Audiohed{};
	audiohed_data.resize(audiohed_count);

	const auto read_audiohed_size = audiohed_file.read(audiohed_data.data(), audiohed_size);

	if (read_audiohed_size != audiohed_size)
	{
		throw AudioContentMgrException{"Failed to read audio TOC file."};
	}

	if (Endian::is_big())
	{
		for (auto& audiohed_item : audiohed_data)
		{
			Endian::little_i(audiohed_item);
		}
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

void AudioContentMgrImpl::make_digitized_sfx(
	AudioChunks& audio_chunks)
{
	struct DigitizedMapItem
	{
		int sfx_index{};
		int digitized_info_index{};
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
		std::uint16_t page_number{};
		std::uint16_t data_size{};
	}; // DigitizedInfo

	static_assert(sizeof(DigitizedInfo) == 4, "Unsupported structure size.");


	const auto digitized_infos = static_cast<const DigitizedInfo*>(PM_GetPage(ChunksInFile - 1));

	for (const auto& digitized_map_item : digitized_map)
	{
		const auto& digitized_info = digitized_infos[digitized_map_item.digitized_info_index];
		const auto page_number = static_cast<int>(bstone::Endian::little(digitized_info.page_number));
		const auto data_size = static_cast<int>(bstone::Endian::little(digitized_info.data_size));
		const auto data = static_cast<const std::uint8_t*>(PM_GetSoundPage(page_number));
		const auto digitized_sfx_chunk_index = digitized_sfx_chunk_base_index + digitized_map_item.sfx_index;

		auto& audio_chunk = audio_chunks[digitized_sfx_chunk_index];
		audio_chunk.data = data;
		audio_chunk.data_size = data_size;
	}
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

AudioContentMgrUPtr make_audio_content_mgr()
{
	return std::make_unique<AudioContentMgrImpl>();
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
