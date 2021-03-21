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


#include "bstone_oal_audio_mixer.h"

#include <cassert>

#include <array>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "3d_def.h"
#include "audio.h"
#include "id_sd.h"

#include "bstone_audio_decoder.h"
#include "bstone_exception.h"
#include "bstone_logger.h"
#include "bstone_oal_source.h"
#include "bstone_oal_loader.h"
#include "bstone_oal_resource.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalAudioMixerException :
	public Exception
{
public:
	explicit OalAudioMixerException(
		const char* message)
		:
		Exception{std::string{"[OAL_AUDIO_MIXER] "} + message}
	{
	}
}; // OalAudioMixerException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalAudioMixer::OalAudioMixer(
	MtTaskMgr* mt_task_manager)
	:
	mt_task_manager_{mt_task_manager}
{
}

OalAudioMixer::~OalAudioMixer()
{
	uninitialize();
}

bool OalAudioMixer::initialize(
	const AudioMixerInitParam& param)
try
{
	uninitialize();

	switch (param.opl3_type_)
	{
		case Opl3Type::dbopl:
			break;

		default:
			return false;
	}

	initialize_oal(param);

	opl3_type_ = param.opl3_type_;

	if (param.mix_size_ms_ < get_min_mix_size_ms())
	{
		mix_size_ms_ = get_min_mix_size_ms();
	}
	else if (param.mix_size_ms_ < get_default_mix_size_ms())
	{
		mix_size_ms_ = get_default_mix_size_ms();
	}
	else
	{
		mix_size_ms_ = param.mix_size_ms_;
	}

	mix_sample_count_ = static_cast<int>((dst_rate_ * mix_size_ms_) / 1'000L);

	initialize_music();
	initialize_sfx();
	initialize_thread();

	interpolation_ = AudioDecoderInterpolationType::none;
	is_lpf_ = false;
	is_mute_ = false;

	is_initialized_ = true;

	return true;
}
catch (...)
{
	is_initialized_ = false;

	return false;
}

void OalAudioMixer::uninitialize()
{
	is_initialized_ = false;

	{
		const auto thread_mutex_guard = MutexUniqueLock{thread_mutex_};
		is_quit_thread_ = true;
	}

	if (thread_.joinable())
	{
		thread_.join();
	}

	uninitialize_music();
	uninitialize_sfx();

	oal_context_resource_ = nullptr;
	oal_device_resource_ = nullptr;
	oal_loader_ = nullptr;
}

bool OalAudioMixer::is_initialized() const
{
	return is_initialized_;
}

Opl3Type OalAudioMixer::get_opl3_type() const
{
	return opl3_type_;
}

int OalAudioMixer::get_rate() const
{
	return dst_rate_;
}

int OalAudioMixer::get_channel_count() const
{
	return get_max_channels();
}

int OalAudioMixer::get_mix_size_ms() const
{
	return mix_size_ms_;
}

float OalAudioMixer::get_sfx_volume() const
{
	return sfx_volume_;
}

float OalAudioMixer::get_music_volume() const
{
	return music_volume_;
}

AudioDecoderInterpolationType OalAudioMixer::get_resampling_interpolation() const noexcept
{
	return interpolation_;
}

bool OalAudioMixer::get_resampling_lpf() const noexcept
{
	return is_lpf_;
}

bool OalAudioMixer::play_adlib_music(
	int music_index,
	const void* data,
	int data_size)
try
{
	if (!data || data_size <= 0)
	{
		return false;
	}

	const auto music_mutex_guard = MutexUniqueLock{music_mutex_};

	music_source_.close();

	auto audio_decoder_param = AudioDecoderInitParam{};
	audio_decoder_param.src_raw_data_ = data;
	audio_decoder_param.src_raw_size_ = data_size;
	audio_decoder_param.dst_rate_ = dst_rate_;
	audio_decoder_param.resampler_interpolation_ = AudioDecoderInterpolationType::zoh;
	audio_decoder_param.resampler_lpf_ = false;

	if (!music_adlib_sound_.audio_decoder->initialize(audio_decoder_param))
	{
		return false;
	}

	auto source_param = OalSourceOpenStreamingParam{};
	source_param.is_2d = true;
	source_param.is_looping = true;
	source_param.sample_rate = dst_rate_;
	source_param.uncaching_sound = &music_adlib_sound_;

	music_source_.open(source_param);
	music_source_.set_volume(music_volume_);
	music_source_.play();

	return true;
}
catch (...)
{
	return false;
}

bool OalAudioMixer::play_adlib_sound(
	int sound_index,
	int priority,
	const void* data,
	int data_size,
	int actor_index,
	ActorType actor_type,
	ActorChannel actor_channel)
try
{
	return play_sfx_sound(
		AudioSfxType::adlib,
		sound_index,
		priority,
		data,
		data_size,
		actor_index,
		actor_type,
		actor_channel
	);
}
catch (...)
{
	return false;
}

bool OalAudioMixer::play_pc_speaker_sound(
	int sound_index,
	int priority,
	const void* data,
	int data_size,
	int actor_index,
	ActorType actor_type,
	ActorChannel actor_channel)
try
{
	return play_sfx_sound(
		AudioSfxType::pc_speaker,
		sound_index,
		priority,
		data,
		data_size,
		actor_index,
		actor_type,
		actor_channel
	);
}
catch (...)
{
	return false;
}

bool OalAudioMixer::play_pcm_sound(
	int sound_index,
	int priority,
	const void* data,
	int data_size,
	int actor_index,
	ActorType actor_type,
	ActorChannel actor_channel)
try
{
	if (!is_initialized())
	{
		return false;
	}

	if (sound_index < 0 || sound_index >= NUMSOUNDS)
	{
		return false;
	}

	if (!data || data_size <= 0)
	{
		return false;
	}

	const auto sfx_mutex_guard = MutexUniqueLock{sfx_mutex_};

	const auto sfx_voice = find_free_sfx_voice(
		priority,
		actor_type,
		actor_index,
		actor_channel
	);

	if (!sfx_voice)
	{
		return false;
	}

	auto source_param = OalSourceOpenStaticParam{};
	source_param.is_2d = is_2d_sfx(actor_type, actor_index);
	source_param.is_8_bit = true;
	source_param.sample_rate = audio_decoder_pcm_fixed_frequency;
	source_param.data = data;
	source_param.data_size = data_size;

	auto& oal_source = sfx_voice->oal_source;
	oal_source.open(source_param);
	oal_source.set_volume(sfx_volume_);
	set_sfx_position(oal_source, actor_type, actor_index);
	set_sfx_reference_distance(oal_source);
	oal_source.play();

	sfx_voice->is_active = true;
	sfx_voice->priority = priority;
	sfx_voice->actor_type = actor_type;
	sfx_voice->actor_index = actor_index;
	sfx_voice->actor_channel = actor_channel;

	return true;
}
catch (...)
{
	return false;
}

bool OalAudioMixer::set_resampling(
	bstone::AudioDecoderInterpolationType interpolation,
	bool low_pass_filter)
{
	if (!is_initialized())
	{
		return false;
	}

	auto new_interpolation = interpolation;

	switch (interpolation)
	{
		case bstone::AudioDecoderInterpolationType::zoh:
		case bstone::AudioDecoderInterpolationType::linear:
			break;

		default:
			new_interpolation = bstone::AudioDecoderInterpolationType::linear;
			break;
	}

	if (new_interpolation == interpolation_ &&
		low_pass_filter == is_lpf_)
	{
		return true;
	}

	return true;
}

bool OalAudioMixer::update_positions()
try
{
	if (!is_initialized())
	{
		return false;
	}

	update_positions_player();
	update_positions_actors();
	update_positions_doors();
	update_positions_pushwall();

	apply_positions_player();

	if (!sfx_actor_modified_position_indices_.empty() ||
		!sfx_door_modified_position_indices_.empty() ||
		!is_sfx_pushwall_position_modified_)
	{
		const auto sfx_mutex_guard = MutexUniqueLock{sfx_mutex_};

		apply_positions();
	}

	return true;
}
catch (...)
{
	return false;
}

bool OalAudioMixer::stop_music()
try
{
	if (!is_initialized())
	{
		return false;
	}

	const auto music_mutex_guard = MutexUniqueLock{music_mutex_};

	music_source_.close();

	return true;
}
catch (...)
{
	return false;
}

bool OalAudioMixer::stop_pausable_sfx()
try
{
	if (!is_initialized())
	{
		return false;
	}

	const auto sfx_mutex_guard = MutexUniqueLock{sfx_mutex_};

	for (auto& sfx_voice : sfx_voices_)
	{
		if (!sfx_voice.is_active || sfx_voice.actor_channel != ActorChannel::unpausable)
		{
			continue;
		}

		sfx_voice.oal_source.stop();
	}

	return true;
}
catch (...)
{
	return false;
}

bool OalAudioMixer::pause_all_sfx(
	bool is_paused)
try
{
	if (!is_initialized())
	{
		return false;
	}

	const auto sfx_mutex_guard = MutexUniqueLock{sfx_mutex_};

	for (auto& sfx_voice : sfx_voices_)
	{
		if (!sfx_voice.is_active)
		{
			continue;
		}

		auto& oal_source = sfx_voice.oal_source;

		if (is_paused)
		{
			oal_source.pause();
		}
		else
		{
			oal_source.resume();
		}
	}

	return true;
}
catch (...)
{
	return false;
}

bool OalAudioMixer::pause_music(
	bool is_paused)
try
{
	if (!is_initialized())
	{
		return false;
	}

	const auto music_mutex_guard = MutexUniqueLock{music_mutex_};

	if (!music_source_.is_open())
	{
		return false;
	}

	if (is_paused)
	{
		music_source_.pause();
	}
	else
	{
		music_source_.resume();
	}

	return true;
}
catch (...)
{
	return false;
}

bool OalAudioMixer::set_mute(
	bool is_mute)
try
{
	if (!is_initialized())
	{
		return false;
	}

	if (is_mute_ == is_mute)
	{
		return true;
	}

	is_mute_ = is_mute;

	const auto al_gain = (is_mute_ ? 0.0F : 1.0F);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alListenerf(AL_GAIN, al_gain);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);

	return true;
}
catch (...)
{
	return false;
}

