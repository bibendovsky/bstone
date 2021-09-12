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


#ifndef BSTONE_OAL_AUDIO_MIXER_INCLUDED
#define BSTONE_OAL_AUDIO_MIXER_INCLUDED


#include "bstone_audio_mixer.h"

#include <array>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "3d_def.h"
#include "audio.h"

#include "bstone_audio_decoder.h"
#include "bstone_circular_queue.h"
#include "bstone_oal_source.h"
#include "bstone_oal_loader.h"
#include "bstone_oal_resource.h"
#include "bstone_spinlock.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalAudioMixer final :
	public AudioMixer
{
public:
	OalAudioMixer();

	~OalAudioMixer() override;


	// Note: Mix size in milliseconds.
	bool initialize(
		const AudioMixerInitParam& param) override;

	void uninitialize() override;

	bool is_initialized() const override;

	Opl3Type get_opl3_type() const override;

	int get_rate() const override;

	int get_channel_count() const override;

	int get_mix_size_ms() const override;

	float get_sfx_volume() const override;

	float get_music_volume() const override;

	bool play_adlib_music(
		int music_index,
		const void* data,
		int data_size,
		bool is_looping) override;

	// Negative index of an actor defines a non-positional sound.
	bool play_adlib_sound(
		int sound_index,
		int priority,
		const void* data,
		int data_size,
		int actor_index = -1,
		ActorType actor_type = ActorType::none,
		ActorChannel actor_channel = ActorChannel::voice) override;

	// Negative index of an actor defines a non-positional sound.
	bool play_pc_speaker_sound(
		int sound_index,
		int priority,
		const void* data,
		int data_size,
		int actor_index = -1,
		ActorType actor_type = ActorType::none,
		ActorChannel actor_channel = ActorChannel::voice) override;

	// Negative index of an actor defines a non-positional sound.
	bool play_pcm_sound(
		int sound_index,
		int priority,
		const void* data,
		int data_size,
		int actor_index = -1,
		ActorType actor_type = ActorType::none,
		ActorChannel actor_channel = ActorChannel::voice) override;

	bool update_positions() override;

	bool stop_music() override;

	bool stop_pausable_sfx() override;

	bool pause_all_sfx(
		bool is_pause) override;

	bool pause_music(
		bool is_pause) override;

	bool set_mute(
		bool value) override;

	bool set_sfx_volume(
		float volume) override;

	bool set_music_volume(
		float volume) override;

	bool is_music_playing() const override;

	bool is_any_unpausable_sfx_playing() const override;

	bool is_player_channel_playing(
		ActorChannel channel) const override;

	int get_min_rate() const override;

	int get_default_rate() const override;

	int get_min_mix_size_ms() const override;

	int get_default_mix_size_ms() const override;

	int get_max_channels() const override;

	int get_max_commands() const override;


private:
	using Mutex = Spinlock;
	using MutexUniqueLock = std::unique_lock<Mutex>;

	using OalString = std::string;
	using OalStrings = std::vector<OalString>;

	using SfxAdLibSounds = std::array<OalSourceCachingSound, NUMSOUNDS>;
	using SfxPcSpeakerSounds = std::array<OalSourceCachingSound, NUMSOUNDS>;

	struct SfxBsPosition
	{
		double x{};
		double y{};
	}; // SfxBsPosition

	struct SfxPosition
	{
		ActorType actor_type{};
		int actor_index{};
		double x{};
		double y{};
	}; // SfxPosition

	struct SfxPlayer
	{
		double view_x{};
		double view_y{};

		double view_cos{};
		double view_sin{};
	}; // SfxPlayer

	struct SfxPushwall
	{
		int direction{};
		int x{};
		int y{};
		double offset{};
	}; // SfxPushwall

	using SfxActorPositions = std::array<SfxBsPosition, MAXACTORS>;
	using SfxDoorPositions = std::array<SfxBsPosition, MAXDOORS>;
	using SfxModifiedPositionIndices = std::unordered_set<int>;

	struct SfxVoice
	{
		int index{};
		bool is_active{};
		OalSource oal_source{};

		int priority{};
		ActorType actor_type{};
		int actor_index{};
		ActorChannel actor_channel{};
	}; // SfxVoice

	using SfxVoices = std::vector<SfxVoice>;

	using VoiceMutex = Spinlock;


	enum class CommandId
	{
		none,

		play_music,
		play_sfx,

		pause_music,
		stop_music,
		set_music_volume,

		stop_pausable_sfx,
		pause_all_sfx,
		set_sfx_volume,
	}; // CommandId

	struct PlayMusicCommandParam
	{
		const void* data{};
		int data_size{};
		bool is_looping{};
	}; // PlayMusicCommandParam

	struct PlaySfxCommandParam
	{
		AudioSfxType sfx_type{};
		int sound_index{};
		int priority{};
		const void* data{};
		int data_size{};
		int actor_index{};
		ActorType actor_type{};
		ActorChannel actor_channel{};
	}; // PlaySfxCommandParam

	struct PauseMusicCommandParam
	{
		bool is_pause{};
	}; // PauseMusicCommandParam

	struct SetMusicVolumeCommandParam
	{
		float volume{};
	}; // SetMusicVolumeCommandParam

	struct PauseAllSfxCommandParam
	{
		bool is_pause{};
	}; // PauseAllSfxCommandParam

	struct SetSfxVolumeCommandParam
	{
		float volume{};
	}; // SetSfxVolumeCommandParam

	union CommandParam
	{
		PlayMusicCommandParam play_music;
		PlaySfxCommandParam play_sfx;
		PauseMusicCommandParam pause_music;
		SetMusicVolumeCommandParam set_music_volume;
		PauseAllSfxCommandParam pause_all_sfx;
		SetSfxVolumeCommandParam set_sfx_volume;
	}; // CommandParam

	struct Command
	{
		CommandId id{};
		CommandParam param{};
	}; // Command

	using CommandQueue = CircularQueue<Command>;
	using CommandQueueMutex = Mutex;

	using SfxPositionQueue = CircularQueue<SfxPosition>;
	using SfxPositionQueueMutex = Mutex;

	using StatMutex = Mutex;
	using PlayerChannelsStats = std::array<int, static_cast<std::size_t>(ActorChannel::count_)>;


	static constexpr auto min_mix_size_ms = 20;
	static constexpr auto max_mix_size_ms = 40;
	static_assert(max_mix_size_ms > min_mix_size_ms, "Mix size out of range.");

	static constexpr auto max_commands = 256;

	static constexpr auto max_sfx_positions = 2 * (MAXACTORS + MAXDOORS + MAXWALLTILES);

	static constexpr auto sfx_voices_limit = 255;
	static constexpr auto music_voices_limit = 1;
	static constexpr auto voices_limit = sfx_voices_limit + music_voices_limit;

	static constexpr auto meters_per_unit = 1.0;
	static constexpr auto default_reference_distance = 1.0;
	static constexpr auto default_oal_position_y = 0.5;

	static constexpr auto adlib_sfx_volume_scale = 7;
	static constexpr auto adlib_music_volume_scale = 6;

	static constexpr auto alc_enumeration_ext_str = "ALC_ENUMERATION_EXT";
	static constexpr auto alc_enumerate_all_ext_str = "ALC_ENUMERATE_ALL_EXT";

	using MusicMutex = Mutex;
	using SfxMutex = Mutex;

	using Thread = std::thread;


	bool is_initialized_{};
	Opl3Type opl3_type_;
	int dst_rate_;
	int mix_sample_count_;
	int mix_size_ms_;
	bool is_mute_{};

	bool has_alc_enumeration_ext_{};
	bool has_alc_enumerate_all_ext_{};

	OalLoaderUPtr oal_loader_{};
	OalAlcSymbols oal_alc_symbols_{};
	OalAlSymbols oal_al_symbols_{};
	OalDeviceResource oal_device_resource_{};
	OalContextResource oal_context_resource_{};

	CommandQueueMutex command_queue_mutex_{};
	CommandQueue command_queue_{};
	CommandQueue mt_command_queue_{};

	SfxPositionQueueMutex sfx_position_queue_mutex_{};
	SfxPositionQueue sfx_position_queue_{};
	SfxPositionQueue mt_sfx_position_queue_{};

	mutable MusicMutex music_mutex_{};
	OalSourceUncachingSound music_adlib_sound_{};
	OalSource music_source_{};
	float music_volume_{};
	bool is_music_looping_{};

	SfxVoices sfx_voices_{};
	SfxAdLibSounds sfx_adlib_sounds_{};
	SfxPcSpeakerSounds sfx_pc_speaker_sounds_{};
	float sfx_volume_{};

	SfxActorPositions sfx_bs_actor_positions_{};
	SfxDoorPositions sfx_door_positions_{};

	bool is_sfx_player_position_modified_{};
	SfxPlayer sfx_player_{};

	SfxModifiedPositionIndices sfx_actor_modified_position_indices_{};

	SfxModifiedPositionIndices sfx_door_modified_position_indices_{};

	bool is_sfx_pushwall_position_modified_{};
	SfxPushwall sfx_pushwall_{};

	mutable StatMutex stat_mutex_{};
	bool stat_music_is_playing_{};
	int stat_unpausable_sfx_count_{};
	PlayerChannelsStats stat_player_channels_{};

	bool is_quit_thread_{};
	Mutex thread_mutex_{};
	Thread thread_{};


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	bool play_sfx_sound(
		AudioSfxType sfx_type,
		int sound_index,
		int priority,
		const void* data,
		int data_size,
		int actor_index,
		ActorType actor_type,
		ActorChannel actor_channel);


	void make_al_context_current();

	OalStrings get_alc_device_names();

	OalString get_default_alc_device_name();

	OalString get_alc_device_name();

	OalStrings parse_al_token_string(
		const char* al_token_string);

	OalStrings get_alc_extensions();

	OalStrings get_al_extensions();

	int get_al_mixing_frequency();

	int get_max_voice_count();


	void detect_alc_extensions();


	void log(
		const OalString& string);

	void log_oal_library_file_name();

	void log_oal_custom_device();

	void log_oal_devices();

	void log_oal_default_device();

	void log_oal_current_device_name();

	void log_oal_alc_extensions();

	void log_oal_al_info();

	void log_oal_al_extensions();

	static const char* get_oal_default_library_file_name() noexcept;

	void initialize_oal(
		const AudioMixerInitParam& param);


	void initialize_command_queue();

	void initialize_sfx_position_queue();


	void initialize_music_adlib_sound();

	void initialize_music_source();

	void initialize_music();

	void uninitialize_music();


	void initialize_sfx_adlib_sounds();

	void initialize_sfx_pc_speaker_sounds();

	void initialize_sfx_voices();

	void initialize_sfx_positions();

	void initialize_sfx();

	void uninitialize_sfx();


	void on_music_start();

	void on_music_stop();


	void on_sfx_start(
		const SfxVoice& sfx_voice);

	void on_sfx_stop(
		const SfxVoice& sfx_voice);


	void handle_play_music_command(
		const PlayMusicCommandParam& param);

	void handle_play_sfx_command(
		const PlaySfxCommandParam& param);

	void handle_pause_music_command(
		const PauseMusicCommandParam& param);

	void handle_stop_music_command();

	void handle_set_music_volume_command(
		const SetMusicVolumeCommandParam& param);

	void handle_stop_pausable_sfx_command();

	void handle_pause_all_sfx_command(
		const PauseAllSfxCommandParam& param);

	void handle_set_sfx_volume_command(
		const SetSfxVolumeCommandParam& param);

	void handle_commands();

	void handle_positions();


	void decode_adlib_sound(
		OalSourceCachingSound& adlib_sound,
		int volume_scale);

	void decode_pc_speaker_sound(
		OalSourceCachingSound& pc_speaker_sound);

	void mix_sfx_voice(
		SfxVoice& sfx_voice);

	bool mix_music_mix_buffer();

	bool mix_music_mix_buffers();

	void mix_music();

	void initialize_thread();

	void thread_func();


	static bool is_2d_sfx(
		ActorType actor_type,
		int actor_index);

	SfxVoice* find_free_sfx_voice(
		int priority,
		ActorType actor_type,
		int actor_index,
		ActorChannel actor_channel);

	void set_al_listener_position(
		float x,
		float y,
		float z);

	void set_al_listener_orientation(
		float at_x,
		float at_y,
		float at_z,
		float up_x,
		float up_y,
		float up_z);


	void update_positions_player();

	void update_positions_actors();

	void update_positions_doors();

	void update_positions_pushwall();


	void apply_positions_player();

	void apply_actor_position(
		SfxVoice& sfx_voice);

	void apply_door_position(
		SfxVoice& sfx_voice);

	void apply_pushwall_position(
		SfxVoice& sfx_voice);

	void enqueue_positions_actors();

	void enqueue_positions_doors();

	void enqueue_positions_pushwall();

	void enqueue_positions();

	void set_sfx_actor_position(
		OalSource& oal_source,
		int actor_index);

	void set_sfx_door_position(
		OalSource& oal_source,
		int actor_index);

	void set_sfx_pushwall_position(
		OalSource& oal_source);

	void set_sfx_position(
		OalSource& oal_source,
		ActorType actor_type,
		int actor_index);

	void set_sfx_reference_distance(
		OalSource& oal_source);


	static OalSourceSample scale_sample(
		OalSourceSample sample,
		int scalar) noexcept;
}; // OalAudioMixer

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_OAL_AUDIO_MIXER_INCLUDED
