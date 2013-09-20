#ifndef BSTONE_AUDIO_MIXER_H
#define BSTONE_AUDIO_MIXER_H


#include <deque>
#include <list>
#include <vector>

#include "SDL.h"

#include "bstone_audio_decoder.h"


namespace bstone {


enum ActorChannel {
    AC_VOICE,
    AC_WEAPON,
    AC_ITEM,
    AC_WALL_HIT,
}; // enum ActorChannel

enum SoundType {
    ST_NONE,
    ST_ADLIB_MUSIC,
    ST_ADLIB_SFX,
    ST_PCM,
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
        const void* data,
        int data_size);

    // Negative index of an actor defines a non-positional sound.
    bool play_adlib_sound(
        const void* data,
        int data_size,
        int actor_index = -1,
        ActorChannel actor_channel = AC_VOICE);

    // Negative index of an actor defines a non-positional sound.
    bool play_pcm_sound(
        const void* data,
        int data_size,
        int actor_index = -1,
        ActorChannel actor_channel = AC_VOICE);

    // Decodes a sound in background.
    bool prepare_sound(
        SoundType sound_type,
        int sound_index,
        const void* data,
        int data_size);

    bool stop_music();

    bool set_mute(
        bool value);

    bool is_music_playing() const;

    bool is_non_music_playing() const;

    bool is_anything_playing() const;

    static int get_min_rate();

    static int get_max_channels();

private:
    typedef int16_t Sample;
    typedef std::vector<Sample> Samples;

    typedef int MixSample;
    typedef std::vector<MixSample> MixSamples;

    class Sound {
    public:
        SoundType type;
        int index;
        int decode_offset;
        int samples_count;
        int actor_index;
        ActorChannel actor_channel;
    }; // class Sound

    typedef std::list<Sound> Sounds;
    typedef Sounds::iterator SoundsIt;
    typedef Sounds::const_iterator SoundsCIt;

    enum PlayCommandType {
        PT_PLAY,
        PT_STOP,
    }; // enum PlayCommandType

    class PlayCommand {
    public:
        PlayCommandType command;
        SoundType sound_type;
        int sound_index;
        int actor_index;
        ActorChannel actor_channel;
    }; // class PlayCommand

    typedef std::deque<PlayCommand> PlayCommands;
    typedef PlayCommands::iterator PlayCommandsIt;
    typedef PlayCommands::const_iterator PlayCommandsCIt;

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

        bool is_finished() const;
    }; // class CacheItem

    typedef std::vector<CacheItem> Cache;
    typedef Cache::iterator CacheIt;

    class CacheCommand {
    public:
        SoundType sound_type;
        int sound_index;
        const void* data;
        int data_size;
    }; // class CacheCommand

    typedef std::deque<CacheCommand> CacheCommands;
    typedef CacheCommands::iterator CacheCommandsIt;
    typedef CacheCommands::const_iterator CacheCommandsCIt;

    bool is_initialized_;
    int dst_rate_;
    SDL_AudioDeviceID device_id_;
    SDL_mutex* mix_mutex_;
    SDL_mutex* decode_command_mutex_;
    SDL_mutex* decode_data_mutex_;
    SDL_Thread* mix_thread_;
    SDL_Thread* decode_thread_;
    int mix_samples_count_;
    Samples buffer_;
    MixSamples mix_buffer_;
    volatile bool is_data_available_;
    volatile bool quit_thread_;
    Sounds sounds_;
    PlayCommands play_commands_;
    PlayCommands play_commands_queue_;
    bool mute_;
    Cache adlib_music_cache_;
    Cache adlib_sfx_cache_;
    Cache pcm_cache_;
    CacheCommands cache_commands_;
    CacheCommands cache_commands_queue_;

    void callback(
        Uint8* dst_data,
        int dst_length);

    void mix();

    void mix_process_samples();

    void mix_handle_command(
        const PlayCommand& command);

    void mix_handle_commands(
        PlayCommands& sound_commands);

    void decode();

    void decode_cache(
        Cache& cache);

    void decode_handle_commands();

    bool play_sound(
        SoundType sound_type,
        const void* data,
        int data_size,
        int actor_index = -1,
        ActorChannel actor_channel = AC_VOICE);

    CacheItem* get_cache_item(
        SoundType sound_type,
        int sound_index);

    static void callback_proxy(
        void* user_data,
        Uint8* dst_data,
        int dst_length);

    static int mix_proxy(
        void* user_data);

    static int decode_proxy(
        void* user_data);

    static int calculate_mix_samples_count(
        int dst_rate);

    static AudioDecoder* create_decoder_by_sound_type(
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