bool OalAudioMixer::set_sfx_volume(
	float volume)
try
{
	if (!is_initialized())
	{
		return false;
	}

	const auto sfx_mutex_guard = MutexUniqueLock{sfx_mutex_};

	if (sfx_volume_ == volume)
	{
		return true;
	}

	sfx_volume_ = volume;

	for (auto& sfx_voice : sfx_voices_)
	{
		if (!sfx_voice.is_active)
		{
			continue;
		}

		auto& oal_source = sfx_voice.oal_source;

		if (!oal_source.is_open())
		{
			continue;
		}

		oal_source.set_volume(sfx_volume_);
	}

	return true;
}
catch (...)
{
	return false;
}

bool OalAudioMixer::set_music_volume(
	float volume)
try
{
	if (!is_initialized())
	{
		return false;
	}

	const auto music_mutex_guard = MutexUniqueLock{music_mutex_};

	if (music_volume_ == volume)
	{
		return true;
	}

	music_volume_ = volume;

	if (music_source_.is_open())
	{
		music_source_.set_volume(music_volume_);
	}

	return true;
}
catch (...)
{
	return false;
}

bool OalAudioMixer::is_music_playing() const
try
{
	if (!is_initialized())
	{
		return false;
	}

	const auto music_mutex_guard = MutexUniqueLock{music_mutex_};

	if (music_source_.is_open() && music_source_.is_playing())
	{
		return true;
	}
	else
	{
		return false;
	}
}
catch (...)
{
	return false;
}

