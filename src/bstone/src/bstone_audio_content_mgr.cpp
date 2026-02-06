/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_audio_content_mgr.h"
#include "id_ca.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_file_stream.h"
#include <cstdint>
#include <vector>

namespace bstone {

namespace {

class AudioContentMgrImpl final : public AudioContentMgr
{
public:
	AudioContentMgrImpl(Vswap& vswap);
	~AudioContentMgrImpl() override = default;

	AudioSfxType get_sfx_type() const override;
	void set_sfx_type(AudioSfxType sfx_type) override;
	void set_is_sfx_digitized(bool is_digitized) override;
	int get_chunk_count() const override;
	const AudioChunk& get_chunk(int chunk_number) const override;
	const AudioChunk& get_sfx_chunk(int chunk_number) const override;
	int get_sfx_priority(int chunk_number) const override;
	const AudioChunk& get_adlib_music_chunk(int chunk_number) const override;

private:
	static constexpr int max_sfx_sounds = NUMSOUNDS;

	static constexpr int pc_speaker_sfx_chunk_base_index = 0;
	static constexpr int adlib_sfx_chunk_base_index = 100;
	static constexpr int digitized_sfx_chunk_base_index = 200;
	static constexpr int adlib_music_chunk_base_index = 300;

	using AudiotData = std::vector<std::uint8_t>;
	using AudioChunks = std::vector<AudioChunk>;

	Vswap& vswap_;

	AudioSfxType sfx_type_{};
	int sfx_chunk_base_index_{};
	bool is_sfx_digitized_{};
	AudiotData audiot_data_{};
	AudioChunks audio_chunks_{};

