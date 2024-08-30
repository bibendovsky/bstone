/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "bstone_oal_audio_mixer.h"

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
#include "bstone_assert.h"
#include "bstone_audio_decoder.h"
#include "bstone_audio_mixer_validator.h"
#include "bstone_exception.h"
#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_oal_source.h"
#include "bstone_oal_loader.h"
#include "bstone_oal_resource.h"
#include "bstone_scope_exit.h"

namespace bstone
{

OalAudioMixer::OalAudioMixer(const AudioMixerInitParam& param)
try {
	switch (param.opl3_type)
	{
		case Opl3Type::dbopl:
		case Opl3Type::nuked:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown OPL3 type.");
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
	initialize_is_mute();
	initialize_gain();
	update_al_gain();
	initialize_listener_r3_position();
	initialize_listener_r3_orientation();
	initialize_voice_handles();
	initialize_voices();
	initialize_command_queue();

	initialize_music();
	initialize_sfx();
	initialize_thread();

	is_mute_ = false;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

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
try {
	auto command = Command{};
	command.type = CommandType::set_mute;
	auto& command_param = command.param.set_mute;
	command_param.is_mute = is_mute;

	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

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

void OalAudioMixer::set_gain(double gain)
try {
	AudioMixerValidator::validate_gain(gain);

	auto command = Command{};
	command.type = CommandType::set_gain;
	auto& command_param = command.param.set_gain;
	command_param.gain = gain;

	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::set_listener_r3_position(const AudioMixerListenerR3Position& r3_position)
try {
	auto command = Command{};
	command.type = CommandType::set_listener_r3_position;
	auto& command_param = command.param.set_listener_r3_position;
	command_param.r3_position = r3_position;
	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::set_listener_r3_orientation(const AudioMixerListenerR3Orientation& r3_orientation)
try {
	auto command = Command{};
	command.type = CommandType::set_listener_r3_orientation;
	auto& command_param = command.param.set_listener_r3_orientation;
	command_param.r3_orientation = r3_orientation;
	const auto commands_lock = MutexUniqueLock{commands_mutex_};
	commands_.emplace_back(command);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

AudioMixerVoiceHandle OalAudioMixer::play_sound(const AudioMixerPlaySoundParam& param)
try {
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
			BSTONE_THROW_STATIC_SOURCE("Unknown sound type.");
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool OalAudioMixer::is_voice_playing(AudioMixerVoiceHandle voice_handle) const
try {
	return voice_handle_mgr_.is_valid_handle(voice_handle);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::pause_voice(AudioMixerVoiceHandle voice_handle)
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::resume_voice(AudioMixerVoiceHandle voice_handle)
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::stop_voice(AudioMixerVoiceHandle voice_handle)
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::set_voice_gain(AudioMixerVoiceHandle voice_handle, double gain)
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::set_voice_r3_position(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Position& r3_position)
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool OalAudioMixer::can_set_voice_output_gains() const
{
	return false;
}

void OalAudioMixer::enable_set_voice_output_gains(
	AudioMixerVoiceHandle,
	bool)
try {
	fail_unsupported();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalAudioMixer::set_voice_output_gains(
	AudioMixerVoiceHandle,
	AudioMixerOutputGains&)
try {
	fail_unsupported();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

[[noreturn]] void OalAudioMixer::fail_unsupported()
{
	BSTONE_THROW_STATIC_SOURCE("Not supported.");
}

void OalAudioMixer::make_al_context_current()
{
	const auto al_result = al_symbols_.alcMakeContextCurrent(oal_context_resource_.get());

	if (al_result == ALC_FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to make context current.");
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
		BSTONE_THROW_STATIC_SOURCE("Attributes size out of range.");
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

	BSTONE_THROW_STATIC_SOURCE("No ALC_FREQUENCY attribute.");
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
	BSTONE_ASSERT(al_symbols_.alcIsExtensionPresent);

	has_alc_enumeration_ext_ = (al_symbols_.alcIsExtensionPresent(nullptr, alc_enumeration_ext_str) != ALC_FALSE);
	has_alc_enumerate_all_ext_ = (al_symbols_.alcIsExtensionPresent(nullptr, alc_enumerate_all_ext_str) != ALC_FALSE);
}

void OalAudioMixer::log(const OalString& string)
{
	static const auto prefix = OalString{"[SND_OAL] "};
	globals::logger->log_information((prefix + string).c_str());
}

void OalAudioMixer::log_oal_library_file_name()
{
	log(std::string{"Default library: \""} + get_oal_default_library_file_name() + '\"');
	const auto oal_library = sd_get_oal_library();
	const auto oal_library_string = std::string{
		oal_library.get_data(),
		static_cast<std::size_t>(oal_library.get_size())};
	log("Custom library: \"" + oal_library_string + '\"');
}

void OalAudioMixer::log_oal_custom_device()
{
	auto message = std::string{};
	message += "Custom device: \"";
	const auto device_name = sd_get_oal_device_name();
	message.append(device_name.get_data(), static_cast<std::size_t>(device_name.get_size()));
	message += '"';
	log(message);
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

	auto oal_library_string = std::string{};
	const auto oal_library = sd_get_oal_library();

	if (oal_library.is_empty())
	{
		oal_library_string = get_oal_default_library_file_name();
	}
	else
	{
		oal_library_string.append(oal_library.get_data(), static_cast<std::size_t>(oal_library.get_size()));
	}

	oal_loader_ = make_oal_loader(oal_library_string.c_str());
	oal_loader_->load_alc_symbols(al_symbols_);

	detect_alc_extensions();

	log_oal_devices();
	log_oal_default_device();

	auto device_name_c_string = static_cast<const char*>(nullptr);
	auto device_name_string = std::string{};
	const auto device_name_sv = sd_get_oal_device_name();

	if (!device_name_sv.is_empty())
	{
		device_name_string.append(
			device_name_sv.get_data(),
			static_cast<std::size_t>(device_name_sv.get_size()));

		device_name_c_string = device_name_string.c_str();
	}

	oal_device_resource_ = make_oal_device(al_symbols_, device_name_c_string);
	log_oal_current_device_name();
	log_oal_alc_extensions();

	oal_context_resource_ = make_oal_context(al_symbols_, *oal_device_resource_, al_context_attributes);
	make_al_context_current();

	oal_loader_->load_al_symbols(al_symbols_);
	log_oal_al_info();
	log_oal_al_extensions();

	dst_rate_ = get_al_mixing_frequency();
}

void OalAudioMixer::initialize_distance_model()
{
	BSTONE_ASSERT(al_symbols_.alDistanceModel != nullptr);
	al_symbols_.alDistanceModel(AL_NONE);
}

void OalAudioMixer::initialize_is_mute() noexcept
{
	is_mute_ = false;
}

void OalAudioMixer::initialize_gain() noexcept
{
	gain_ = audio_mixer_max_gain;
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
			BSTONE_THROW_STATIC_SOURCE("Failed to create SFX AdLib audio decoder.");
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
			BSTONE_THROW_STATIC_SOURCE("Failed to create SFX PC Speaker audio decoder.");
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
			BSTONE_THROW_STATIC_SOURCE("Failed to create SFX PCM audio decoder.");
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

void OalAudioMixer::update_al_gain()
{
	const auto al_gain = (is_mute_ ? 0.0F : static_cast<ALfloat>(gain_));

	static_cast<void>(al_symbols_.alGetError());
	al_symbols_.alListenerf(AL_GAIN, al_gain);
	BSTONE_ASSERT(al_symbols_.alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::handle_play_music_command(const PlayMusicCommandParam& param)
{
	auto is_started = false;
	auto voice = static_cast<Voice*>(nullptr);

	const auto voice_handle_guard = make_scope_exit(
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
		});

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

	const auto voice_handle_guard = make_scope_exit(
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
		});

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
		voice->r3_position = audio_mixer_make_default_voice_r3_position();
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
	update_al_gain();
}

void OalAudioMixer::handle_set_gain_command(const SetGainCommandParam& param)
{
	gain_ = param.gain;
	update_al_gain();
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
	voice->oal_source.set_position(param.position.x, param.position.y, param.position.z);
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

			default:
				BSTONE_ASSERT(false && "Unknown command.");
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

void OalAudioMixer::set_al_listener_r3_position(double x, double y, double z)
{
	BSTONE_ASSERT(al_symbols_.alGetError);
	BSTONE_ASSERT(al_symbols_.alListener3f);

	static_cast<void>(al_symbols_.alGetError());
	al_symbols_.alListener3f(AL_POSITION, static_cast<ALfloat>(x), static_cast<ALfloat>(y), static_cast<ALfloat>(z));
	BSTONE_ASSERT(al_symbols_.alGetError() == AL_NO_ERROR);
}

void OalAudioMixer::set_listener_r3_position()
{
	set_al_listener_r3_position(listener_r3_position_.x, listener_r3_position_.y, listener_r3_position_.z);
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

	BSTONE_ASSERT(al_symbols_.alGetError);
	BSTONE_ASSERT(al_symbols_.alListenerfv);

	static_cast<void>(al_symbols_.alGetError());
	al_symbols_.alListenerfv(AL_ORIENTATION, al_orientation);
	BSTONE_ASSERT(al_symbols_.alGetError() == AL_NO_ERROR);
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