bool OalAudioMixer::is_any_unpausable_sfx_playing() const
try
{
	if (!is_initialized())
	{
		return false;
	}

	const auto sfx_mutex_guard = MutexUniqueLock{sfx_mutex_};

	for (auto& sfx_voice : sfx_voices_)
	{
		if (sfx_voice.is_active &&
			sfx_voice.actor_channel == ActorChannel::unpausable)
		{
			return true;
		}
	}

	return false;
}
catch (...)
{
	return false;
}

bool OalAudioMixer::is_player_channel_playing(
	ActorChannel channel) const
try
{
	if (!is_initialized())
	{
		return false;
	}

	const auto sfx_mutex_guard = MutexUniqueLock{sfx_mutex_};

	for (auto& sfx_voice : sfx_voices_)
	{
		if (sfx_voice.is_active &&
			sfx_voice.actor_index == 0 &&
			sfx_voice.actor_type == ActorType::actor &&
			sfx_voice.actor_channel == channel)
		{
			return true;
		}
	}

	return false;
}
catch (...)
{
	return false;
}

int OalAudioMixer::get_min_rate() const
{
	return 11'025;
}

int OalAudioMixer::get_default_rate() const
{
	return 44'100;
}

int OalAudioMixer::get_min_mix_size_ms() const
{
	return min_mix_size_ms;
}

int OalAudioMixer::get_default_mix_size_ms() const
{
	return max_mix_size_ms;
}

int OalAudioMixer::get_max_channels() const
{
	return 2;
}

int OalAudioMixer::get_max_commands() const
{
	return 0;
}

bool OalAudioMixer::play_sfx_sound(
	AudioSfxType sfx_type,
	int sound_index,
	int priority,
	const void* data,
	int data_size,
	int actor_index,
	ActorType actor_type,
	ActorChannel actor_channel)
{
	if (!is_initialized())
	{
		return false;
	}

	if (sound_index < 0 || sound_index >= NUMSOUNDS)
	{
		return false;
	}

	if (!data || data_size <= 0)
	{
		return false;
	}

	const auto sfx_mutex_guard = MutexUniqueLock{sfx_mutex_};

	auto& sfx_sound = (
		sfx_type == AudioSfxType::adlib ?
		sfx_adlib_sounds_[sound_index] :
		sfx_pc_speaker_sounds_[sound_index]
	);

	if (!sfx_sound.is_initialized)
	{
		auto audio_decoder_param = AudioDecoderInitParam{};
		audio_decoder_param.src_raw_data_ = data;
		audio_decoder_param.src_raw_size_ = data_size;
		audio_decoder_param.dst_rate_ = dst_rate_;
		audio_decoder_param.resampler_interpolation_ = AudioDecoderInterpolationType::zoh;
		audio_decoder_param.resampler_lpf_ = false;

		auto audio_decoder = sfx_sound.audio_decoder.get();

		if (!audio_decoder->initialize(audio_decoder_param))
		{
			return false;
		}

		auto sample_count = audio_decoder->get_dst_length_in_samples();

		if (sample_count <= 0)
		{
			return false;
		}

		sfx_sound.is_initialized = true;
		sfx_sound.is_decoded = false;
		sfx_sound.sample_offset = 0;
		sfx_sound.sample_count = sample_count;
		sfx_sound.samples.resize(sample_count);
	}

	const auto sfx_voice = find_free_sfx_voice(
		priority,
		actor_type,
		actor_index,
		actor_channel
	);

	if (!sfx_voice)
	{
		return false;
	}

	const auto is_2d = is_2d_sfx(actor_type, actor_index);

	if (sfx_sound.is_decoded)
	{
		const auto decoded_data_size = static_cast<int>(sfx_sound.sample_count * sizeof(OalSourceSample));

		auto source_param = OalSourceOpenStaticParam{};
		source_param.is_2d = is_2d;
		source_param.is_8_bit = false;
		source_param.sample_rate = dst_rate_;
		source_param.data = sfx_sound.samples.data();
		source_param.data_size = decoded_data_size;

		sfx_voice->oal_source.open(source_param);
	}
	else
	{
		auto source_param = OalSourceOpenStreamingParam{};
		source_param.is_2d = is_2d;
		source_param.is_looping = false;
		source_param.sample_rate = dst_rate_;
		source_param.caching_sound = &sfx_sound;

		sfx_voice->oal_source.open(source_param);
	}

	auto& oal_source = sfx_voice->oal_source;
	oal_source.set_volume(sfx_volume_);
	set_sfx_position(oal_source, actor_type, actor_index);
	set_sfx_reference_distance(oal_source);
	oal_source.play();

	sfx_voice->is_active = true;
	sfx_voice->priority = priority;
	sfx_voice->actor_type = actor_type;
	sfx_voice->actor_index = actor_index;
	sfx_voice->actor_channel = actor_channel;

	return true;
}

void OalAudioMixer::make_al_context_current()
{
	const auto al_result = oal_symbols_->alcMakeContextCurrent(oal_context_resource_.get());

	if (al_result == ALC_TRUE)
	{
		return;
	}

	throw OalAudioMixerException{"Failed to make context current."};
}

