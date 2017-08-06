/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#ifndef BSTONE_AUDIO_MIXER_INCLUDED
#define BSTONE_AUDIO_MIXER_INCLUDED


#define BSTONE_AUDIO_MIXER_USE_THREAD (0)


#include <atomic>
#include <list>
#include <memory>

#if BSTONE_AUDIO_MIXER_USE_THREAD
#include <mutex>
#endif // BSTONE_AUDIO_MIXER_USE_THREAD

#include <thread>
#include <vector>
#include "SDL.h"
#include "bstone_atomic.h"
#include "bstone_audio_decoder.h"
#include "bstone_mt_queue_1r1w.h"


namespace bstone
{


enum ActorType
{
    AT_NONE,
    AT_ACTOR,
    AT_DOOR,
    AT_WALL
}; // ActorType

enum ActorChannel
{
    AC_VOICE,
    AC_WEAPON,
    AC_ITEM,
    AC_HIT_WALL,
    AC_NO_WAY,
    AC_INTERROGATION,
}; // ActorChannel

enum SoundType
{
    ST_NONE,
    ST_ADLIB_MUSIC,
    ST_ADLIB_SFX,
    ST_PCM
}; // SoundType


class AudioMixer
{
public:
    AudioMixer();

    AudioMixer(
        const AudioMixer& that) = delete;

    AudioMixer& operator=(
        const AudioMixer& that) = delete;

    ~AudioMixer();


    // Note: Mix size in milliseconds.
    bool initialize(
        int dst_rate,
        int mix_size_ms);

    void uninitialize();

    bool is_initialized() const;

    bool play_adlib_music(
        int music_index,
        const void* data,
        int data_size);

    // Negative index of an actor defines a non-positional sound.
    bool play_adlib_sound(
        int sound_index,
        int priority,
        const void* data,
        int data_size,
        int actor_index = -1,
        ActorType actor_type = AT_NONE,
        ActorChannel actor_channel = AC_VOICE);

    // Negative index of an actor defines a non-positional sound.
    bool play_pcm_sound(
        int sound_index,
        int priority,
        const void* data,
        int data_size,
        int actor_index = -1,
        ActorType actor_type = AT_NONE,
        ActorChannel actor_channel = AC_VOICE);

    bool update_positions();

    bool stop_music();

    bool stop_all_sfx();

    bool set_mute(
        bool value);

    bool set_sfx_volume(
        float volume);

    bool set_music_volume(
        float volume);

    bool is_music_playing() const;

    bool is_any_sfx_playing() const;

    bool is_player_channel_playing(
        ActorChannel channel) const;

    static int get_min_rate();

    static int get_default_rate();

    static int get_min_mix_size_ms();

    static int get_default_mix_size_ms();

    static int get_max_channels();

    static int get_max_commands();


private:
    using Sample = int16_t;
    using Samples = std::vector<Sample>;

    using MixSample = float;
    using MixSamples = std::vector<MixSample>;

    class CacheItem
    {
    public:
        bool is_active;
        bool is_invalid;
        SoundType sound_type;
        int samples_count;
        int decoded_count;
        Samples samples;
        std::unique_ptr<AudioDecoder> decoder;

        CacheItem();

        CacheItem(
            const CacheItem& that);

        ~CacheItem();

        CacheItem& operator=(
            const CacheItem& that);

        bool is_decoded() const;
    }; // CacheItem

    using Cache = std::vector<CacheItem>;

    class Location
    {
    public:
        Atomic<int> x;
        Atomic<int> y;
    }; // Location

    using Locations = std::vector<Location>;

    class PlayerLocation
    {
    public:
        Atomic<int> view_x;
        Atomic<int> view_y;
        Atomic<int> view_cos;
        Atomic<int> view_sin;
    }; // PlayerLocation

    class Positions
    {
    public:
        PlayerLocation player;
        Locations actors;
        Locations doors;
        Location wall;

        void initialize();

        void fixed_copy_to(
            Positions& positions);
    }; // Positions

    class Sound
    {
    public:
        SoundType type;
        int priority;
        CacheItem* cache;
        int decode_offset;
        int actor_index;
        ActorType actor_type;
        ActorChannel actor_channel;
        float left_volume;
        float right_volume;

        bool is_audible() const;
    }; // Sound

    using Sounds = std::list<Sound>;

    enum CommandType
    {
        CMD_PLAY,
        CMD_STOP_MUSIC,
        CMD_STOP_ALL_SFX
    }; // CommandType

    class Command
    {
    public:
        CommandType command;
        Sound sound;
        const void* data;
        int data_size;
    }; // Command

    using Commands = bstone::MtQueue1R1W<Command>;

#if BSTONE_AUDIO_MIXER_USE_THREAD
    using Mutex = std::mutex;
    using MutexGuard = std::lock_guard<Mutex>;
#endif // BSTONE_AUDIO_MIXER_USE_THREAD

    bool is_initialized_;
    int dst_rate_;
    SDL_AudioDeviceID device_id_;

#if BSTONE_AUDIO_MIXER_USE_THREAD
    Mutex mutex_;
    std::thread thread_;
#endif // BSTONE_AUDIO_MIXER_USE_THREAD

    int mix_samples_count_;
    Samples buffer_;
    MixSamples mix_buffer_;
    std::atomic_bool is_data_available_;

#if BSTONE_AUDIO_MIXER_USE_THREAD
    std::atomic_bool quit_thread_;
#endif // BSTONE_AUDIO_MIXER_USE_THREAD

    Sounds sounds_;
    Commands commands_;
    bool mute_;
    Cache adlib_music_cache_;
    Cache adlib_sfx_cache_;
    Cache pcm_cache_;
    Positions positions_;
    std::atomic_int player_channels_state_;
    std::atomic_bool is_music_playing_;
    std::atomic_bool is_any_sfx_playing_;
    std::atomic<float> sfx_volume_;
    std::atomic<float> music_volume_;
    int mix_size_ms_;

    void callback(
        uint8_t* dst_data,
        int dst_length);

    void mix();

    void mix_samples();

    void handle_commands();

    void handle_play_command(
        const Command& command);

    void handle_stop_music_command();

    void handle_stop_all_sfx_command();

    bool initialize_cache_item(
        const Command& command,
        CacheItem& cache_item);

    bool decode_sound(
        const Sound& sound);

    void spatialize_sound(
        Sound& sound);

    void spatialize_sounds();

    bool play_sound(
        SoundType sound_type,
        int sound_index,
        int priority,
        const void* data,
        int data_size,
        int actor_index = -1,
        ActorType actor_type = AT_NONE,
        ActorChannel actor_channel = AC_VOICE);

    CacheItem* get_cache_item(
        SoundType sound_type,
        int sound_index);

    void set_player_channel_state(
        const Sound& sound,
        bool state);

    void lock();

    void unlock();

    static void callback_proxy(
        void* user_data,
        uint8_t* dst_data,
        int dst_length);

    static int mix_proxy(
        void* user_data);

    static int calculate_mix_samples_count(
        int dst_rate,
        int mix_size_ms);

    static AudioDecoder* create_decoder_by_sound_type(
        SoundType sound_type);

    static bool is_sound_type_valid(
        SoundType sound_type);

    static bool is_sound_index_valid(
        int sound_index,
        SoundType sound_type);
}; // AudioMixer


} // bstone


#endif // BSTONE_AUDIO_MIXER_INCLUDED
