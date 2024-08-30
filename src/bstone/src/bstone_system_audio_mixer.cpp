/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cmath>
#include <algorithm>
#include <mutex>
#include "bstone_assert.h"
#include "bstone_audio_decoder.h"
#include "bstone_audio_mixer_utils.h"
#include "bstone_audio_mixer_validator.h"
#include "bstone_audio_sample_converter.h"
#include "bstone_exception.h"
#include "bstone_globals.h"
#include "bstone_scope_exit.h"
#include "bstone_system_audio_mixer.h"

namespace bstone {

void SystemAudioMixer::SysCallback::set_mixer(SystemAudioMixer* mixer)
{
	if (mixer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null mixer.");
	}

	mixer_ = mixer;
}

void SystemAudioMixer::SysCallback::do_invoke(float* samples, int sample_count)
{
	if (mixer_ == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null mixer.");
	}

	mixer_->mix();
	mixer_->callback(samples, sample_count);
}

SystemAudioMixer::CacheItem::CacheItem()
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

bool SystemAudioMixer::CacheItem::is_decoded() const noexcept
{
	return decoded_count == samples_count;
}

SystemAudioMixer::SystemAudioMixer(const AudioMixerInitParam& param)
try {
	if (param.max_voices < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Max voice count out of range.");
	}

	switch (param.opl3_type)
	{
		case Opl3Type::dbopl:
		case Opl3Type::nuked:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown OPL3 type.");
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

	if (bstone::globals::sys_system_mgr == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null system manager.");
	}

	mix_samples_count_ = calculate_mix_samples_count(dst_rate_, mix_size_ms_);

	sys_audio_mgr_ = &bstone::globals::sys_system_mgr->get_audio_mgr();

	if (!sys_audio_mgr_->is_initialized())
	{
		BSTONE_THROW_STATIC_SOURCE("Audio system not available.");
	}

	sys_callback_.set_mixer(this);

	auto audio_device_param = bstone::sys::PollingAudioDeviceOpenParam{};
	audio_device_param.desired_rate = dst_rate_;
	audio_device_param.channel_count = get_max_channels();
	audio_device_param.desired_frame_count = mix_samples_count_;
	audio_device_param.callback = &sys_callback_;

	auto audio_device = sys_audio_mgr_->make_polling_audio_device(audio_device_param);

	dst_rate_ = audio_device->get_rate();
	mix_samples_count_ = audio_device->get_frame_count();

	opl3_type_ = param.opl3_type;
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

	initialize_is_mute();
	initialize_gain();
	initialize_listener_r3_position();
	initialize_listener_r3_orientation();
	initialize_voice_handles();
	initialize_voices(param.max_voices);

	audio_device->pause(false);

	sys_audio_device_.swap(audio_device);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SystemAudioMixer::~SystemAudioMixer() = default;

Opl3Type SystemAudioMixer::get_opl3_type() const
{
	return opl3_type_;
}

int SystemAudioMixer::get_rate() const
{
	return dst_rate_;
}

int SystemAudioMixer::get_channel_count() const
{
	return get_max_channels();
}

int SystemAudioMixer::get_mix_size_ms() const
{
	return mix_size_ms_;
}

void SystemAudioMixer::set_mute(bool is_mute)
try {
	auto command = Command{};
	command.type = CommandType::set_mute;
	command.param.set_mute.is_mute = is_mute;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_gain(double gain)
try {
	AudioMixerValidator::validate_gain(gain);

	auto command = Command{};
	command.type = CommandType::set_gain;
	command.param.set_gain.gain = gain;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int SystemAudioMixer::get_min_rate() const noexcept
{
	return 11'025;
}

int SystemAudioMixer::get_default_rate() const noexcept
{
	return 44'100;
}

int SystemAudioMixer::get_min_mix_size_ms() const noexcept
{
	return 20;
}

int SystemAudioMixer::get_default_mix_size_ms() const noexcept
{
	return 40;
}

int SystemAudioMixer::get_max_channels() const noexcept
{
	return 2;
}

void SystemAudioMixer::suspend_state()
try {
	is_state_suspended_.store(true, std::memory_order_release);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::resume_state()
try {
	is_state_suspended_.store(false, std::memory_order_release);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_listener_r3_position(const AudioMixerListenerR3Position& r3_position)
try {
	auto command = Command{};
	command.type = CommandType::set_listener_r3_position;
	command.param.set_listener_r3_position.r3_position = r3_position;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_listener_r3_orientation(const AudioMixerListenerR3Orientation& r3_orientation)
try {
	auto command = Command{};
	command.type = CommandType::set_listener_r3_orientation;
	command.param.set_listener_r3_orientation.r3_orientation = r3_orientation;
	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool SystemAudioMixer::is_voice_playing(AudioMixerVoiceHandle voice_handle) const
try {
	return voice_handle_mgr_.is_valid_handle(voice_handle);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::pause_voice(AudioMixerVoiceHandle voice_handle)
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

void SystemAudioMixer::resume_voice(AudioMixerVoiceHandle voice_handle)
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

void SystemAudioMixer::stop_voice(AudioMixerVoiceHandle voice_handle)
try {
	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::stop_voice;
	command.param.stop_voice.handle = voice_handle;

	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_voice_gain(AudioMixerVoiceHandle voice_handle, double gain)
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_voice_r3_position(
	AudioMixerVoiceHandle voice_handle,
	const AudioMixerVoiceR3Position& r3_position)
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool SystemAudioMixer::can_set_voice_output_gains() const
{
	return true;
}

void SystemAudioMixer::enable_set_voice_output_gains(
	AudioMixerVoiceHandle voice_handle,
	bool is_enable)
try {
	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::enable_set_voice_output_gains;
	command.param.enable_set_voice_output_gains.handle = voice_handle;
	command.param.enable_set_voice_output_gains.is_enable = is_enable;

	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::set_voice_output_gains(
	AudioMixerVoiceHandle voice_handle,
	AudioMixerOutputGains& output_gains)
try {
	if (!voice_handle.is_valid())
	{
		return;
	}

	AudioMixerValidator::validate_output_gains(output_gains);

	auto command = Command{};
	command.type = CommandType::set_voice_output_gains;
	command.param.set_voice_output_gains.handle = voice_handle;
	command.param.set_voice_output_gains.output_gains = output_gains;

	MtLockGuard guard_lock{mt_commands_lock_};
	mt_commands_.push_back(command);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SystemAudioMixer::initialize_is_mute()
{
	is_mute_ = false;
}

void SystemAudioMixer::initialize_gain()
{
	gain_ = audio_mixer_max_gain;
}

void SystemAudioMixer::initialize_listener_r3_position()
{
	is_listener_r3_position_changed_ = true;
	listener_r3_position_ = audio_mixer_make_default_listener_r3_position();
}

void SystemAudioMixer::initialize_listener_r3_orientation()
{
	is_listener_r3_orientation_changed_ = true;
	listener_r3_orientation_ = audio_mixer_make_default_listener_r3_orientation();
}

void SystemAudioMixer::initialize_voice_handles()
{
	const auto capacity = static_cast<int>(commands_.size());
	voice_handle_mgr_.set_cache_capacity(capacity);
	voice_handle_mgr_.set_map_capacity(capacity);
}

void SystemAudioMixer::initialize_voices(int max_voices)
{
	voices_.clear();
	voices_.resize(max_voices);
}

void SystemAudioMixer::callback(float* samples, int)
{
	std::copy(buffer_.cbegin(), buffer_.cend(), samples);
}

void SystemAudioMixer::mix()
{
	handle_commands();
	mix_samples();
}

void SystemAudioMixer::mix_samples()
{
	spatialize_voices();

	std::fill(buffer_.begin(), buffer_.end(), Sample{});
	std::fill(mix_buffer_.begin(), mix_buffer_.end(), MixSample{});

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

		auto gain_scale = (voice.is_custom_output_gains ? 1.0 : voice.gain);

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
			BSTONE_ASSERT(remain_count >= 0);
			decode_count = std::min(remain_count, mix_samples_count_);
		}

		if (!is_mute_)
		{
			auto& mix_buffer = mix_buffer_;
			const auto base_offset = (is_adlib_music ? 0 : voice.decode_offset);
			const auto& gains = (voice.is_custom_output_gains ? voice.custom_output_gains : voice.output_gains);

			for (int i = 0; i < decode_count; ++i)
			{
				const auto sample = gain_scale * cache_item->samples[base_offset + i];
				const auto left_sample = static_cast<Sample>(gains[0] * sample);
				const auto right_sample = static_cast<Sample>(gains[1] * sample);

				mix_buffer[(2 * i) + 0] += left_sample;
				mix_buffer[(2 * i) + 1] += right_sample;
			}
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

	if (!is_mute_)
	{
		const auto max_mix_sample_it = std::max_element(
			mix_buffer_.cbegin(),
			mix_buffer_.cend(),
			[](const auto lhs, const auto rhs)
			{
				return std::abs(lhs) < std::abs(rhs);
			});

		if (max_mix_sample_it != mix_buffer_.cend())
		{
			const auto max_mix_sample_value = gain_;
			const auto max_mix_sample = std::abs(*max_mix_sample_it);

			if (max_mix_sample <= max_mix_sample_value)
			{
				std::copy(
					mix_buffer_.cbegin(),
					mix_buffer_.cend(),
					buffer_.begin());
			}
			else
			{
				const auto scalar = max_mix_sample_value / max_mix_sample;

				std::transform(
					mix_buffer_.cbegin(),
					mix_buffer_.cend(),
					buffer_.begin(),
					[scalar](const auto& item)
					{
						return static_cast<Sample>(item * scalar);
					});
			}
		}
	}
}

void SystemAudioMixer::handle_set_mute_command(const SetMuteCommandParam& param) noexcept
{
	is_mute_ = param.is_mute;
}

void SystemAudioMixer::handle_set_gain_command(const SetGainCommandParam& param) noexcept
{
	gain_ = param.gain;
}

void SystemAudioMixer::handle_set_listener_r3_position_command(
	const SetListenerR3PositionCommandParam& param) noexcept
{
	if (listener_r3_position_ != param.r3_position)
	{
		is_listener_r3_position_changed_ = true;
		listener_r3_position_ = param.r3_position;
	}
}

void SystemAudioMixer::handle_set_listener_r3_orientation_command(
	const SetListenerR3OrientationCommandParam& param) noexcept
{
	if (listener_r3_orientation_ != param.r3_orientation)
	{
		is_listener_r3_orientation_changed_ = true;
		listener_r3_orientation_ = param.r3_orientation;
	}
}

void SystemAudioMixer::handle_pause_voice_command(const PauseVoiceCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (!voice)
	{
		return;
	}

	voice->is_paused = true;
}

void SystemAudioMixer::handle_resume_voice_command(const ResumeVoiceCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (!voice)
	{
		return;
	}

	voice->is_paused = false;
}

void SystemAudioMixer::handle_stop_voice_command(const StopVoiceCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice_and_invalidate(param.handle);

	if (!voice)
	{
		return;
	}

	voice->is_active = false;
}

void SystemAudioMixer::handle_set_voice_gain_command(const SetVoiceGainCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (!voice)
	{
		return;
	}

	voice->gain = param.gain;
}

void SystemAudioMixer::handle_set_voice_r3_position_command(const SetVoiceR3PositionCommandParam& param)
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

void SystemAudioMixer::handle_enable_set_voice_output_gains_command(
	const EnableSetVoiceOutputGainsCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (voice == nullptr)
	{
		return;
	}

	voice->is_custom_output_gains = param.is_enable;
}

void SystemAudioMixer::handle_set_voice_output_gains_command(const SetVoiceOutputGainsCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (voice == nullptr)
	{
		return;
	}

	voice->custom_output_gains = param.output_gains;
}

void SystemAudioMixer::handle_commands()
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

			case CommandType::set_gain:
				handle_set_gain_command(command.param.set_gain);
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

			case CommandType::set_voice_r3_position:
				handle_set_voice_r3_position_command(command.param.set_voice_r3_position);
				break;

			case CommandType::enable_set_voice_output_gains:
				handle_enable_set_voice_output_gains_command(command.param.enable_set_voice_output_gains);
				break;

			case CommandType::set_voice_output_gains:
				handle_set_voice_output_gains_command(command.param.set_voice_output_gains);
				break;

			default:
				BSTONE_ASSERT(false && "Unknown command.");
				break;
		}
	}

	commands_.clear();
}

void SystemAudioMixer::handle_play_sound_command(const Command& command)
{
	auto is_started = false;
	auto voice = static_cast<Voice*>(nullptr);

	const auto voice_handle_guard = make_scope_exit(
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
		});

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
	voice->is_r3_position_changed = voice->is_r3;
	voice->is_custom_output_gains = false;
	voice->cache = play_sound_param.cache;
	voice->decode_offset = 0;
	voice->gain = audio_mixer_default_gain;
	voice->output_gains.fill(audio_mixer_max_gain * 0.5 * voice->gain);
	voice->custom_output_gains.fill(audio_mixer_max_gain * 0.5);
	voice->handle = play_sound_param.handle;
	voice->r3_position = audio_mixer_make_default_voice_r3_position();
	voice->r3_position_cache = voice->r3_position;
	voice->is_active = true;

	is_started = true;
}

bool SystemAudioMixer::initialize_digitized_cache_item(const Command& command, CacheItem& cache_item)
{
	BSTONE_ASSERT(!cache_item.is_active);
	BSTONE_ASSERT(command.param.play_sound.sound_type == SoundType::pcm);

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

bool SystemAudioMixer::initialize_cache_item(const Command& command, CacheItem& cache_item)
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

bool SystemAudioMixer::decode_digitized_voice(const Voice& voice)
{
	auto cache_item = voice.cache;

	BSTONE_ASSERT(cache_item);
	BSTONE_ASSERT(cache_item->is_active);
	BSTONE_ASSERT(!cache_item->is_invalid);
	BSTONE_ASSERT(!cache_item->is_decoded());
	BSTONE_ASSERT(voice.type == SoundType::pcm);

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

bool SystemAudioMixer::decode_voice(const Voice& voice)
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

void SystemAudioMixer::spatialize_voice(Voice& voice)
{
	if (!voice.is_active || !voice.is_r3 || voice.is_custom_output_gains)
	{
		return;
	}

	if (!is_listener_r3_position_changed_ &&
		!is_listener_r3_orientation_changed_ &&
		!voice.is_r3_position_changed)
	{
		return;
	}

	if (voice.is_r3_position_changed)
	{
		voice.r3_position_cache = voice.r3_position;
	}

	voice.is_r3_position_changed = false;

	AudioMixerUtils::spatialize_voice_2_0(
		listener_r3_position_cache_,
		listener_r3_orientation_cache_,
		voice.r3_position_cache,
		voice.output_gains[0],
		voice.output_gains[1]);
}

void SystemAudioMixer::spatialize_voices()
{
	if (is_listener_r3_position_changed_)
	{
		listener_r3_position_cache_ = listener_r3_position_;
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

	is_listener_r3_position_changed_ = false;
	is_listener_r3_orientation_changed_ = false;
}

AudioMixerVoiceHandle SystemAudioMixer::play_sound(const AudioMixerPlaySoundParam& param)
try {
	if (!is_sound_type_valid(param.sound_type))
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid sound type.");
	}

	if (!param.data)
	{
		BSTONE_THROW_STATIC_SOURCE("Null data.");
	}

	if (param.data_size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Data size out of range.");
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int SystemAudioMixer::calculate_mix_samples_count(int dst_rate, int mix_size_ms)
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

SystemAudioMixer::CacheItem* SystemAudioMixer::get_cache_item(SoundType sound_type, int sound_index)
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

AudioDecoderUPtr SystemAudioMixer::create_decoder_by_sound_type(SoundType sound_type) const
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

bool SystemAudioMixer::is_sound_type_valid(SoundType sound_type)
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

bool SystemAudioMixer::is_sound_index_valid(int sound_index, SoundType sound_type)
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

int SystemAudioMixer::calculate_digitized_sample_count(int dst_sample_rate, int digitized_byte_count) noexcept
{
	BSTONE_ASSERT(dst_sample_rate >= 0);
	BSTONE_ASSERT(digitized_byte_count >= 0);
	BSTONE_ASSERT(audio_decoder_w3d_pcm_frequency <= dst_sample_rate);

	const auto src_sample_rate = audio_decoder_w3d_pcm_frequency;
	const auto sample_count = ((digitized_byte_count * dst_sample_rate) + src_sample_rate - 1) / src_sample_rate;
	return sample_count;
}

} // namespace bstone