OalAudioMixer::OalString OalAudioMixer::get_default_alc_device_name()
{
	auto default_device_name = OalString{};

	const auto default_alc_device_name = oal_symbols_->alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);

	if (default_alc_device_name)
	{
		default_device_name = default_alc_device_name;
	}

	return default_device_name;
}

OalAudioMixer::OalString OalAudioMixer::get_alc_device_name()
{
	auto device_name = OalString{};

	const auto alc_device_name = oal_symbols_->alcGetString(oal_device_resource_.get(), ALC_DEVICE_SPECIFIER);

	if (alc_device_name)
	{
		device_name = alc_device_name;
	}

	return device_name;
}

OalAudioMixer::OalStrings OalAudioMixer::get_alc_device_names()
{
	auto device_names = OalStrings{};
	device_names.reserve(4);

	auto alc_device_names = oal_symbols_->alcGetString(nullptr, ALC_DEVICE_SPECIFIER);

	if (alc_device_names)
	{
		while ((*alc_device_names) != '\0')
		{
			device_names.emplace_back(alc_device_names);
			alc_device_names += device_names.back().size() + 1;
		}
	}

	return device_names;
}

OalAudioMixer::OalStrings OalAudioMixer::parse_al_token_string(
	const char* al_token_string)
{
	auto tokens = OalStrings{};

	if (al_token_string)
	{
		{
			auto token_count = 0;

			for (auto al_token = al_token_string; *al_token != '\0'; ++al_token)
			{
				if ((*al_token) == ' ')
				{
					token_count += 1;
				}
			}

			tokens.reserve(token_count + 1);
		}

		auto is_parsed = false;
		auto al_token = al_token_string;
		auto al_token_begin = al_token_string;

		while (!is_parsed)
		{
			switch (*al_token)
			{
				case '\0':
					is_parsed = true;

				case ' ':
					if (al_token != al_token_begin)
					{
						tokens.emplace_back();
						tokens.back().assign(al_token_begin, al_token - al_token_begin);
					}

					al_token_begin = al_token + 1;

					break;

				default:
					break;
			}

			al_token += 1;
		}
	}

	return tokens;
}

OalAudioMixer::OalStrings OalAudioMixer::get_alc_extensions()
{
	const auto alc_extensions = oal_symbols_->alcGetString(oal_device_resource_.get(), ALC_EXTENSIONS);
	const auto extensions = parse_al_token_string(alc_extensions);

	return extensions;
}

OalAudioMixer::OalStrings OalAudioMixer::get_al_extensions()
{
	const auto al_extensions = oal_symbols_->alGetString(AL_EXTENSIONS);
	const auto extensions = parse_al_token_string(al_extensions);

	return extensions;
}

int OalAudioMixer::get_al_mixing_frequency()
{
	auto al_attribute_size = ALCint{};

	oal_symbols_->alcGetIntegerv(
		oal_device_resource_.get(),
		ALC_ATTRIBUTES_SIZE,
		1,
		&al_attribute_size
	);

	constexpr auto max_attributes = 64;
	constexpr auto max_al_attributes_size = (2 * (max_attributes - 1)) + 1;

	if (al_attribute_size <= 0 || al_attribute_size > max_al_attributes_size)
	{
		throw OalAudioMixerException{"Attributes size out of range."};
	}

	struct OalAttribute
	{
		ALint name;
		ALint value;
	}; // OalAttribute

	using OalAttributes = std::array<OalAttribute, max_attributes>;
	auto al_attributes = OalAttributes{};

	oal_symbols_->alcGetIntegerv(
		oal_device_resource_.get(),
		ALC_ALL_ATTRIBUTES,
		max_al_attributes_size,
		reinterpret_cast<ALCint*>(al_attributes.data())
	);

	for (const auto al_attribute : al_attributes)
	{
		if (al_attribute.name == 0)
		{
			break;
		}

		if (al_attribute.name == ALC_FREQUENCY)
		{
			return al_attribute.value;
		}
	}

	throw OalAudioMixerException{"No such attribute."};
}

int OalAudioMixer::get_max_voice_count()
{
	using OalSourceResources = std::array<OalSourceResource, voices_limit>;

	auto oal_source_resources = OalSourceResources{};

	auto voice_count = 0;

	try
	{
		for (auto& oal_source_resource : oal_source_resources)
		{
			oal_source_resource = make_oal_source(oal_symbols_);
			voice_count += 1;
		}
	}
	catch (...)
	{
	}

	return voice_count;
}

void OalAudioMixer::log(
	const OalString& string)
{
	static const auto prefix = OalString{"[SND_OAL] "};

	logger_->write(prefix + string);
}

void OalAudioMixer::log_oal_library_path_name()
{
	log(std::string{"Default library: \""} + get_oal_default_library_path_name() + '\"');
	log("Library: \"" + sd_oal_library + '\"');
}

void OalAudioMixer::log_oal_device_name()
{
	log(std::string{"Device name: \""} + sd_oal_device_name + '\"');
}

void OalAudioMixer::log_oal_device_names()
{
	log("Device names:");

	const auto device_names = get_alc_device_names();

	for (const auto& device_name : device_names)
	{
		log("\t\"" + device_name + '\"');
	}
}

void OalAudioMixer::log_oal_default_device_name()
{
	const auto default_device_name = get_default_alc_device_name();

	log("Default device name: \"" + default_device_name + '\"');
}

void OalAudioMixer::log_oal_current_device_name()
{
	const auto current_device_name = get_alc_device_name();

	log("Current device name: \"" + current_device_name + '\"');
}

void OalAudioMixer::log_oal_alc_extensions()
{
	log("Device extensions:");

	const auto extensions = get_alc_extensions();

	for (const auto& extension : extensions)
	{
		log('\t' + extension);
	}
}

