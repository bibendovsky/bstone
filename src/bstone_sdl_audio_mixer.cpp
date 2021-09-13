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

#include "bstone_sdl_audio_mixer.h"

#include <cassert>
#include <cmath>
#include <algorithm>
#include <mutex>
#include "SDL_audio.h"
#include "3d_def.h"
#include "audio.h"
#include "bstone_audio_decoder.h"
#include "bstone_audio_mixer_utils.h"
#include "bstone_audio_mixer_validator.h"
#include "bstone_audio_sample_converter.h"
#include "bstone_exception.h"
#include "bstone_scope_guard.h"

namespace bstone
{

namespace
{

class SdlAudioMixerException : public Exception
{
public:
	explicit SdlAudioMixerException(const char* message) noexcept
		:
		Exception{"SDL_AUDIO_MIXER", message}
	{
	}
}; // SdlAudioMixerException

} // namespace


SdlAudioMixer::CacheItem::CacheItem()
	:
	is_active{},
	is_invalid{},
	sound_type{},
	samples_count{},
	decoded_count{},
	buffer_size_{},
	decoder{}
{
}

bool SdlAudioMixer::CacheItem::is_decoded() const noexcept
{
	return decoded_count == samples_count;
}

bool SdlAudioMixer::Voice::is_audible() const noexcept
{
	return left_gain > 0.0 || right_gain > 0.0;
}

SdlAudioMixer::SdlAudioMixer(const AudioMixerInitParam& param)
try
{
	if (param.max_voices < 0)
	{
		fail("Max voice count out of range.");
	}

	switch (param.opl3_type)
	{
		case Opl3Type::dbopl:
		case Opl3Type::nuked:
			break;

		default:
			fail("Unknown OPL3 type.");
	}

	if (param.dst_rate == 0)
	{
		dst_rate_ = get_default_rate();
	}
	else
	{
		dst_rate_ = std::max(param.dst_rate, get_min_rate());
	}

	if (param.mix_size_ms == 0)
	{
		mix_size_ms_ = get_default_mix_size_ms();
	}
	else
	{
		mix_size_ms_ = std::max(param.mix_size_ms, get_min_mix_size_ms());
	}

	mix_samples_count_ = calculate_mix_samples_count(dst_rate_, mix_size_ms_);

	auto src_spec = SDL_AudioSpec{};
	src_spec.freq = dst_rate_;
	src_spec.format = AUDIO_F32SYS;
	src_spec.channels = static_cast<std::uint8_t>(get_max_channels());
	src_spec.samples = static_cast<std::uint16_t>(mix_samples_count_);
	src_spec.callback = callback_proxy;
	src_spec.userdata = this;

	auto dst_spec = SDL_AudioSpec{};

	sdl_audio_device_.reset(SDL_OpenAudioDevice(
		nullptr,
		0,
		&src_spec,
		&dst_spec,
		SDL_AUDIO_ALLOW_FREQUENCY_CHANGE));

	if (!sdl_audio_device_)
	{
		auto error_message = std::string{};
		error_message.reserve(1'024);
		error_message += (SDL_GetError() ? SDL_GetError() : "Generic failure.");
		fail(error_message.c_str());
	}

	opl3_type_ = param.opl3_type;
	mix_samples_count_ = dst_spec.samples;
	const auto total_samples = get_max_channels() * mix_samples_count_;
	buffer_.resize(total_samples);
	s16_samples_.resize(total_samples);
	mix_buffer_.resize(total_samples);
	adlib_music_cache_.resize(LASTMUSIC);
	adlib_sfx_cache_.resize(NUMSOUNDS);
	pc_speaker_sfx_cache_.resize(NUMSOUNDS);
	pcm_cache_.resize(NUMSOUNDS);

	const auto commands_reserve = param.max_voices * 4;
	commands_.reserve(commands_reserve);
	mt_commands_.reserve(commands_reserve);

	initialize_mute();
	initialize_distance_model();
	initialize_listener_meters_per_unit();
	initialize_listener_r3_position();
	initialize_listener_r3_orientation();
	initialize_voice_handles();
	initialize_voices(param.max_voices);

	SDL_PauseAudioDevice(sdl_audio_device_.get(), 0);
}
catch (...)
{
	fail_nested(__func__);
}

Opl3Type SdlAudioMixer::get_opl3_type() const
{
	return opl3_type_;
}

int SdlAudioMixer::get_rate() const
{
	return dst_rate_;
}

int SdlAudioMixer::get_channel_count() const
{
	return get_max_channels();
}

int SdlAudioMixer::get_mix_size_ms() const
{
	return mix_size_ms_;
}

void SdlAudioMixer::set_mute(bool is_mute)
try
{
	auto command = Command{};
	command.type = CommandType::set_mute;
	command.param.set_mute.is_mute = is_mute;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void SdlAudioMixer::set_distance_model(AudioMixerDistanceModel distance_model)
try
{
	AudioMixerValidator::validate_distance_model(distance_model);
	auto command = Command{};
	command.type = CommandType::set_distance_model;
	command.param.set_distance_model.distance_model = distance_model;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

int SdlAudioMixer::get_min_rate() const noexcept
{
	return 11'025;
}

int SdlAudioMixer::get_default_rate() const noexcept
{
	return 44'100;
}

int SdlAudioMixer::get_min_mix_size_ms() const noexcept
{
	return 20;
}

int SdlAudioMixer::get_default_mix_size_ms() const noexcept
{
	return 40;
}

int SdlAudioMixer::get_max_channels() const noexcept
{
	return 2;
}

void SdlAudioMixer::suspend_state()
try
{
	is_state_suspended_.store(true, std::memory_order_release);
}
catch (...)
{
	fail_nested(__func__);
}

void SdlAudioMixer::resume_state()
try
{
	is_state_suspended_.store(false, std::memory_order_release);
}
catch (...)
{
	fail_nested(__func__);
}

void SdlAudioMixer::set_listener_meters_per_unit(double meters_per_unit)
try
{
	AudioMixerValidator::validate_listener_meters_per_unit(meters_per_unit);
	auto command = Command{};
	command.type = CommandType::set_listener_meters_per_unit;
	command.param.set_listener_meters_per_unit.meters_per_unit = meters_per_unit;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void SdlAudioMixer::set_listener_r3_position(const AudioMixerListenerR3Position& r3_position)
try
{
	auto command = Command{};
	command.type = CommandType::set_listener_r3_position;
	command.param.set_listener_r3_position.r3_position = r3_position;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void SdlAudioMixer::set_listener_r3_orientation(const AudioMixerListenerR3Orientation& r3_orientation)
try
{
	auto command = Command{};
	command.type = CommandType::set_listener_r3_orientation;
	command.param.set_listener_r3_orientation.r3_orientation = r3_orientation;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

bool SdlAudioMixer::is_voice_playing(AudioMixerVoiceHandle voice_handle) const
try
{
	return voice_handle_mgr_.is_valid_handle(voice_handle);
}
catch (...)
{
	fail_nested(__func__);
}

void SdlAudioMixer::pause_voice(AudioMixerVoiceHandle voice_handle)
{
	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::pause_voice;
	command.param.pause_voice.handle = voice_handle;

	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}

void SdlAudioMixer::resume_voice(AudioMixerVoiceHandle voice_handle)
{
	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::resume_voice;
	command.param.resume_voice.handle = voice_handle;

	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}

void SdlAudioMixer::stop_voice(AudioMixerVoiceHandle voice_handle)
try
{
	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::stop_voice;
	command.param.stop_voice.handle = voice_handle;

	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void SdlAudioMixer::set_voice_gain(AudioMixerVoiceHandle voice_handle, double gain)
try
{
	AudioMixerValidator::validate_gain(gain);

	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::set_voice_gain;
	command.param.set_voice_gain.handle = voice_handle;
	command.param.set_voice_gain.gain = gain;

	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void SdlAudioMixer::set_voice_r3_attenuation(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Attenuation& r3_attenuation)
try
{
	AudioMixerValidator::validate_voice_r3_attenuation(r3_attenuation);

	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::set_voice_r3_attenuation;
	command.param.set_voice_r3_attenuation.handle = voice_handle;
	command.param.set_voice_r3_attenuation.attributes = r3_attenuation;

	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void SdlAudioMixer::set_voice_r3_position(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Position& r3_position)
try
{
	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::set_voice_r3_position;
	command.param.set_voice_r3_position.handle = voice_handle;
	command.param.set_voice_r3_position.position = r3_position;

	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]] void SdlAudioMixer::fail(const char* message)
{
	throw SdlAudioMixerException{message};
}

[[noreturn]] void SdlAudioMixer::fail_nested(const char* message)
{
	std::throw_with_nested(SdlAudioMixerException{message});
}

void SdlAudioMixer::initialize_mute()
{
	is_mute_ = false;
}

void SdlAudioMixer::initialize_distance_model()
{
	distance_model_ = audio_mixer_default_distance_model;
}

void SdlAudioMixer::initialize_listener_meters_per_unit()
{
	is_meters_per_unit_changed_ = true;
	listener_meters_per_unit_ = audio_mixer_default_meters_per_units;
}

void SdlAudioMixer::initialize_listener_r3_position()
{
	is_listener_r3_position_changed_ = true;
	listener_r3_position_ = audio_mixer_make_default_listener_r3_position();
}

void SdlAudioMixer::initialize_listener_r3_orientation()
{
	is_listener_r3_orientation_changed_ = true;
	listener_r3_orientation_ = audio_mixer_make_default_listener_r3_orientation();
}

void SdlAudioMixer::initialize_voice_handles()
{
	const auto capacity = static_cast<int>(commands_.size());
	voice_handle_mgr_.set_cache_capacity(capacity);
	voice_handle_mgr_.set_map_capacity(capacity);
}

void SdlAudioMixer::initialize_voices(int max_voices)
{
	voices_.clear();
	voices_.resize(max_voices);
}

void SdlAudioMixer::callback(std::uint8_t* dst_data, int dst_length)
{
	std::uninitialized_copy_n(reinterpret_cast<const std::uint8_t*>(buffer_.data()), dst_length, dst_data);
}

void SdlAudioMixer::mix()
{
	handle_commands();
	mix_samples();
}

void SdlAudioMixer::mix_samples()
{
	spatialize_voices();

	std::uninitialized_fill(buffer_.begin(), buffer_.end(), Sample{});
	std::uninitialized_fill(mix_buffer_.begin(), mix_buffer_.end(), MixSample{});

	constexpr auto sfx_gain_scale = 7.0F;
	constexpr auto music_gain_scale = 6.0F;

	for (auto& voice : voices_)
	{
		if (!voice.is_active || voice.is_paused)
		{
			continue;
		}

		if (!decode_voice(voice))
		{
			voice_handle_mgr_.unmap(voice.handle);
			voice.is_active = false;
			continue;
		}

		const auto is_adlib_music = (voice.type == SoundType::adlib_music);

		auto cache_item = voice.cache;

		if (!is_adlib_music && voice.decode_offset == cache_item->decoded_count)
		{
			voice_handle_mgr_.unmap(voice.handle);
			voice.is_active = false;
			continue;
		}

		if (!voice.is_audible())
		{
			continue;
		}

		auto gain_scale = voice.gain;

		switch (voice.type)
		{
			case SoundType::adlib_music:
				gain_scale *= music_gain_scale;
				break;

			case SoundType::adlib_sfx:
				gain_scale *= sfx_gain_scale;
				break;

			default:
				break;
		}

		auto decode_count = 0;

		if (is_adlib_music)
		{
			decode_count = cache_item->buffer_size_;
		}
		else
		{
			const auto remain_count = cache_item->decoded_count - voice.decode_offset;
			assert(remain_count >= 0);
			decode_count = std::min(remain_count, mix_samples_count_);
		}

		auto& mix_buffer = mix_buffer_;
		const auto base_offset = (is_adlib_music ? 0 : voice.decode_offset);

		for (int i = 0; i < decode_count; ++i)
		{
			const auto sample = gain_scale * cache_item->samples[base_offset + i];
			const auto left_sample = static_cast<Sample>(voice.left_gain * sample);
			const auto right_sample = static_cast<Sample>(voice.right_gain * sample);

			mix_buffer[(2 * i) + 0] += left_sample;
			mix_buffer[(2 * i) + 1] += right_sample;
		}

		if (!is_adlib_music)
		{
			voice.decode_offset += decode_count;
		}

		if ((is_adlib_music && cache_item->is_decoded()) ||
			(!is_adlib_music && voice.decode_offset == cache_item->decoded_count))
		{
			if (cache_item->is_decoded())
			{
				auto is_erase = false;

				if (voice.type == SoundType::adlib_music)
				{
					if (voice.is_looping && cache_item->decoder->rewind())
					{
						cache_item->decoded_count = 0;
						cache_item->buffer_size_ = 0;
					}
					else
					{
						is_erase = true;
					}
				}
				else
				{
					is_erase = true;
				}

				if (is_erase)
				{
					voice_handle_mgr_.unmap(voice.handle);
					voice.is_active = false;
					continue;
				}
			}
		}
	}

	const auto max_mix_sample_it = std::max_element(
		mix_buffer_.cbegin(),
		mix_buffer_.cend(),
		[](const auto lhs, const auto rhs)
		{
			return std::abs(lhs) < std::abs(rhs);
		});

	if (max_mix_sample_it != mix_buffer_.cend())
	{
		constexpr auto max_mix_sample_value = 1.0F;

		const auto max_mix_sample = std::abs(*max_mix_sample_it);

		if (max_mix_sample <= max_mix_sample_value)
		{
			std::uninitialized_copy(
				mix_buffer_.cbegin(),
				mix_buffer_.cend(),
				buffer_.begin());
		}
		else
		{
			const auto scalar = 1.0F / max_mix_sample;

			std::transform(
				mix_buffer_.cbegin(),
				mix_buffer_.cend(),
				buffer_.begin(),
				[scalar](const auto item)
				{
					return static_cast<Sample>(item * scalar);
				});
		}
	}
}

void SdlAudioMixer::handle_set_mute_command(const SetMuteCommandParam& param) noexcept
{
	is_mute_ = param.is_mute;
}

void SdlAudioMixer::handle_set_distance_model_command(const SetDistanceModelCommandParam& param) noexcept
{
	if (distance_model_ == param.distance_model)
	{
		return;
	}

	is_distance_model_changed_ = true;
	distance_model_ = param.distance_model;
}

void SdlAudioMixer::handle_set_listener_meters_per_unit_command(const SetListenerMetersPerUnitCommandParam& param) noexcept
{
	if (listener_meters_per_unit_ != param.meters_per_unit)
	{
		is_meters_per_unit_changed_ = true;
		listener_meters_per_unit_ = param.meters_per_unit;
	}
}

void SdlAudioMixer::handle_set_listener_r3_position_command(const SetListenerR3PositionCommandParam& param) noexcept
{
	if (listener_r3_position_ != param.r3_position)
	{
		is_listener_r3_position_changed_ = true;
		listener_r3_position_ = param.r3_position;
	}
}

void SdlAudioMixer::handle_set_listener_r3_orientation_command(const SetListenerR3OrientationCommandParam& param) noexcept
{
	if (listener_r3_orientation_ != param.r3_orientation)
	{
		is_listener_r3_orientation_changed_ = true;
		listener_r3_orientation_ = param.r3_orientation;
	}
}

void SdlAudioMixer::handle_pause_voice_command(const PauseVoiceCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (!voice)
	{
		return;
	}

	voice->is_paused = true;
}

void SdlAudioMixer::handle_resume_voice_command(const ResumeVoiceCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (!voice)
	{
		return;
	}

	voice->is_paused = false;
}

void SdlAudioMixer::handle_stop_voice_command(const StopVoiceCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice_and_invalidate(param.handle);

	if (!voice)
	{
		return;
	}

	voice->is_active = false;
}

void SdlAudioMixer::handle_set_voice_gain_command(const SetVoiceGainCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (!voice)
	{
		return;
	}

	voice->gain = param.gain;
}

void SdlAudioMixer::handle_set_voice_r3_attenuation_command(const SetVoiceR3AttenuationCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (!voice ||
		!voice->is_r3 ||
		voice->r3_attenuation == param.attributes)
	{
		return;
	}

	voice->is_r3_attenuation_changed = true;
	voice->r3_attenuation = param.attributes;
	voice->r3_attenuation_cache = voice->r3_attenuation;
}

void SdlAudioMixer::handle_set_voice_r3_position_command(const SetVoiceR3PositionCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (!voice ||
		!voice->is_r3 ||
		voice->r3_position == param.position)
	{
		return;
	}

	voice->r3_position = param.position;
	voice->is_r3_position_changed = true;
}

void SdlAudioMixer::handle_commands()
{
	if (is_state_suspended_.load(std::memory_order_acquire))
	{
		return;
	}

	{
		MtLockGuard guard_lock{mt_commands_lock_};

		if (!mt_commands_.empty())
		{
			if (commands_.empty())
			{
				commands_ = mt_commands_;
			}
			else
			{
				commands_.insert(commands_.cend(), mt_commands_.cbegin(), mt_commands_.cend());
			}

			mt_commands_.clear();
		}
	}

	if (commands_.empty())
	{
		return;
	}

	for (const auto& command : commands_)
	{
		switch (command.type)
		{
			case CommandType::play_sound:
				handle_play_sound_command(command);
				break;

			case CommandType::set_mute:
				handle_set_mute_command(command.param.set_mute);
				break;

			case CommandType::set_distance_model:
				handle_set_distance_model_command(command.param.set_distance_model);
				break;

			case CommandType::set_listener_meters_per_unit:
				handle_set_listener_meters_per_unit_command(command.param.set_listener_meters_per_unit);
				break;

			case CommandType::set_listener_r3_position:
				handle_set_listener_r3_position_command(command.param.set_listener_r3_position);
				break;

			case CommandType::set_listener_r3_orientation:
				handle_set_listener_r3_orientation_command(command.param.set_listener_r3_orientation);
				break;

			case CommandType::pause_voice:
				handle_pause_voice_command(command.param.pause_voice);
				break;

			case CommandType::resume_voice:
				handle_resume_voice_command(command.param.resume_voice);
				break;

			case CommandType::stop_voice:
				handle_stop_voice_command(command.param.stop_voice);
				break;

			case CommandType::set_voice_gain:
				handle_set_voice_gain_command(command.param.set_voice_gain);
				break;

			case CommandType::set_voice_r3_attenuation:
				handle_set_voice_r3_attenuation_command(command.param.set_voice_r3_attenuation);
				break;

			case CommandType::set_voice_r3_position:
				handle_set_voice_r3_position_command(command.param.set_voice_r3_position);
				break;

			default:
				assert(false && "Unknown command.");
				break;
		}
	}

	commands_.clear();
}

void SdlAudioMixer::handle_play_sound_command(const Command& command)
{
	auto is_started = false;
	auto voice = static_cast<Voice*>(nullptr);

	const auto voice_handle_guard = ScopeGuard{
		[this, &is_started, &voice, &command]()
		{
			if (is_started && voice)
			{
				voice_handle_mgr_.uncache_and_map(voice->handle, voice);
			}
			else
			{
				voice_handle_mgr_.uncache(command.param.play_sound.handle);
			}
		}
	};

	auto cache_item = command.param.play_sound.cache;

	if (!cache_item)
	{
		return;
	}

	if (!initialize_cache_item(command, *cache_item))
	{
		return;
	}

	for (auto& i : voices_)
	{
		if (!i.is_active)
		{
			voice = &i;
			break;
		}
	}

	if (!voice)
	{
		return;
	}

	const auto& play_sound_param = command.param.play_sound;
	voice->type = play_sound_param.sound_type;
	voice->is_r3 = play_sound_param.is_r3;
	voice->is_looping = play_sound_param.is_looping;
	voice->is_paused = false;
	voice->is_r3_attenuation_changed = voice->is_r3;
	voice->is_r3_position_changed = voice->is_r3;
	voice->cache = play_sound_param.cache;
	voice->decode_offset = 0;
	voice->gain = audio_mixer_default_gain;
	voice->left_gain = 0.5 * voice->gain;
	voice->right_gain = 0.5 * voice->gain;
	voice->handle = play_sound_param.handle;
	voice->r3_attenuation = audio_mixer_make_default_voice_attenuation();
	voice->r3_attenuation_cache = voice->r3_attenuation;
	voice->r3_position = audio_mixer_make_default_voice_r3_position();
	voice->r3_position_cache = voice->r3_position;
	voice->is_active = true;

	is_started = true;
}

bool SdlAudioMixer::initialize_digitized_cache_item(const Command& command, CacheItem& cache_item)
{
	assert(!cache_item.is_active);
	assert(command.param.play_sound.sound_type == SoundType::pcm);

	const auto sample_count = calculate_digitized_sample_count(dst_rate_, command.param.play_sound.data_size);

	cache_item.is_active = true;
	cache_item.sound_type = command.param.play_sound.sound_type;
	cache_item.samples_count = sample_count;
	cache_item.samples.resize(sample_count);
	cache_item.digitized_resampler_counter = dst_rate_;
	cache_item.digitized_data = static_cast<const std::uint8_t*>(command.param.play_sound.data);
	cache_item.digitized_data_size = command.param.play_sound.data_size;

	return true;
}

bool SdlAudioMixer::initialize_cache_item(const Command& command, CacheItem& cache_item)
{
	const auto is_adlib_music = (command.param.play_sound.sound_type == SoundType::adlib_music);

	if (cache_item.is_active)
	{
		if (!is_adlib_music)
		{
			return !cache_item.is_invalid;
		}
	}

	if (command.param.play_sound.sound_type == SoundType::pcm)
	{
		return initialize_digitized_cache_item(command, cache_item);
	}

	cache_item = CacheItem{};
	cache_item.is_invalid = true;

	auto decoder = create_decoder_by_sound_type(command.param.play_sound.sound_type);

	if (!decoder)
	{
		return false;
	}

	auto param = AudioDecoderInitParam{};
	param.src_raw_data_ = command.param.play_sound.data;
	param.src_raw_size_ = command.param.play_sound.data_size;
	param.dst_rate_ = dst_rate_;

	if (!decoder->initialize(param))
	{
		return false;
	}

	const auto samples_count = decoder->get_dst_length_in_samples();

	if (samples_count <= 0)
	{
		return false;
	}

	cache_item.is_active = true;
	cache_item.is_invalid = false;
	cache_item.sound_type = command.param.play_sound.sound_type;
	cache_item.samples_count = samples_count;
	cache_item.samples.resize(is_adlib_music ? mix_samples_count_ : samples_count);
	cache_item.buffer_size_ = 0;
	cache_item.decoder.swap(decoder);

	return true;
}

bool SdlAudioMixer::decode_digitized_voice(const Voice& voice)
{
	auto cache_item = voice.cache;

	assert(cache_item);
	assert(cache_item->is_active);
	assert(!cache_item->is_invalid);
	assert(!cache_item->is_decoded());
	assert(voice.type == SoundType::pcm);

	auto to_decode_count = std::min(cache_item->samples_count - cache_item->decoded_count, mix_samples_count_);

	for (auto i = 0; i < to_decode_count; ++i)
	{
		if (cache_item->digitized_resampler_counter >= dst_rate_)
		{
			cache_item->digitized_resampler_counter -= dst_rate_;

			if (cache_item->digitized_data_offset < cache_item->digitized_data_size)
			{
				const auto u8_sample = cache_item->digitized_data[cache_item->digitized_data_offset];
				const auto f32_sample = AudioSampleConverter::u8_to_f32(u8_sample);
				cache_item->digitized_last_sample = f32_sample;

				cache_item->digitized_data_offset += 1;
			}
		}

		cache_item->samples[cache_item->decoded_count] = cache_item->digitized_last_sample;
		cache_item->decoded_count += 1;
		cache_item->digitized_resampler_counter += audio_decoder_w3d_pcm_frequency;
	}

	return true;
}

bool SdlAudioMixer::decode_voice(const Voice& voice)
{
	auto cache_item = voice.cache;

	if (!cache_item)
	{
		return false;
	}

	if (!cache_item->is_active)
	{
		return false;
	}

	if (cache_item->is_invalid)
	{
		return false;
	}

	if (cache_item->is_decoded())
	{
		return true;
	}

	if (voice.type == SoundType::pcm)
	{
		return decode_digitized_voice(voice);
	}

	if (voice.type == SoundType::adlib_music)
	{
		const auto total_remain_count = cache_item->samples_count - cache_item->decoded_count;

		if (total_remain_count == 0)
		{
			return true;
		}

		auto remain_count = std::min(total_remain_count, cache_item->buffer_size_);

		if (remain_count == 0)
		{
			remain_count = std::min(total_remain_count, mix_samples_count_);
		}

		cache_item->buffer_size_ = cache_item->decoder->decode(remain_count, s16_samples_.data());

		if (cache_item->buffer_size_ > 0)
		{
			std::transform(
				s16_samples_.cbegin(),
				s16_samples_.cbegin() + cache_item->buffer_size_,
				cache_item->samples.begin(),
				AudioSampleConverter::s16_to_f32);

			cache_item->decoded_count += cache_item->buffer_size_;
		}

		return true;
	}

	const auto ahead_count = std::min(voice.decode_offset + mix_samples_count_, cache_item->samples_count);

	if (ahead_count <= cache_item->decoded_count)
	{
		return true;
	}

	const auto planned_count = std::min(cache_item->samples_count - cache_item->decoded_count, mix_samples_count_);
	const auto actual_count = cache_item->decoder->decode(planned_count, s16_samples_.data());

	if (actual_count > 0)
	{
		std::transform(
			s16_samples_.cbegin(),
			s16_samples_.cbegin() + actual_count,
			cache_item->samples.begin() + cache_item->decoded_count,
			AudioSampleConverter::s16_to_f32);

		cache_item->decoded_count += actual_count;
	}

	return true;
}

void SdlAudioMixer::spatialize_voice(Voice& voice)
{
	if (!voice.is_active || !voice.is_r3)
	{
		return;
	}

	if (!is_distance_model_changed_ &&
		!is_meters_per_unit_changed_ &&
		!is_listener_r3_position_changed_ &&
		!is_listener_r3_orientation_changed_ &&
		!voice.is_r3_position_changed &&
		!voice.is_r3_attenuation_changed)
	{
		return;
	}

	if (voice.is_r3_attenuation_changed || voice.is_r3_position_changed || is_meters_per_unit_changed_)
	{
		voice.r3_position_cache = voice.r3_position * listener_meters_per_unit_;
	}

	voice.is_r3_attenuation_changed = false;
	voice.is_r3_position_changed = false;

	AudioMixerUtils::spatialize_voice_2_0(
		distance_model_,
		listener_r3_position_cache_,
		listener_r3_orientation_cache_,
		voice.r3_attenuation_cache,
		voice.r3_position_cache,
		voice.left_gain,
		voice.right_gain);
}

void SdlAudioMixer::spatialize_voices()
{
	if (is_meters_per_unit_changed_ || is_listener_r3_position_changed_)
	{
		listener_r3_position_cache_ = listener_r3_position_ * listener_meters_per_unit_;
	}

	if (is_listener_r3_orientation_changed_)
	{
		listener_r3_orientation_cache_.at = AudioMixerUtils::normalize(listener_r3_orientation_.at);
		listener_r3_orientation_cache_.up = AudioMixerUtils::normalize(listener_r3_orientation_.up);
	}

	for (auto& voice : voices_)
	{
		spatialize_voice(voice);
	}

	is_distance_model_changed_ = false;
	is_meters_per_unit_changed_ = false;
	is_listener_r3_position_changed_ = false;
	is_listener_r3_orientation_changed_ = false;
}

AudioMixerVoiceHandle SdlAudioMixer::play_sound(const AudioMixerPlaySoundParam& param)
try
{
	if (!is_sound_type_valid(param.sound_type))
	{
		fail("Invalid sound type.");
	}

	if (!param.data)
	{
		fail("Null data.");
	}

	if (param.data_size <= 0)
	{
		fail("Data size out of range.");
	}

	const auto voice_handle = voice_handle_mgr_.generate();

	auto command = Command{};
	command.type = CommandType::play_sound;
	command.param.play_sound.sound_type = param.sound_type;
	command.param.play_sound.is_r3 = param.is_r3;
	command.param.play_sound.is_looping = param.is_looping;
	command.param.play_sound.cache = get_cache_item(param.sound_type, param.sound_index);
	command.param.play_sound.handle = voice_handle;
	command.param.play_sound.data = param.data;
	command.param.play_sound.data_size = param.data_size;

	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
	voice_handle_mgr_.cache(voice_handle);
	return voice_handle;
}
catch (...)
{
	fail_nested(__func__);
}

void SdlAudioMixer::lock()
{
	SDL_LockAudioDevice(sdl_audio_device_.get());
}

void SdlAudioMixer::unlock()
{
	SDL_UnlockAudioDevice(sdl_audio_device_.get());
}

void SdlAudioMixer::callback_proxy(void* user_data, std::uint8_t* dst_data, int dst_length)
try
{
	assert(user_data);
	auto mixer = static_cast<SdlAudioMixer*>(user_data);
	mixer->mix();
	mixer->callback(dst_data, dst_length);
}
catch (...)
{
}

int SdlAudioMixer::calculate_mix_samples_count(int dst_rate, int mix_size_ms)
{
	const auto exact_count = (dst_rate * mix_size_ms) / 1000;
	auto actual_count = 1;

	while (actual_count < exact_count)
	{
		actual_count *= 2;
	}

	// Maximum power-of-two value for 16-bit unsigned type is 2^15 (32'768).
	if (actual_count > 32'768)
	{
		actual_count = 32'768;
	}

	return actual_count;
}

SdlAudioMixer::CacheItem* SdlAudioMixer::get_cache_item(SoundType sound_type, int sound_index)
{
	if (!is_sound_index_valid(sound_index, sound_type))
	{
		return nullptr;
	}

	switch (sound_type)
	{
		case SoundType::adlib_music: return &adlib_music_cache_[sound_index];
		case SoundType::adlib_sfx: return &adlib_sfx_cache_[sound_index];
		case SoundType::pc_speaker_sfx: return &pc_speaker_sfx_cache_[sound_index];
		case SoundType::pcm: return &pcm_cache_[sound_index];
		default: return nullptr;
	}
}

AudioDecoderUPtr SdlAudioMixer::create_decoder_by_sound_type(SoundType sound_type) const
{
	switch (sound_type)
	{
		case SoundType::adlib_music:
			return make_audio_decoder(AudioDecoderType::adlib_music, opl3_type_);

		case SoundType::adlib_sfx:
			return make_audio_decoder(AudioDecoderType::adlib_sfx, opl3_type_);

		case SoundType::pc_speaker_sfx:
			return make_audio_decoder(AudioDecoderType::pc_speaker, opl3_type_);

		default:
			return nullptr;
	}
}

bool SdlAudioMixer::is_sound_type_valid(SoundType sound_type)
{
	switch (sound_type)
	{
		case SoundType::adlib_music:
		case SoundType::adlib_sfx:
		case SoundType::pc_speaker_sfx:
		case SoundType::pcm:
			return true;

		default:
			return false;
	}
}

bool SdlAudioMixer::is_sound_index_valid(int sound_index, SoundType sound_type)
{
	switch (sound_type)
	{
		case SoundType::adlib_music:
			return sound_index >= 0 && sound_index < LASTMUSIC;

		case SoundType::adlib_sfx:
		case SoundType::pc_speaker_sfx:
		case SoundType::pcm:
			return sound_index >= 0 && sound_index < NUMSOUNDS;

		default:
			return false;
	}
}

int SdlAudioMixer::calculate_digitized_sample_count(int dst_sample_rate, int digitized_byte_count) noexcept
{
	assert(dst_sample_rate >= 0);
	assert(digitized_byte_count >= 0);
	assert(audio_decoder_w3d_pcm_frequency <= dst_sample_rate);

	const auto src_sample_rate = audio_decoder_w3d_pcm_frequency;
	const auto sample_count = ((digitized_byte_count * dst_sample_rate) + src_sample_rate - 1) / src_sample_rate;
	return sample_count;
}

} // bstone
