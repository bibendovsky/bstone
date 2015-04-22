/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

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
============================================================== */


#ifndef BSTONE_AUDIO_MIXER_H
#define BSTONE_AUDIO_MIXER_H


#include <deque>
#include <list>
#include <vector>

#include "SDL.h"

#include "bstone_audio_decoder.h"


namespace bstone {


enum ActorType {
    AT_NONE,
    AT_ACTOR,
    AT_DOOR,
    AT_WALL
}; // enum ActorType

enum ActorChannel {
    AC_VOICE,
    AC_WEAPON,
    AC_ITEM,
    AC_HIT_WALL,
    AC_NO_WAY
}; // enum ActorChannel

enum SoundType {
    ST_NONE,
    ST_ADLIB_MUSIC,
    ST_ADLIB_SFX,
    ST_PCM
}; // enum SoundType


class AudioMixer {
public:
    AudioMixer();

    ~AudioMixer();

    bool initialize(
        int dst_rate);

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

    static int get_max_channels();

private:
    typedef int16_t Sample;
    typedef std::vector<Sample> Samples;

    typedef float MixSample;
    typedef std::vector<MixSample> MixSamples;

    class CacheItem {
    public:
        bool is_active;
        bool is_invalid;
        SoundType sound_type;
        int samples_count;
        int decoded_count;
        Samples samples;
        AudioDecoder* decoder;

        CacheItem();

        CacheItem(
            const CacheItem& that);

        ~CacheItem();

        CacheItem& operator=(
            const CacheItem& that);

        bool is_decoded() const;
    }; // class CacheItem

    typedef std::vector<CacheItem> Cache;
    typedef Cache::iterator CacheIt;

    class Position {
    public:
        int x;
        int y;
    }; // class Position

    typedef std::vector<Position> Positions;
    typedef Positions::iterator PositionsIt;
    typedef Positions::const_iterator PositionsCIt;

    class PlayerPosition {
    public:
        int view_x;
        int view_y;
        int view_cos;
        int view_sin;
    }; // class PlayerPosition

    class PositionsState {
    public:
        PlayerPosition player;
        Positions actors;
        Positions doors;
        Position wall;
    }; // class PositionsState

    typedef std::deque<PositionsState> PositionsStateQueue;

    class Sound {
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
    }; // class Sound

    typedef std::list<Sound> Sounds;
    typedef Sounds::iterator SoundsIt;
    typedef Sounds::const_iterator SoundsCIt;

    enum CommandType {
        CMD_PLAY,
        CMD_STOP_MUSIC,
        CMD_STOP_ALL_SFX
    }; // enum CommandType

    class Command {
    public:
        CommandType command;
        Sound sound;
        const void* data;
        int data_size;
    }; // class Command

    typedef std::deque<Command> PlayCommands;
    typedef PlayCommands::iterator PlayCommandsIt;
    typedef PlayCommands::const_iterator PlayCommandsCIt;

    bool is_initialized_;
    int dst_rate_;
    SDL_AudioDeviceID device_id_;
    SDL_mutex* mutex_;
    SDL_Thread* thread_;
    int mix_samples_count_;
    Samples buffer_;
    MixSamples mix_buffer_;
    volatile bool is_data_available_;
    volatile bool quit_thread_;
    Sounds sounds_;
    PlayCommands commands_;
    PlayCommands commands_queue_;
    bool mute_;
    Cache adlib_music_cache_;
    Cache adlib_sfx_cache_;
    Cache pcm_cache_;
    PositionsState positions_state_;
    PositionsStateQueue positions_state_queue_;
    volatile int player_channels_state_;
    volatile bool is_music_playing_;
    volatile bool is_any_sfx_playing_;
    volatile float sfx_volume_;
    volatile float music_volume_;

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

    static void callback_proxy(
        void* user_data,
        uint8_t* dst_data,
        int dst_length);

    static int mix_proxy(
        void* user_data);

    static int calculate_mix_samples_count(
        int dst_rate);

    static AudioDecoder* create_decoder_by_sound_type(
        SoundType sound_type);

    static bool is_sound_type_valid(
        SoundType sound_type);

    static bool is_sound_index_valid(
        int sound_index,
        SoundType sound_type);

    AudioMixer(
        const AudioMixer& that);

    AudioMixer& operator=(
        const AudioMixer& that);
}; // class AudioMixer


} // namespace bstone


#endif // BSTONE_AUDIO_MIXER_H