void OalAudioMixer::log_oal_al_info()
{
	const auto al_version = oal_symbols_->alGetString(AL_VERSION);
	const auto version = OalString(al_version ? al_version : "");
	log("Version: " + version);

	const auto al_renderer = oal_symbols_->alGetString(AL_RENDERER);
	const auto renderer = OalString(al_renderer ? al_renderer : "");
	log("Renderer: " + renderer);

	const auto al_vendor = oal_symbols_->alGetString(AL_VENDOR);
	const auto vendor = OalString(al_vendor ? al_vendor : "");
	log("Vendor: " + vendor);
}

void OalAudioMixer::log_oal_al_extensions()
{
	log("Context extensions:");

	const auto extensions = get_al_extensions();

	for (const auto& extension : extensions)
	{
		log('\t' + extension);
	}
}

const char* OalAudioMixer::get_oal_default_library_path_name() noexcept
{
	return
#if _WIN32
		"OpenAL32.dll"
#else
		"libopenal.so"
#endif // _WIN32
	;
}

void OalAudioMixer::initialize_oal(
	const AudioMixerInitParam& param)
{
	ALCint al_context_attributes[] =
	{
		0, 0,
		0,
	};

	if (param.dst_rate_ > 0)
	{
		al_context_attributes[0] = ALC_FREQUENCY;
		al_context_attributes[1] = param.dst_rate_;
	}

	log_oal_library_path_name();
	log_oal_device_name();

	oal_loader_ = make_oal_loader(!sd_oal_library.empty() ? sd_oal_library.c_str() : get_oal_default_library_path_name());

	oal_symbols_ = &oal_loader_->get_symbols();

	log_oal_device_names();
	log_oal_default_device_name();

	oal_device_resource_ = make_oal_device(
		oal_symbols_,
		!sd_oal_device_name.empty() ? sd_oal_device_name.c_str() : nullptr
	);

	log_oal_current_device_name();
	log_oal_alc_extensions();

	oal_context_resource_ = make_oal_context(oal_symbols_, oal_device_resource_.get(), al_context_attributes);
	make_al_context_current();

	log_oal_al_info();
	log_oal_al_extensions();

	dst_rate_ = get_al_mixing_frequency();
}

void OalAudioMixer::initialize_music_adlib_sound()
{
	music_adlib_sound_.is_initialized = true;
	music_adlib_sound_.queue_size = 0;
	music_adlib_sound_.read_sample_offset = 0;
	music_adlib_sound_.write_sample_offset = 0;
	music_adlib_sound_.samples.resize(mix_sample_count_ * oal_source_max_streaming_buffers);
	music_adlib_sound_.audio_decoder = make_audio_decoder(AudioDecoderType::adlib_music, opl3_type_);
}

void OalAudioMixer::initialize_music_source()
{
	auto param = OalSourceInitParam{};
	param.mix_sample_rate = dst_rate_;
	param.mix_sample_count = mix_sample_count_;
	param.oal_symbols = oal_symbols_;

	music_source_.initialize(param);
}

void OalAudioMixer::initialize_music()
{
	initialize_music_adlib_sound();
	initialize_music_source();
}

void OalAudioMixer::uninitialize_music()
{
	music_source_.uninitialize();
	music_adlib_sound_.is_initialized = false;
	music_adlib_sound_.audio_decoder = nullptr;
}

void OalAudioMixer::initialize_sfx_adlib_sounds()
{
	for (auto& sfx_adlib_sound : sfx_adlib_sounds_)
	{
		sfx_adlib_sound.is_initialized = false;
		sfx_adlib_sound.audio_decoder = make_audio_decoder(AudioDecoderType::adlib_sfx, opl3_type_);

		if (!sfx_adlib_sound.audio_decoder)
		{
			throw OalAudioMixerException{"Failed to create SFX AdLib audio decoder."};
		}
	}
}

void OalAudioMixer::initialize_sfx_pc_speaker_sounds()
{
	for (auto& sfx_pc_speaker_sound : sfx_pc_speaker_sounds_)
	{
		sfx_pc_speaker_sound.is_initialized = false;
		sfx_pc_speaker_sound.audio_decoder = make_audio_decoder(AudioDecoderType::pc_speaker, opl3_type_);

		if (!sfx_pc_speaker_sound.audio_decoder)
		{
			throw OalAudioMixerException{"Failed to create SFX PC Speaker audio decoder."};
		}
	}
}

void OalAudioMixer::initialize_sfx_voices()
{
	const auto max_voice_count = get_max_voice_count();
	const auto max_sfx_voice_count = max_voice_count - music_voices_limit;

	if (max_sfx_voice_count <= 0)
	{
		return;
	}

	sfx_voices_.resize(max_sfx_voice_count);

	auto param = OalSourceInitParam{};
	param.mix_sample_rate = dst_rate_;
	param.mix_sample_count = mix_sample_count_;
	param.oal_symbols = oal_symbols_;

	for (auto& sfx_voice : sfx_voices_)
	{
		sfx_voice.index = 0;
		sfx_voice.is_active = false;
		sfx_voice.oal_source.initialize(param);
	}
}

void OalAudioMixer::initialize_sfx_positions()
{
	sfx_player_ = SfxPlayer{};

	sfx_actor_positions_ = SfxActorPositions{};
	sfx_door_positions_ = SfxDoorPositions{};

	sfx_actor_modified_position_indices_.clear();
	sfx_actor_modified_position_indices_.reserve(sfx_actor_positions_.size());

	sfx_door_modified_position_indices_.clear();
	sfx_door_modified_position_indices_.reserve(sfx_door_positions_.size());

	sfx_pushwall_ = SfxPushwall{};
}

