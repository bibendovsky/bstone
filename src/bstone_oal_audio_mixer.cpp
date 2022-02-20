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
#include <cfloat>
#include <array>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_set>
#include <vector>
#include "3d_def.h"
#include "audio.h"
#include "id_sd.h"
#include "bstone_audio_decoder.h"
#include "bstone_audio_mixer_validator.h"
#include "bstone_exception.h"
#include "bstone_logger.h"
#include "bstone_oal_source.h"
#include "bstone_oal_loader.h"
#include "bstone_oal_resource.h"
#include "bstone_scope_guard.h"

namespace bstone
{

class OalAudioMixerException : public Exception
{
public:
	explicit OalAudioMixerException(const char* message) noexcept
		:
		Exception{"OAL_AUDIO_MIXER", message}
	{
	}
}; // OalAudioMixerException

// ==========================================================================

OalAudioMixer::OalAudioMixer(const AudioMixerInitParam& param)
try
{
	switch (param.opl3_type)
	{
		case Opl3Type::dbopl:
		case Opl3Type::nuked:
			break;

		default:
			fail("Unknown OPL3 type.");
	}

	initialize_oal(param);

	opl3_type_ = param.opl3_type;

	if (param.mix_size_ms < get_min_mix_size_ms())
	{
		mix_size_ms_ = get_min_mix_size_ms();
	}
	else if (param.mix_size_ms < get_default_mix_size_ms())
	{
		mix_size_ms_ = get_default_mix_size_ms();
	}
	else
	{
		mix_size_ms_ = param.mix_size_ms;
	}

	mix_sample_count_ = static_cast<int>((dst_rate_ * mix_size_ms_) / 1'000L);

	initialize_distance_model();
	initialize_listener_meters_per_unit();
	initialize_listener_r3_position();
	initialize_listener_r3_orientation();
	initialize_voice_handles();
	initialize_voices();
	initialize_command_queue();

	initialize_music();
	initialize_sfx();
	initialize_thread();

	is_mute_ = false;
}
catch (...)
{
	fail_nested(__func__);
}

OalAudioMixer::~OalAudioMixer()
{
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

void OalAudioMixer::set_mute(bool is_mute)
try
{
	auto command = Command{};
	command.type = CommandType::set_mute;
	auto& command_param = command.param.set_mute;
	command_param.is_mute = is_mute;

	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

int OalAudioMixer::get_min_rate() const noexcept
{
	return 11'025;
}

int OalAudioMixer::get_min_mix_size_ms() const noexcept
{
	return min_mix_size_ms;
}

int OalAudioMixer::get_default_mix_size_ms() const noexcept
{
	return max_mix_size_ms;
}

int OalAudioMixer::get_max_channels() const noexcept
{
	return 2;
}

void OalAudioMixer::suspend_state()
{
	is_state_suspended_.store(true, std::memory_order_release);
}

void OalAudioMixer::resume_state()
{
	is_state_suspended_.store(false, std::memory_order_release);
}

void OalAudioMixer::set_distance_model(AudioMixerDistanceModel distance_model)
try
{
	AudioMixerValidator::validate_distance_model(distance_model);
}
catch (...)
{
	fail_nested(__func__);
}

void OalAudioMixer::set_listener_meters_per_unit(double meters_per_unit)
try
{
	AudioMixerValidator::validate_listener_meters_per_unit(meters_per_unit);
	auto command = Command{};
	command.type = CommandType::set_listener_meters_per_unit;
	auto& command_param = command.param.set_listener_meters_per_unit;
	command_param.meters_per_unit = meters_per_unit;
	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void OalAudioMixer::set_listener_r3_position(const AudioMixerListenerR3Position& r3_position)
try
{
	auto command = Command{};
	command.type = CommandType::set_listener_r3_position;
	auto& command_param = command.param.set_listener_r3_position;
	command_param.r3_position = r3_position;
	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void OalAudioMixer::set_listener_r3_orientation(const AudioMixerListenerR3Orientation& r3_orientation)
try
{
	auto command = Command{};
	command.type = CommandType::set_listener_r3_orientation;
	auto& command_param = command.param.set_listener_r3_orientation;
	command_param.r3_orientation = r3_orientation;
	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

AudioMixerVoiceHandle OalAudioMixer::play_sound(const AudioMixerPlaySoundParam& param)
try
{
	auto is_music = false;

	switch (param.sound_type)
	{
		case SoundType::adlib_music:
			is_music = true;
			break;

		case SoundType::adlib_sfx:
		case SoundType::pc_speaker_sfx:
		case SoundType::pcm:
			break;

		default:
			fail("Unknown sound type.");
	}

	if (is_music)
	{
		auto voice_handle = AudioMixerVoiceHandle{};

		if (!param.data || param.data_size <= 0)
		{
			return voice_handle;
		}

		return play_adlib_music_internal(param.data, param.data_size, param.is_looping);
	}
	else
	{
		auto voice_handle = AudioMixerVoiceHandle{};

		if (param.sound_index < 0 || param.sound_index >= NUMSOUNDS)
		{
			return voice_handle;
		}

		if (!param.data || param.data_size <= 0)
		{
			return voice_handle;
		}

		return play_sfx_sound_internal(param.sound_type, param.sound_index, param.data, param.data_size, param.is_r3);
	}
}
catch (...)
{
	fail_nested(__func__);
}

bool OalAudioMixer::is_voice_playing(AudioMixerVoiceHandle voice_handle) const
try
{
	return voice_handle_mgr_.is_valid_handle(voice_handle);
}
catch (...)
{
	fail_nested(__func__);
}

void OalAudioMixer::pause_voice(AudioMixerVoiceHandle voice_handle)
try
{
	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::pause_voice;
	auto& command_param = command.param.pause_voice;
	command_param.handle = voice_handle;
	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void OalAudioMixer::resume_voice(AudioMixerVoiceHandle voice_handle)
try
{
	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::resume_voice;
	auto& command_param = command.param.resume_voice;
	command_param.handle = voice_handle;
	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void OalAudioMixer::stop_voice(AudioMixerVoiceHandle voice_handle)
try
{
	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::stop_voice;
	auto& command_param = command.param.stop_voice;
	command_param.handle = voice_handle;
	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void OalAudioMixer::set_voice_gain(AudioMixerVoiceHandle voice_handle, double gain)
try
{
	AudioMixerValidator::validate_gain(gain);

	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::set_voice_gain;
	auto& command_param = command.param.set_voice_gain;
	command_param.handle = voice_handle;
	command_param.gain = gain;
	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void OalAudioMixer::set_voice_r3_attenuation(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Attenuation& r3_attenuation)
try
{
	AudioMixerValidator::validate_voice_r3_attenuation(r3_attenuation);

	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::set_voice_r3_attenuation;
	auto& command_param = command.param.set_voice_r3_attenuation;
	command_param.handle = voice_handle;
	command_param.attributes = r3_attenuation;
	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

void OalAudioMixer::set_voice_r3_position(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Position& r3_position)
try
{
	if (!voice_handle.is_valid())
	{
		return;
	}

	auto command = Command{};
	command.type = CommandType::set_voice_r3_position;
	auto& command_param = command.param.set_voice_r3_position;
	command_param.handle = voice_handle;
	command_param.position = r3_position;
	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]] void OalAudioMixer::fail(const char* message)
{
	throw OalAudioMixerException{message};
}

[[noreturn]] void OalAudioMixer::fail_nested(const char* message)
{
	std::throw_with_nested(OalAudioMixerException{message});
}

void OalAudioMixer::make_al_context_current()
{
	const auto al_result = al_symbols_.alcMakeContextCurrent(oal_context_resource_.get());

	if (al_result == ALC_FALSE)
	{
		fail("Failed to make context current.");
	}
}

OalAudioMixer::OalString OalAudioMixer::get_default_alc_device_name()
{
	auto default_device_name = OalString{};

	if (!has_alc_enumeration_ext_ && !has_alc_enumerate_all_ext_)
	{
		return default_device_name;
	}

	const auto alc_enum = (has_alc_enumerate_all_ext_ ? ALC_DEFAULT_ALL_DEVICES_SPECIFIER : ALC_DEFAULT_DEVICE_SPECIFIER);
	const auto default_alc_device_name = al_symbols_.alcGetString(nullptr, alc_enum);

	if (default_alc_device_name)
	{
		default_device_name = default_alc_device_name;
	}

	return default_device_name;
}

OalAudioMixer::OalString OalAudioMixer::get_alc_device_name()
{
	auto device_name = OalString{};

	const auto alc_device_name = al_symbols_.alcGetString(oal_device_resource_.get(), ALC_DEVICE_SPECIFIER);

	if (alc_device_name)
	{
		device_name = alc_device_name;
	}

	return device_name;
}

OalAudioMixer::OalStrings OalAudioMixer::get_alc_device_names()
{
	auto device_names = OalStrings{};

	if (!has_alc_enumeration_ext_ && !has_alc_enumerate_all_ext_)
	{
		return device_names;
	}

	device_names.reserve(4);
	const auto alc_enum = (has_alc_enumerate_all_ext_ ? ALC_ALL_DEVICES_SPECIFIER : ALC_DEVICE_SPECIFIER);
	auto alc_device_names = al_symbols_.alcGetString(nullptr, alc_enum);

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

OalAudioMixer::OalStrings OalAudioMixer::parse_al_token_string(const char* al_token_string)
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
					break;

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
	const auto alc_extensions = al_symbols_.alcGetString(oal_device_resource_.get(), ALC_EXTENSIONS);
	const auto extensions = parse_al_token_string(alc_extensions);

	auto present_extensions = OalStrings{};
	present_extensions.reserve(extensions.size());

	for (const auto& extension : extensions)
	{
		const auto is_present = (al_symbols_.alcIsExtensionPresent(oal_device_resource_.get(), extension.c_str()) != ALC_FALSE);

		if (is_present)
		{
			present_extensions.emplace_back(extension);
		}
	}

	return present_extensions;
}

OalAudioMixer::OalStrings OalAudioMixer::get_al_extensions()
{
	const auto al_extensions = al_symbols_.alGetString(AL_EXTENSIONS);
	const auto extensions = parse_al_token_string(al_extensions);

	auto present_extensions = OalStrings{};
	present_extensions.reserve(extensions.size());

	for (const auto& extension : extensions)
	{
		const auto is_present = (al_symbols_.alIsExtensionPresent(extension.c_str()) != AL_FALSE);

		if (is_present)
		{
			present_extensions.emplace_back(extension);
		}
	}

	return present_extensions;
}

int OalAudioMixer::get_al_mixing_frequency()
{
	auto al_attribute_size = ALCint{};

	al_symbols_.alcGetIntegerv(oal_device_resource_.get(), ALC_ATTRIBUTES_SIZE, 1, &al_attribute_size);

	constexpr auto max_attributes = 64;
	constexpr auto max_al_attributes_size = (2 * (max_attributes - 1)) + 1;

	if (al_attribute_size <= 0 || al_attribute_size > max_al_attributes_size)
	{
		fail("Attributes size out of range.");
	}

	struct OalAttribute
	{
		ALint name;
		ALint value;
	}; // OalAttribute

	using OalAttributes = std::array<OalAttribute, max_attributes>;
	auto al_attributes = OalAttributes{};

	al_symbols_.alcGetIntegerv(oal_device_resource_.get(), ALC_ALL_ATTRIBUTES, max_al_attributes_size, reinterpret_cast<ALCint*>(al_attributes.data()));

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

	fail("No ALC_FREQUENCY attribute.");
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
			oal_source_resource = make_oal_source(al_symbols_);
			voice_count += 1;
		}
	}
	catch (...)
	{
	}

	return voice_count;
}

void OalAudioMixer::detect_alc_extensions()
{
	assert(al_symbols_.alcIsExtensionPresent);

	has_alc_enumeration_ext_ = (al_symbols_.alcIsExtensionPresent(nullptr, alc_enumeration_ext_str) != ALC_FALSE);
	has_alc_enumerate_all_ext_ = (al_symbols_.alcIsExtensionPresent(nullptr, alc_enumerate_all_ext_str) != ALC_FALSE);
}

void OalAudioMixer::detect_efx_features()
{
	log("Checking for EFX.");

	const auto has_efx = (al_symbols_.alcIsExtensionPresent(oal_device_resource_.get(), ALC_EXT_EFX_NAME) == ALC_TRUE);

	if (!has_efx)
	{
		log("No EFX extension.");
		return;
	}

	al_meters_per_unit_enum_ = al_symbols_.alcGetEnumValue(oal_device_resource_.get(), "AL_METERS_PER_UNIT");

	if (al_meters_per_unit_enum_ == AL_NONE)
	{
		log("No AL_METERS_PER_UNIT enum.");
		return;
	}

	try
	{
		oal_loader_->load_efx_symbols(al_symbols_);
	}
	catch (...)
	{
		log("Does not have all symbols.");
		return;
	}

	std::ignore = al_symbols_.alGetError();
	al_symbols_.alListenerf(AL_METERS_PER_UNIT, AL_DEFAULT_METERS_PER_UNIT);
	has_efx_meters_per_unit_ = (al_symbols_.alGetError() == AL_NO_ERROR);

	if (!has_efx_meters_per_unit_)
	{
		log("Does not support AL_METERS_PER_UNIT.");
	}
}

void OalAudioMixer::log(const OalString& string)
{
	static const auto prefix = OalString{"[SND_OAL] "};
	logger_->write(prefix + string);
}

void OalAudioMixer::log_oal_library_file_name()
{
	log(std::string{"Default library: \""} + get_oal_default_library_file_name() + '\"');
	log("Custom library: \"" + sd_oal_library + '\"');
}

void OalAudioMixer::log_oal_custom_device()
{
	log(std::string{} + "Custom device: \"" + sd_oal_device_name + '\"');
}

void OalAudioMixer::log_oal_devices()
{
	log("Available devices:");
	const auto device_names = get_alc_device_names();

	for (const auto& device_name : device_names)
	{
		log("\t\"" + device_name + '\"');
	}
}

void OalAudioMixer::log_oal_default_device()
{
	const auto default_device_name = get_default_alc_device_name();
	log("Default device: \"" + default_device_name + '\"');
}

void OalAudioMixer::log_oal_current_device_name()
{
	const auto current_device_name = get_alc_device_name();
	log("Current device: \"" + current_device_name + '\"');
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
	const auto al_version = al_symbols_.alGetString(AL_VERSION);
	const auto version = OalString(al_version ? al_version : "");
	log("Version: " + version);

	const auto al_renderer = al_symbols_.alGetString(AL_RENDERER);
	const auto renderer = OalString(al_renderer ? al_renderer : "");
	log("Renderer: " + renderer);

	const auto al_vendor = al_symbols_.alGetString(AL_VENDOR);
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

const char* OalAudioMixer::get_oal_default_library_file_name() noexcept
{
	return
#if _WIN32
		"OpenAL32.dll"
#else
		"libopenal.so"
#endif // _WIN32
	;
}

void OalAudioMixer::initialize_oal(const AudioMixerInitParam& param)
{
	ALCint al_context_attributes[] = {0, 0, 0};

	if (param.dst_rate > 0)
	{
		al_context_attributes[0] = ALC_FREQUENCY;
		al_context_attributes[1] = param.dst_rate;
	}

	log_oal_library_file_name();
	log_oal_custom_device();

	oal_loader_ = make_oal_loader(!sd_oal_library.empty() ? sd_oal_library.c_str() : get_oal_default_library_file_name());
	oal_loader_->load_alc_symbols(al_symbols_);

	detect_alc_extensions();

	log_oal_devices();
	log_oal_default_device();

	oal_device_resource_ = make_oal_device(al_symbols_, !sd_oal_device_name.empty() ? sd_oal_device_name.c_str() : nullptr);

	log_oal_current_device_name();
	log_oal_alc_extensions();

	oal_context_resource_ = make_oal_context(al_symbols_, *oal_device_resource_, al_context_attributes);
	make_al_context_current();

	oal_loader_->load_al_symbols(al_symbols_);
	log_oal_al_info();
	log_oal_al_extensions();

	dst_rate_ = get_al_mixing_frequency();

	detect_efx_features();
}

void OalAudioMixer::initialize_distance_model()
{
	distance_model_ = audio_mixer_default_distance_model;
	set_distance_model();
}

void OalAudioMixer::initialize_listener_meters_per_unit()
{
	listener_meters_per_unit_ = audio_mixer_default_meters_per_units;
	set_meters_per_unit();
}

void OalAudioMixer::initialize_listener_r3_position()
{
	listener_r3_position_ = audio_mixer_make_default_listener_r3_position();
	set_listener_r3_position();
}

void OalAudioMixer::initialize_listener_r3_orientation()
{
	listener_r3_orientation_ = audio_mixer_make_default_listener_r3_orientation();
	set_listener_r3_orientation();
}

void OalAudioMixer::initialize_voice_handles()
{
	voice_handle_mgr_.set_cache_capacity(commands_min_capacity);
	voice_handle_mgr_.set_map_capacity(voices_limit);
}

void OalAudioMixer::initialize_command_queue()
{
	commands_.clear();
	commands_.reserve(commands_min_capacity);

	mt_commands_.clear();
	mt_commands_.reserve(commands_min_capacity);
}

void OalAudioMixer::initialize_voices()
{
	const auto max_voice_count = get_max_voice_count();

	if (max_voice_count <= 0)
	{
		return;
	}

	voices_.resize(max_voice_count);

	auto param = OalSourceInitParam{};
	param.mix_sample_rate = dst_rate_;
	param.mix_sample_count = mix_sample_count_;
	param.oal_al_symbols = &al_symbols_;

	for (auto& voice : voices_)
	{
		voice.index = 0;
		voice.is_active = false;
		voice.is_looping = false;
		voice.is_music = false;
		voice.oal_source.initialize(param);
	}
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

void OalAudioMixer::initialize_music()
{
	initialize_music_adlib_sound();
}

void OalAudioMixer::uninitialize_music()
{
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
			fail("Failed to create SFX AdLib audio decoder.");
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
			fail("Failed to create SFX PC Speaker audio decoder.");
		}
	}
}

void OalAudioMixer::initialize_sfx_pcm_sounds()
{
	for (auto& sfx_pcm_sound : sfx_pcm_sounds_)
	{
		sfx_pcm_sound.is_initialized = false;
		sfx_pcm_sound.audio_decoder = make_audio_decoder(AudioDecoderType::pcm, opl3_type_);

		if (!sfx_pcm_sound.audio_decoder)
		{
			fail("Failed to create SFX PCM audio decoder.");
		}
	}
}

void OalAudioMixer::initialize_sfx()
{
	initialize_sfx_adlib_sounds();
	initialize_sfx_pc_speaker_sounds();
	initialize_sfx_pcm_sounds();
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

	voices_.clear();
}

void OalAudioMixer::on_music_stop(Voice& voice)
{
	voice_handle_mgr_.unmap(voice.handle);
}

void OalAudioMixer::on_sfx_stop(
	const Voice& voice)
{
	voice_handle_mgr_.unmap(voice.handle);
}

AudioMixerVoiceHandle OalAudioMixer::play_adlib_music_internal(const void* data, int data_size, bool is_looping)
{
	const auto voice_handle = voice_handle_mgr_.generate();

	auto command = Command{};
	command.type = CommandType::play_music;

	auto& command_param = command.param.play_music;
	command_param.data = data;
	command_param.data_size = data_size;
	command_param.is_looping = is_looping;
	command_param.voice_handle = voice_handle;

	const auto command_mutex_guard = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
	voice_handle_mgr_.cache(voice_handle);
	return voice_handle;
}

AudioMixerVoiceHandle OalAudioMixer::play_sfx_sound_internal(SoundType sound_type, int sound_index, const void* data, int data_size, bool is_r3)
{
	const auto voice_handle = voice_handle_mgr_.generate();

	auto command = Command{};
	command.type = CommandType::play_sfx;

	auto& command_param = command.param.play_sfx;
	command_param.sound_type = sound_type;
	command_param.is_r3 = is_r3;
	command_param.sound_index = sound_index;
	command_param.data = data;
	command_param.data_size = data_size;
	command_param.voice_handle = voice_handle;

	const auto command_mutex_guard = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
	voice_handle_mgr_.cache(voice_handle);
	return voice_handle;
}

void OalAudioMixer::handle_play_music_command(const PlayMusicCommandParam& param)
{
	auto is_started = false;
	auto voice = static_cast<Voice*>(nullptr);

	const auto voice_handle_guard = ScopeGuard{
		[this, &param, &is_started, &voice]()
		{
			if (is_started && voice)
			{
				voice->handle = param.voice_handle;
				voice_handle_mgr_.uncache_and_map(param.voice_handle, voice);
			}
			else
			{
				voice_handle_mgr_.uncache(param.voice_handle);
			}
		}
	};

	voice = find_music_voice();

	if (voice)
	{
		voice->is_active = false;
		voice->oal_source.close();
	}
	else
	{
		voice = find_free_voice();
	}

	if (!voice)
	{
		return;
	}

	auto audio_decoder_param = AudioDecoderInitParam{};
	audio_decoder_param.src_raw_data_ = param.data;
	audio_decoder_param.src_raw_size_ = param.data_size;
	audio_decoder_param.dst_rate_ = dst_rate_;

	if (!music_adlib_sound_.audio_decoder->initialize(audio_decoder_param))
	{
		return;
	}

	auto source_param = OalSourceOpenStreamingParam{};
	source_param.is_3d = false;
	source_param.is_looping = param.is_looping;
	source_param.sample_rate = dst_rate_;
	source_param.uncaching_sound = &music_adlib_sound_;

	voice->oal_source.open(source_param);
	voice->oal_source.play();

	voice->is_active = true;
	voice->is_r3 = false;
	voice->is_looping = param.is_looping;
	voice->is_music = true;

	is_started = true;
}

void OalAudioMixer::handle_play_sfx_command(const PlaySfxCommandParam& param)
{
	auto is_started = false;
	auto voice = static_cast<Voice*>(nullptr);

	const auto voice_handle_guard = ScopeGuard{
		[this, &param, &is_started, &voice]()
		{
			if (is_started && voice)
			{
				voice_handle_mgr_.uncache_and_map(param.voice_handle, voice);
			}
			else
			{
				voice_handle_mgr_.uncache(param.voice_handle);
			}
		}
	};

	auto& sfx_sound = (param.sound_type == SoundType::adlib_sfx ? sfx_adlib_sounds_[param.sound_index] : (param.sound_type == SoundType::pc_speaker_sfx ? sfx_pc_speaker_sounds_[param.sound_index] : sfx_pcm_sounds_[param.sound_index]));

	if (!sfx_sound.is_initialized)
	{
		auto audio_decoder_param = AudioDecoderInitParam{};
		audio_decoder_param.src_raw_data_ = param.data;
		audio_decoder_param.src_raw_size_ = param.data_size;
		audio_decoder_param.dst_rate_ = dst_rate_;
		auto audio_decoder = sfx_sound.audio_decoder.get();

		if (!audio_decoder->initialize(audio_decoder_param))
		{
			return;
		}

		auto sample_count = audio_decoder->get_dst_length_in_samples();

		if (sample_count <= 0)
		{
			return;
		}

		sfx_sound.is_initialized = true;
		sfx_sound.is_decoded = false;
		sfx_sound.sample_offset = 0;
		sfx_sound.sample_count = sample_count;
		sfx_sound.samples.resize(sample_count);
	}

	voice = find_free_voice();

	if (!voice)
	{
		return;
	}

	const auto is_3d = param.is_r3;

	if (sfx_sound.is_decoded)
	{
		const auto decoded_data_size = static_cast<int>(sfx_sound.sample_count * sizeof(OalSourceSample));
		auto source_param = OalSourceOpenStaticParam{};
		source_param.is_3d = is_3d;
		source_param.sample_rate = dst_rate_;
		source_param.data = sfx_sound.samples.data();
		source_param.data_size = decoded_data_size;
		voice->oal_source.open(source_param);
	}
	else
	{
		auto source_param = OalSourceOpenStreamingParam{};
		source_param.is_3d = is_3d;
		source_param.is_looping = false;
		source_param.sample_rate = dst_rate_;
		source_param.caching_sound = &sfx_sound;
		voice->oal_source.open(source_param);
	}

	voice->is_r3 = is_3d;
	voice->is_looping = false;
	voice->is_music = false;

	if (voice->is_r3)
	{
		voice->r3_attenuation = audio_mixer_make_default_voice_attenuation();
		voice->r3_position = audio_mixer_make_default_voice_r3_position();
		voice->oal_source.set_reference_distance(voice->r3_attenuation.min_distance);
		voice->oal_source.set_max_distance(voice->r3_attenuation.max_distance);
		voice->oal_source.set_rolloff_factor(voice->r3_attenuation.roll_off_factor);
		voice->oal_source.set_position(voice->r3_position.x, voice->r3_position.y, voice->r3_position.z);
	}

	voice->oal_source.play();
	voice->handle = param.voice_handle;
	voice->is_active = true;

	is_started = true;
}

void OalAudioMixer::handle_set_mute_command(const SetMuteCommandParam& param)
{
	if (is_mute_ == param.is_mute)
	{
		return;
	}

	is_mute_ = param.is_mute;
	const auto al_gain = (is_mute_ ? 0.0F : 1.0F);

	static_cast<void>(al_symbols_.alGetError());
	al_symbols_.alListenerf(AL_GAIN, al_gain);
	assert(al_symbols_.alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::handle_set_distance_model_command(const SetDistanceModelCommandParam& param)
{
	if (distance_model_ == param.distance_model)
	{
		return;
	}

	distance_model_ = param.distance_model;
	set_distance_model();
}

void OalAudioMixer::handle_set_listener_meters_per_unit_command(const SetListenerMetersPerUnitCommandParam& param)
{
	if (listener_meters_per_unit_ == param.meters_per_unit)
	{
		return;
	}

	listener_meters_per_unit_ = param.meters_per_unit;
	set_meters_per_unit();
}

void OalAudioMixer::handle_set_listener_r3_position_command(const SetListenerR3PositionCommandParam& param)
{
	if (listener_r3_position_ == param.r3_position)
	{
		return;
	}

	listener_r3_position_ = param.r3_position;
	set_listener_r3_position();
}

void OalAudioMixer::handle_set_listener_r3_orientation_command(const SetListenerR3OrientationCommandParam& param)
{
	if (listener_r3_orientation_ == param.r3_orientation)
	{
		return;
	}

	listener_r3_orientation_ = param.r3_orientation;
	set_listener_r3_orientation();
}

void OalAudioMixer::handle_set_voice_pause_command(AudioMixerVoiceHandle voice_handle, bool is_pause)
{
	const auto voice = voice_handle_mgr_.get_voice(voice_handle);

	if (!voice || !voice->is_active)
	{
		return;
	}

	if (is_pause)
	{
		voice->oal_source.pause();
	}
	else
	{
		voice->oal_source.resume();
	}
}

void OalAudioMixer::handle_pause_voice_command(const PauseVoiceCommandParam& param)
{
	handle_set_voice_pause_command(param.handle, true);
}

void OalAudioMixer::handle_resume_voice_command(const ResumeVoiceCommandParam& param)
{
	handle_set_voice_pause_command(param.handle, false);
}

void OalAudioMixer::handle_stop_voice_command(const StopVoiceCommandParam& param)
{
	const auto voice = voice_handle_mgr_.get_voice_and_invalidate(param.handle);

	if (!voice || !voice->is_active)
	{
		return;
	}

	voice->is_active = false;
	voice->oal_source.stop();
}

void OalAudioMixer::handle_set_voice_gain_command(const SetVoiceGainCommandParam& param)
{
	const auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (!voice || !voice->is_active)
	{
		return;
	}

	voice->oal_source.set_gain(param.gain);
}

void OalAudioMixer::handle_set_voice_r3_attenuation_command(const SetVoiceR3AttenuationCommandParam& param)
{
	const auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (!voice || !voice->is_active)
	{
		return;
	}

	if (voice->r3_attenuation == param.attributes)
	{
		return;
	}

	voice->r3_attenuation = param.attributes;
	voice->oal_source.set_reference_distance(voice->r3_attenuation.min_distance);
	voice->oal_source.set_max_distance(voice->r3_attenuation.max_distance);
	voice->oal_source.set_rolloff_factor(voice->r3_attenuation.roll_off_factor);
}

void OalAudioMixer::handle_set_voice_r3_position_command(const SetVoiceR3PositionCommandParam& param)
{
	auto voice = voice_handle_mgr_.get_voice(param.handle);

	if (!voice)
	{
		return;
	}

	if (voice->r3_position == param.position)
	{
		return;
	}

	voice->r3_position = param.position;

	if (has_efx_meters_per_unit_)
	{
		voice->oal_source.set_position(param.position.x, param.position.y, param.position.z);
	}
	else
	{
		const auto r3_position = voice->r3_position * listener_meters_per_unit_;
		voice->oal_source.set_position(r3_position.x, r3_position.y, r3_position.z);
	}
}

void OalAudioMixer::handle_commands()
{
	if (is_state_suspended_.load(std::memory_order_acquire))
	{
		return;
	}

	{
		const auto commands_lock = MutexUniqueLock{commands_mutex_};

		if (!commands_.empty())
		{
			mt_commands_.insert(mt_commands_.end(), commands_.cbegin(), commands_.cend());
			commands_.clear();
		}
	}

	for (const auto& command : mt_commands_)
	{
		switch (command.type)
		{
			case CommandType::play_music:
				handle_play_music_command(command.param.play_music);
				break;

			case CommandType::play_sfx:
				handle_play_sfx_command(command.param.play_sfx);
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

	mt_commands_.clear();
}

void OalAudioMixer::decode_adlib_sound(OalSourceCachingSound& adlib_sound, int gain_scale)
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
		samples[i] = scale_sample(samples[i], gain_scale);
	}

	adlib_sound.sample_offset += decoded_count;

	if (decoded_count <= sample_count)
	{
		adlib_sound.is_decoded = true;
		adlib_sound.sample_count = adlib_sound.sample_offset;
	}
}

void OalAudioMixer::decode_pc_speaker_sound(OalSourceCachingSound& pc_speaker_sound)
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

void OalAudioMixer::decode_pcm_sound(OalSourceCachingSound& pcm_sound)
{
	if (!pcm_sound.is_initialized || pcm_sound.is_decoded)
	{
		return;
	}

	const auto remain_count = pcm_sound.sample_count - pcm_sound.sample_offset;

	if (remain_count == 0)
	{
		pcm_sound.is_decoded = true;
		return;
	}

	const auto sample_count = std::min(remain_count, oal_source_max_streaming_buffers * dst_rate_);
	const auto samples = &pcm_sound.samples[pcm_sound.sample_offset];
	const auto decoded_count = pcm_sound.audio_decoder->decode(sample_count, samples);

	pcm_sound.sample_offset += decoded_count;

	if (decoded_count <= sample_count)
	{
		pcm_sound.is_decoded = true;
		pcm_sound.sample_count = pcm_sound.sample_offset;
	}
}

void OalAudioMixer::mix_sfx_voice(Voice& voice)
{
	if (!voice.is_active)
	{
		return;
	}

	if (voice.is_music)
	{
		mix_music(voice);
		return;
	}

	auto& oal_source = voice.oal_source;
	oal_source.mix();

	if (oal_source.is_finished())
	{
		voice.is_active = false;
		on_sfx_stop(voice);
	}

	if (!voice.is_active)
	{
		return;
	}
}

bool OalAudioMixer::mix_music_mix_buffer(Voice& voice)
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
				samples[i] = scale_sample(samples[i], adlib_music_gain_scale);
			}

			decode_offset += decoded_count;
			to_decode_count -= decoded_count;
		}
		else
		{
			if (!voice.is_looping || !audio_decoder->rewind())
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

bool OalAudioMixer::mix_music_mix_buffers(Voice& voice)
{
	while (music_adlib_sound_.queue_size < oal_source_max_streaming_buffers)
	{
		if (!mix_music_mix_buffer(voice))
		{
			return false;
		}

		music_adlib_sound_.queue_size += 1;
	}

	return true;
}

void OalAudioMixer::mix_music(Voice& voice)
{
	if (!music_adlib_sound_.is_initialized || !voice.oal_source.is_open())
	{
		return;
	}

	if (!mix_music_mix_buffers(voice))
	{
		voice.oal_source.close();
		on_music_stop(voice);

		return;
	}

	voice.oal_source.mix();

	if (voice.oal_source.is_finished())
	{
		voice.oal_source.close();
		on_music_stop(voice);
	}
}

void OalAudioMixer::initialize_thread()
{
	is_quit_thread_ = false;
	thread_ = Thread{&OalAudioMixer::thread_func, this};
}

void OalAudioMixer::thread_func()
{
	constexpr auto sleep_delay = std::chrono::milliseconds{1};

	while (true)
	{
		{
			const auto thread_mutex_guard = MutexUniqueLock{thread_mutex_};

			if (is_quit_thread_)
			{
				return;
			}
		}

		for (auto& sfx_adlib_sound : sfx_adlib_sounds_)
		{
			decode_adlib_sound(sfx_adlib_sound, adlib_sfx_gain_scale);
		}

		for (auto& sfx_pc_speaker_sound : sfx_pc_speaker_sounds_)
		{
			decode_pc_speaker_sound(sfx_pc_speaker_sound);
		}

		for (auto& sfx_pcm_sound : sfx_pcm_sounds_)
		{
			decode_pcm_sound(sfx_pcm_sound);
		}

		al_symbols_.alcSuspendContext(oal_context_resource_.get());
		handle_commands();

		for (auto& voice : voices_)
		{
			mix_sfx_voice(voice);
		}

		al_symbols_.alcProcessContext(oal_context_resource_.get());
		std::this_thread::sleep_for(sleep_delay);
	}
}

OalAudioMixer::Voice* OalAudioMixer::find_free_voice() noexcept
{
	for (auto& voice : voices_)
	{
		if (!voice.is_active)
		{
			return &voice;
		}
	}

	return nullptr;
}

OalAudioMixer::Voice* OalAudioMixer::find_music_voice() noexcept
{
	for (auto& voice : voices_)
	{
		if (voice.is_active && voice.is_music)
		{
			return &voice;
		}
	}

	return nullptr;
}

void OalAudioMixer::set_al_distance_model(ALenum al_distance_model)
{
	static_cast<void>(al_symbols_.alGetError());
	al_symbols_.alDistanceModel(al_distance_model);
	assert(al_symbols_.alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::set_distance_model()
{
	auto al_distance_model = ALenum{};

	switch (distance_model_)
	{
		case AudioMixerDistanceModel::inverse_clamped:
			al_distance_model = AL_INVERSE_DISTANCE_CLAMPED;
			break;

		case AudioMixerDistanceModel::linear_clamped:
			al_distance_model = AL_LINEAR_DISTANCE_CLAMPED;
			break;

		default:
			fail("Unknown distance model.");
	}

	set_al_distance_model(al_distance_model);
}

void OalAudioMixer::set_al_meters_per_unit(double meters_per_unit)
{
	static_cast<void>(al_symbols_.alGetError());
	al_symbols_.alListenerf(AL_METERS_PER_UNIT, static_cast<ALfloat>(meters_per_unit));
	assert(al_symbols_.alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::set_meters_per_unit()
{
	if (has_efx_meters_per_unit_)
	{
		set_al_meters_per_unit(listener_meters_per_unit_);
		return;
	}

	const auto listener_r3_position = listener_r3_position_ * listener_meters_per_unit_;
	set_al_listener_r3_position(listener_r3_position.x, listener_r3_position.y, listener_r3_position.z);

	for (auto& voice : voices_)
	{
		if (!voice.is_active || voice.is_music || !voice.is_r3)
		{
			continue;
		}

		const auto voice_r3_position = voice.r3_position * listener_meters_per_unit_;
		voice.oal_source.set_position(voice_r3_position.x, voice_r3_position.y, voice_r3_position.z);
	}
}

void OalAudioMixer::set_al_listener_r3_position(double x, double y, double z)
{
	assert(al_symbols_.alGetError);
	assert(al_symbols_.alListener3f);

	static_cast<void>(al_symbols_.alGetError());
	al_symbols_.alListener3f(AL_POSITION, static_cast<ALfloat>(x), static_cast<ALfloat>(y), static_cast<ALfloat>(z));
	assert(al_symbols_.alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::set_listener_r3_position()
{
	if (has_efx_meters_per_unit_)
	{
		set_al_listener_r3_position(listener_r3_position_.x, listener_r3_position_.y, listener_r3_position_.z);
	}
	else
	{
		const auto listener_r3_position = listener_r3_position_ * listener_meters_per_unit_;
		set_al_listener_r3_position(listener_r3_position.x, listener_r3_position.y, listener_r3_position.z);
	}
}

void OalAudioMixer::set_al_listener_orientation(double at_x, double at_y, double at_z, double up_x, double up_y, double up_z)
{
	const ALfloat al_orientation[] =
	{
		static_cast<ALfloat>(at_x),
		static_cast<ALfloat>(at_y),
		static_cast<ALfloat>(at_z),

		static_cast<ALfloat>(up_x),
		static_cast<ALfloat>(up_y),
		static_cast<ALfloat>(up_z),
	};

	assert(al_symbols_.alGetError);
	assert(al_symbols_.alListenerfv);

	static_cast<void>(al_symbols_.alGetError());
	al_symbols_.alListenerfv(AL_ORIENTATION, al_orientation);
	assert(al_symbols_.alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::set_listener_r3_orientation()
{
	set_al_listener_orientation(listener_r3_orientation_.at.x, listener_r3_orientation_.at.y, listener_r3_orientation_.at.z, listener_r3_orientation_.up.x, listener_r3_orientation_.up.y, listener_r3_orientation_.up.z);
}

OalSourceSample OalAudioMixer::scale_sample(OalSourceSample sample, int scalar) noexcept
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

} // bstone