	AudiotData load_audiot_data();
	AudioChunks make_audio_chunks(const AudiotData& audiot_data);
	void make_digitized_sfx(AudioChunks& audio_chunks);
};

// -------------------------------------

AudioContentMgrImpl::AudioContentMgrImpl(Vswap& vswap)
	:
	vswap_{vswap}
{
	audiot_data_ = load_audiot_data();
	audio_chunks_ = make_audio_chunks(audiot_data_);
	make_digitized_sfx(audio_chunks_);
	set_sfx_type(AudioSfxType::adlib);
	is_sfx_digitized_ = true;
}

AudioSfxType AudioContentMgrImpl::get_sfx_type() const
{
	return sfx_type_;
}

void AudioContentMgrImpl::set_sfx_type(AudioSfxType sfx_type)
{
	int sfx_chunk_base_index;
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

void AudioContentMgrImpl::set_is_sfx_digitized(bool is_digitized)
{
	is_sfx_digitized_ = is_digitized;
}

int AudioContentMgrImpl::get_chunk_count() const
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
		const AudioChunk& digitized_sfx_chunk = audio_chunks_[digitized_sfx_chunk_base_index + chunk_number];
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
	const AudioChunk& audio_chunk = audio_chunks_[sfx_chunk_base_index_ + chunk_number];
	if (audio_chunk.data_size < 4)
	{
		BSTONE_THROW_STATIC_SOURCE("SFX chunk header too small.");
	}
	return endian::read_u16_le(audio_chunk.data + 2);
}

const AudioChunk& AudioContentMgrImpl::get_adlib_music_chunk(int chunk_number) const
{
	const int music_chunk_count = get_chunk_count() - adlib_music_chunk_base_index;
	if (chunk_number < 0 || chunk_number >= music_chunk_count)
	{
		BSTONE_THROW_STATIC_SOURCE("Music chunk number out of range.");
	}
	return audio_chunks_[adlib_music_chunk_base_index + chunk_number];
}

AudioContentMgrImpl::AudiotData AudioContentMgrImpl::load_audiot_data()
{
	FileStream audiot_file{};
	ca_open_resource(AssetsResourceType::audiot, audiot_file);
	const int audiot_size = static_cast<int>(audiot_file.get_size());
	AudiotData audiot_data{};
	audiot_data.resize(audiot_size);
	audiot_file.read_exactly(audiot_data.data(), audiot_size);
	return audiot_data;
}

AudioContentMgrImpl::AudioChunks AudioContentMgrImpl::make_audio_chunks(const AudiotData& audiot_data)
{
	constexpr int audiohed_item_size = 4;
	FileStream audiohed_file{};
	ca_open_resource(AssetsResourceType::audiohed, audiohed_file);
	const int audiohed_size = static_cast<int>(audiohed_file.get_size());
	if ((audiohed_size % audiohed_item_size) != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid TOC file size.");
	}
	const int audiohed_count = audiohed_size / audiohed_item_size;
	const int audio_chunk_count = audiohed_count - 1;
	if (audio_chunk_count <= adlib_music_chunk_base_index)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid audio chunk count.");
	}
	using Audiohed = std::vector<std::uint8_t>;
	Audiohed audiohed_data{};
	audiohed_data.resize(audiohed_size);
	audiohed_file.read_exactly(audiohed_data.data(), audiohed_size);
	AudioChunks audio_chunks{};
	audio_chunks.resize(audio_chunk_count);
	int chunk_offset = 0;
	for (int i = 0; i < audio_chunk_count; ++i)
	{
		const std::uint8_t* const data = &audiot_data[chunk_offset];
		const std::uint8_t* const offsets_bytes = audiohed_data.data() + i * 4;
		const int data_offset_0 = endian::read_s32_le(offsets_bytes);
		const int data_offset_1 = endian::read_s32_le(offsets_bytes + 4);
		const int data_size = data_offset_1 - data_offset_0;
		AudioChunk& audio_chunk = audio_chunks[i];
		audio_chunk.data = data;
		audio_chunk.data_size = data_size;
		audio_chunk.index = i;
		chunk_offset += data_size;
	}
	for (int i = 0; i < max_sfx_sounds; ++i)
	{
		AudioChunk& audio_chunk = audio_chunks[pc_speaker_sfx_chunk_base_index + i];
		audio_chunk.type = AudioChunkType::pc_speaker;
		audio_chunk.audio_index = i;
	}
	for (int i = 0; i < max_sfx_sounds; ++i)
	{
		AudioChunk& audio_chunk = audio_chunks[adlib_sfx_chunk_base_index + i];
		audio_chunk.type = AudioChunkType::adlib_sfx;
		audio_chunk.audio_index = i;
	}
	for (int i = 0; i < max_sfx_sounds; ++i)
	{
		AudioChunk& audio_chunk = audio_chunks[digitized_sfx_chunk_base_index + i];
		audio_chunk.data = nullptr;
		audio_chunk.data_size = 0;
		audio_chunk.type = AudioChunkType::digitized;
		audio_chunk.audio_index = i;
	}
	const int adlib_music_count = audio_chunk_count - adlib_music_chunk_base_index;
	for (int i = 0; i < adlib_music_count; ++i)
	{
		AudioChunk& audio_chunk = audio_chunks[adlib_music_chunk_base_index + i];
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
	};

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
	};
	const std::uint8_t* const digitized_infos_bytes = vswap_.get_audio_data(vswap_.get_audio_count() - 1);
	for (const DigitizedMapItem& digitized_map_item : digitized_map)
	{
		const int digitized_info_offset = digitized_map_item.digitized_info_index * (2 + 2);
		const int page_number = endian::read_u16_le(digitized_infos_bytes + digitized_info_offset);
		const int data_size = endian::read_u16_le(digitized_infos_bytes + digitized_info_offset + 2);
		const std::uint8_t* const data = vswap_.get_audio_data(page_number);
		const int digitized_sfx_chunk_index = digitized_sfx_chunk_base_index + digitized_map_item.sfx_index;
		AudioChunk& audio_chunk = audio_chunks[digitized_sfx_chunk_index];
		audio_chunk.data = data;
		audio_chunk.data_size = data_size;
	}
}

} // namespace

// =====================================

AudioContentMgrUPtr make_audio_content_mgr(Vswap& vswap)
{
	return std::make_unique<AudioContentMgrImpl>(vswap);
}

} // namespace bstone