void OalAudioMixer::initialize_sfx()
{
	initialize_sfx_adlib_sounds();
	initialize_sfx_pc_speaker_sounds();
	initialize_sfx_voices();
	initialize_sfx_positions();
}

void OalAudioMixer::uninitialize_sfx()
{
	for (auto& sfx_adlib_sound : sfx_adlib_sounds_)
	{
		sfx_adlib_sound.audio_decoder = nullptr;
	}

	for (auto& sfx_pc_speaker_sound : sfx_pc_speaker_sounds_)
	{
		sfx_pc_speaker_sound.audio_decoder = nullptr;
	}

	sfx_voices_.clear();
}

void OalAudioMixer::decode_adlib_sound(
	OalSourceCachingSound& adlib_sound,
	int volume_scale)
{
	if (!adlib_sound.is_initialized || adlib_sound.is_decoded)
	{
		return;
	}

	const auto remain_count = adlib_sound.sample_count - adlib_sound.sample_offset;

	if (remain_count == 0)
	{
		adlib_sound.is_decoded = true;
		return;
	}

	const auto sample_count = std::min(remain_count, oal_source_max_streaming_buffers * dst_rate_);
	const auto samples = &adlib_sound.samples[adlib_sound.sample_offset];
	const auto decoded_count = adlib_sound.audio_decoder->decode(sample_count, samples);

	for (auto i = 0; i < decoded_count; ++i)
	{
		samples[i] = scale_sample(samples[i], volume_scale);
	}

	adlib_sound.sample_offset += decoded_count;

	if (decoded_count <= sample_count)
	{
		adlib_sound.is_decoded = true;
		adlib_sound.sample_count = adlib_sound.sample_offset;
	}
}

void OalAudioMixer::decode_pc_speaker_sound(
	OalSourceCachingSound& pc_speaker_sound)
{
	if (!pc_speaker_sound.is_initialized || pc_speaker_sound.is_decoded)
	{
		return;
	}

	const auto remain_count = pc_speaker_sound.sample_count - pc_speaker_sound.sample_offset;

	if (remain_count == 0)
	{
		pc_speaker_sound.is_decoded = true;
		return;
	}

	const auto sample_count = std::min(remain_count, oal_source_max_streaming_buffers * dst_rate_);
	const auto samples = &pc_speaker_sound.samples[pc_speaker_sound.sample_offset];
	const auto decoded_count = pc_speaker_sound.audio_decoder->decode(sample_count, samples);

	pc_speaker_sound.sample_offset += decoded_count;

	if (decoded_count <= sample_count)
	{
		pc_speaker_sound.is_decoded = true;
		pc_speaker_sound.sample_count = pc_speaker_sound.sample_offset;
	}
}

void OalAudioMixer::mix_sfx_voice(
	SfxVoice& sfx_voice)
{
	if (!sfx_voice.is_active)
	{
		return;
	}

	auto& oal_source = sfx_voice.oal_source;

	oal_source.mix();

	if (oal_source.is_finished())
	{
		sfx_voice.is_active = false;
	}
}

bool OalAudioMixer::mix_music_mix_buffer()
{
	auto decode_offset = 0;
	auto audio_decoder = music_adlib_sound_.audio_decoder.get();
	auto to_decode_count = mix_sample_count_;

	const auto max_mix_sample_count = oal_source_max_streaming_buffers * mix_sample_count_;

	if (music_adlib_sound_.write_sample_offset >= max_mix_sample_count)
	{
		music_adlib_sound_.write_sample_offset = 0;
	}

	while (to_decode_count > 0)
	{
		const auto samples = &music_adlib_sound_.samples[music_adlib_sound_.write_sample_offset + decode_offset];
		const auto decoded_count = audio_decoder->decode(to_decode_count, samples);

		if (decoded_count > 0)
		{
			for (auto i = 0; i < decoded_count; ++i)
			{
				samples[i] = scale_sample(samples[i], adlib_music_volume_scale);
			}

			decode_offset += decoded_count;
			to_decode_count -= decoded_count;
		}
		else
		{
			sd_sq_played_once_ = true;

			if (!audio_decoder->rewind())
			{
				return false;
			}
		}
	}

	if (decode_offset != mix_sample_count_)
	{
		return false;
	}

	music_adlib_sound_.write_sample_offset += mix_sample_count_;

	return true;
}

bool OalAudioMixer::mix_music_mix_buffers()
{
	while (music_adlib_sound_.queue_size < oal_source_max_streaming_buffers)
	{
		if (!mix_music_mix_buffer())
		{
			return false;
		}

		music_adlib_sound_.queue_size += 1;
	}

	return true;
}

void OalAudioMixer::mix_music()
{
	if (!music_adlib_sound_.is_initialized || !music_source_.is_open())
	{
		return;
	}

	if (!mix_music_mix_buffers())
	{
		music_source_.close();
		return;
	}

	music_source_.mix();

	if (music_source_.is_finished())
	{
		music_source_.close();
	}
}

void OalAudioMixer::initialize_thread()
{
	is_quit_thread_ = false;
	thread_ = Thread{&OalAudioMixer::thread_func, this};
}

