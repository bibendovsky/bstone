#ifndef BSTONE_AUDIO_MIXER_H
#define BSTONE_AUDIO_MIXER_H


#include <list>
#include <queue>
#include <vector>

#include "SDL.h"

#include "bstone_audio_decoder.h"


namespace bstone {


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

    bool play_adlib_sound(
        const void* data,
        int data_size);

    bool play_pcm_sound(
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

    enum SoundType {
        ST_NONE,
        ST_ADLIB_MUSIC,
        ST_ADLIB_SFX,
        ST_PCM,
    }; // enum SoundType

    class Sound {
    public:
        SoundType type;
        const void* data;
        int data_size;
        Samples buffer;
        AudioDecoder* decoder;

        Sound();

        Sound(
            const Sound& that);

        ~Sound();

        Sound& operator=(
            const Sound& that);
    }; // class Sound

    typedef std::list<Sound> Sounds;
    typedef Sounds::iterator SoundsIt;
    typedef Sounds::const_iterator SoundsCIt;

    bool is_initialized_;
    int dst_rate_;
    SDL_AudioDeviceID device_id_;
    SDL_mutex* mutex_;
    SDL_Thread* thread_;
    int mix_samples_count_;
    Samples buffer_;
    volatile bool is_data_available_;
    volatile bool quit_thread_;
    Sounds sounds_;
    bool mute_;

    void callback(
        Uint8* dst_data,
        int dst_length);

    void mix();

    void mix_samples();

    bool play_sound(
        SoundType sound_type,
        const void* data,
        int data_size);

    static void callback_proxy(
        void* user_data,
        Uint8* dst_data,
        int dst_length);

    static int mix_proxy(
        void* user_data);

    static int calculate_mix_samples_count(
        int dst_rate);

    AudioMixer(
        const AudioMixer& that);

    AudioMixer& operator=(
        const AudioMixer& that);
}; // class AudioMixer


} // namespace bstone


#endif // BSTONE_AUDIO_MIXER_H