void OalAudioMixer::thread_func()
{
	constexpr auto delay = std::chrono::milliseconds{min_thread_delay_ms};

	while (true)
	{
		{
			const auto thread_mutex_guard = MutexUniqueLock{thread_mutex_};

			if (is_quit_thread_)
			{
				return;
			}
		}

		{
			const auto sfx_mutex_guard = MutexUniqueLock{sfx_mutex_};

			for (auto& sfx_adlib_sound : sfx_adlib_sounds_)
			{
				decode_adlib_sound(sfx_adlib_sound, adlib_sfx_volume_scale);
			}
		}

		{
			const auto sfx_mutex_guard = MutexUniqueLock{sfx_mutex_};

			for (auto& sfx_pc_speaker_sound : sfx_pc_speaker_sounds_)
			{
				decode_pc_speaker_sound(sfx_pc_speaker_sound);
			}
		}

		{
			const auto sfx_mutex_guard = MutexUniqueLock{sfx_mutex_};

			for (auto& sfx_voice : sfx_voices_)
			{
				mix_sfx_voice(sfx_voice);
			}
		}

		{
			const auto music_mutex_guard = MutexUniqueLock{music_mutex_};

			mix_music();
		}

		std::this_thread::sleep_for(delay);
	}
}

bool OalAudioMixer::is_2d_sfx(
	ActorType actor_type,
	int actor_index)
{
	switch (actor_type)
	{
		case ActorType::actor:
			return actor_index <= 0;

		case ActorType::door:
			return actor_index < 0;

		case ActorType::wall:
		default:
			return false;
	}
}

OalAudioMixer::SfxVoice* OalAudioMixer::find_free_sfx_voice(
	int priority,
	ActorType actor_type,
	int actor_index,
	ActorChannel actor_channel)
{
	if (actor_index < 0)
	{
		for (auto& sfx_voice : sfx_voices_)
		{
			if (!sfx_voice.is_active)
			{
				return &sfx_voice;
			}
		}

		return nullptr;
	}
	else
	{
		auto free_sfx = static_cast<SfxVoice*>(nullptr);

		for (auto& sfx_voice : sfx_voices_)
		{
			if (!sfx_voice.is_active && !free_sfx)
			{
				free_sfx = &sfx_voice;
			}

			if (sfx_voice.is_active &&
				sfx_voice.actor_type == actor_type &&
				sfx_voice.actor_index == actor_index &&
				sfx_voice.actor_channel == actor_channel)
			{
				if (sfx_voice.priority > priority)
				{
					return nullptr;
				}

				sfx_voice.is_active = false;
				sfx_voice.oal_source.stop();

				return &sfx_voice;
			}
		}

		return free_sfx;
	}
}

void OalAudioMixer::set_al_listener_position(
	float x,
	float y,
	float z)
{
	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alListener3f);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alListener3f(AL_POSITION, x, y, z);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::set_al_listener_orientation(
	float at_x,
	float at_y,
	float at_z,
	float up_x,
	float up_y,
	float up_z)
{
	const ALfloat al_orientation[] =
	{
		at_x, at_y, at_z,
		up_x, up_y, up_z,
	};

	assert(oal_symbols_->alGetError);
	assert(oal_symbols_->alListenerfv);

	static_cast<void>(oal_symbols_->alGetError());
	oal_symbols_->alListenerfv(AL_ORIENTATION, al_orientation);
	assert(oal_symbols_->alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::update_positions_player()
{
	is_sfx_player_position_modified_ = false;

	if (sfx_player_.view_x != viewx ||
		sfx_player_.view_y != viewy ||
		sfx_player_.view_cos != viewcos ||
		sfx_player_.view_sin != viewsin)
	{
		sfx_player_.view_x = viewx;
		sfx_player_.view_y = viewy;
		sfx_player_.view_cos = viewcos;
		sfx_player_.view_sin = viewsin;

		is_sfx_player_position_modified_ = true;
	}
}

void OalAudioMixer::update_positions_actors()
{
	sfx_actor_modified_position_indices_.clear();

	// Actor at index "0" is the player.
	for (auto i = 1; i < MAXACTORS; ++i)
	{
		const auto& bs_actor = objlist[i];
		auto& sfx_actor = sfx_actor_positions_[i];

		if (sfx_actor.x != bs_actor.x ||
			sfx_actor.y != bs_actor.y)
		{
			sfx_actor.x = bs_actor.x;
			sfx_actor.y = bs_actor.y;

			sfx_actor_modified_position_indices_.emplace(i);
		}
	}
}

void OalAudioMixer::update_positions_doors()
{
	sfx_door_modified_position_indices_.clear();

	for (auto i = 0; i < MAXDOORS; ++i)
	{
		const auto& bs_door = doorobjlist[i];
		auto& sfx_door = sfx_door_positions_[i];

		if (sfx_door.x != bs_door.tilex ||
			sfx_door.y != bs_door.tiley)
		{
			sfx_door.x = bs_door.tilex;
			sfx_door.y = bs_door.tiley;

			sfx_door_modified_position_indices_.emplace(i);
		}
	}
}

void OalAudioMixer::update_positions_pushwall()
{
	is_sfx_pushwall_position_modified_ = false;

	if (sfx_pushwall_.direction != pwalldir ||
		sfx_pushwall_.x != pwallx ||
		sfx_pushwall_.y != pwally ||
		sfx_pushwall_.offset != pwallpos)
	{
		sfx_pushwall_.direction = pwalldir;
		sfx_pushwall_.x = pwallx;
		sfx_pushwall_.y = pwally;
		sfx_pushwall_.offset = pwallpos;

		is_sfx_pushwall_position_modified_ = true;
	}
}

void OalAudioMixer::apply_positions_player()
{
	if (!is_sfx_player_position_modified_)
	{
		return;
	}

	const auto al_position_x = static_cast<float>(sfx_player_.view_x * meters_per_unit);
	const auto al_position_y = static_cast<float>(default_position_y * meters_per_unit);
	const auto al_position_z = static_cast<float>((sfx_player_.view_y - MAPSIZE) * meters_per_unit);

	set_al_listener_position(al_position_x, al_position_y, al_position_z);


	const auto al_at_x = static_cast<float>(sfx_player_.view_cos);
	const auto al_at_y = 0.0F;
	const auto al_at_z = static_cast<float>(-sfx_player_.view_sin);

	const auto al_up_x = 0.0F;
	const auto al_up_y = 1.0F;
	const auto al_up_z = 0.0F;

	set_al_listener_orientation(al_at_x, al_at_y, al_at_z, al_up_x, al_up_y, al_up_z);
}

void OalAudioMixer::apply_actor_position(
	SfxVoice& sfx_voice)
{
	if (!sfx_voice.is_active ||
		sfx_voice.actor_type != ActorType::actor ||
		sfx_voice.actor_index <= 0)
	{
		return;
	}

	if (sfx_actor_modified_position_indices_.count(sfx_voice.actor_index) == 0)
	{
		return;
	}

	set_sfx_actor_position(sfx_voice.oal_source, sfx_voice.actor_index);
}

void OalAudioMixer::apply_door_position(
	SfxVoice& sfx_voice)
{
	if (!sfx_voice.is_active ||
		sfx_voice.actor_type != ActorType::door ||
		sfx_voice.actor_index < 0)
	{
		return;
	}

	if (sfx_door_modified_position_indices_.count(sfx_voice.actor_index) == 0)
	{
		return;
	}

	set_sfx_door_position(sfx_voice.oal_source, sfx_voice.actor_index);
}

void OalAudioMixer::apply_pushwall_position(
	SfxVoice& sfx_voice)
{
	if (!sfx_voice.is_active ||
		sfx_voice.actor_type != ActorType::wall)
	{
		return;
	}

	set_sfx_pushwall_position(sfx_voice.oal_source, sfx_voice.actor_index);
}

void OalAudioMixer::apply_positions_actors()
{
	if (sfx_actor_modified_position_indices_.empty())
	{
		return;
	}

	for (auto& sfx_voice : sfx_voices_)
	{
		apply_actor_position(sfx_voice);
	}
}

void OalAudioMixer::apply_positions_doors()
{
	if (sfx_actor_modified_position_indices_.empty())
	{
		return;
	}

	for (auto& sfx_voice : sfx_voices_)
	{
		apply_door_position(sfx_voice);
	}
}

void OalAudioMixer::apply_positions_pushwall()
{
	if (!is_sfx_pushwall_position_modified_)
	{
		return;
	}

	for (auto& sfx_voice : sfx_voices_)
	{
		apply_pushwall_position(sfx_voice);
	}
}

void OalAudioMixer::apply_positions()
{
	apply_positions_actors();
	apply_positions_doors();
	apply_positions_pushwall();
}

void OalAudioMixer::set_sfx_actor_position(
	OalSource& oal_source,
	int actor_index)
{
	const auto& sfx_position = sfx_actor_positions_[actor_index];
	const auto al_position_x = static_cast<float>(sfx_position.x * meters_per_unit);
	const auto al_position_y = static_cast<float>(default_position_y * meters_per_unit);
	const auto al_position_z = static_cast<float>((sfx_position.y - MAPSIZE) * meters_per_unit);

	oal_source.set_position(al_position_x, al_position_y, al_position_z);
}

void OalAudioMixer::set_sfx_door_position(
	OalSource& oal_source,
	int actor_index)
{
	const auto& sfx_position = sfx_door_positions_[actor_index];
	const auto al_position_x = static_cast<float>((sfx_position.x + 0.5) * meters_per_unit);
	const auto al_position_y = static_cast<float>(default_position_y * meters_per_unit);
	const auto al_position_z = static_cast<float>((sfx_position.y - MAPSIZE + 0.5) * meters_per_unit);

	oal_source.set_position(al_position_x, al_position_y, al_position_z);
}

void OalAudioMixer::set_sfx_pushwall_position(
	OalSource& oal_source,
	int actor_index)
{
	auto x = sfx_pushwall_.x + 0.5;
	auto y = sfx_pushwall_.y + 0.5;

	switch (pwalldir)
	{
		case di_east:
			x += sfx_pushwall_.offset;
			break;

		case di_north:
			y -= sfx_pushwall_.offset;
			break;

		case di_south:
			y += sfx_pushwall_.offset;
			break;

		case di_west:
			x -= sfx_pushwall_.offset;
			break;

		default:
			break;
	}

	const auto al_position_x = static_cast<float>(x * meters_per_unit);
	const auto al_position_y = static_cast<float>(default_position_y * meters_per_unit);
	const auto al_position_z = static_cast<float>((y - MAPSIZE) * meters_per_unit);

	oal_source.set_position(al_position_x, al_position_y, al_position_z);
}

void OalAudioMixer::set_sfx_position(
	OalSource& oal_source,
	ActorType actor_type,
	int actor_index)
{
	switch (actor_type)
	{
		case ActorType::actor:
			if (actor_index > 0)
			{
				set_sfx_actor_position(oal_source, actor_index);
			}

			break;

		case ActorType::door:
			if (actor_index >= 0)
			{
				set_sfx_door_position(oal_source, actor_index);
			}
			break;

		case ActorType::wall:
			set_sfx_pushwall_position(oal_source, actor_index);
			break;

		default:
			break;
	}
}

void OalAudioMixer::set_sfx_reference_distance(
	OalSource& oal_source)
{
	oal_source.set_reference_distance(
		static_cast<float>(default_reference_distance * meters_per_unit));
}

OalSourceSample OalAudioMixer::scale_sample(
	OalSourceSample sample,
	int scalar) noexcept
{
	auto new_sample = sample * scalar;

	if (new_sample < -32'768)
	{
		new_sample = -32'768;
	}
	else if (new_sample > +32'767)
	{
		new_sample = +32'767;
	}

	return static_cast<OalSourceSample>(new_sample);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
