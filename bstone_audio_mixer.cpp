#include "bstone_audio_mixer.h"

#include <cassert>

#include <algorithm>

#include "id_heads.h"

#include "bstone_adlib_music_decoder.h"
#include "bstone_adlib_sfx_decoder.h"
#include "bstone_pcm_decoder.h"


extern boolean sqPlayedOnce;


namespace bstone {


AudioMixer::Sound::Sound() :
    type(ST_NONE),
    data(NULL),
    data_size(0),
    decoder(NULL)
{
}

AudioMixer::Sound::Sound(
    const Sound& that) :
        type(that.type),
        data(that.data),
        data_size(that.data_size),
        buffer(that.buffer),
        decoder(NULL)
{
}

AudioMixer::Sound::~Sound()
{
    delete decoder;
}

AudioMixer::Sound& AudioMixer::Sound::operator=(
    const Sound& that)
{
    if (&that != this) {
        type = that.type;
        data = that.data;
        data_size = that.data_size;
        buffer = that.buffer;
    }

    return *this;
}

AudioMixer::AudioMixer() :
    is_initialized_(false),
    dst_rate_(0),
    device_id_(0),
    mutex_(NULL),
    thread_(NULL),
    mix_samples_count_(0),
    is_data_available_(false),
    quit_thread_(false)
{
}

AudioMixer::~AudioMixer()
{
    uninitialize();
}

bool AudioMixer::initialize(
    int dst_rate)
{
    uninitialize();

    if (::SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        int sdl_result;

        sdl_result = ::SDL_InitSubSystem(SDL_INIT_AUDIO);

        if (sdl_result != 0)
            return false;
    }

    dst_rate_ = std::max(dst_rate, get_min_rate());

    mix_samples_count_ = calculate_mix_samples_count(dst_rate_);

    SDL_AudioSpec src_spec;
    src_spec.freq = dst_rate_;
    src_spec.format = AUDIO_S16SYS;
    src_spec.channels = get_max_channels();
    src_spec.samples = mix_samples_count_;
    src_spec.callback = callback_proxy;
    src_spec.userdata = this;

    SDL_AudioSpec dst_spec;

    device_id_ = ::SDL_OpenAudioDevice(
        NULL,
        0,
        &src_spec,
        &dst_spec,
        SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    if (device_id_ == 0)
        return false;

    bool is_succeed = true;

    if (is_succeed) {
        mutex_ = ::SDL_CreateMutex();

        is_succeed = (mutex_ != NULL);
    }

    if (is_succeed) {
        thread_ = ::SDL_CreateThread(
            mix_proxy, "bstone_mixer_thread", this);

        is_succeed = (thread_ != NULL);
    }

    if (is_succeed) {
        is_initialized_ = true;
        mix_samples_count_ = dst_spec.samples;

        int total_samples = get_max_channels() * mix_samples_count_;

        buffer_.resize(total_samples);

        ::SDL_PauseAudioDevice(device_id_, 0);
    } else
        uninitialize();

    return is_succeed;
}

void AudioMixer::uninitialize()
{
    is_initialized_ = false;

    if (device_id_ != 0) {
        ::SDL_PauseAudioDevice(device_id_, 1);
        ::SDL_CloseAudioDevice(device_id_);
        device_id_ = 0;
    }

    if (thread_ != NULL) {
        quit_thread_ = true;

        int status;
        ::SDL_WaitThread(thread_, &status);

        thread_ = NULL;
    }

    if (mutex_ != NULL) {
        ::SDL_DestroyMutex(mutex_);
        mutex_ = NULL;
    }

    dst_rate_ = 0;
    mix_samples_count_ = NULL;
    buffer_.swap(Samples());
    is_data_available_ = false;
    quit_thread_ = false;
    sounds_.swap(Sounds());
}

bool AudioMixer::is_initialized() const
{
    return is_initialized_;
}

bool AudioMixer::play_adlib_music(
    const void* data,
    int data_size)
{
    return play_sound(ST_ADLIB_MUSIC, data, data_size);
}

bool AudioMixer::play_adlib_sound(
    const void* data,
    int data_size)
{
    return play_sound(ST_ADLIB_SFX, data, data_size);
}

bool AudioMixer::play_pcm_sound(
    const void* data,
    int data_size)
{
    return play_sound(ST_PCM, data, data_size);
}

bool AudioMixer::stop_music()
{
    if (!is_initialized())
        return false;

    ::SDL_LockMutex(mutex_);

    if (!sounds_.empty()) {
        for (SoundsIt i = sounds_.begin(); i != sounds_.end(); ++i) {
            if (i->type == ST_ADLIB_MUSIC) {
                sounds_.erase(i);
                break;
            }
        }
    }

    ::SDL_UnlockMutex(mutex_);

    return true;
}

bool AudioMixer::set_mute(
    bool value)
{
    if (!is_initialized())
        return false;

    mute_ = value;

    return true;
}

bool AudioMixer::is_music_playing() const
{
    if (!is_initialized())
        return false;

    bool result = false;

    ::SDL_LockMutex(mutex_);

    if (!sounds_.empty()) {
        class Predicate {
        public:
            bool operator()(const Sound& sound)
            {
                return sound.type == ST_ADLIB_MUSIC;
            }
        }; // class Predicate

        result = (std::find_if(
            sounds_.begin(), sounds_.end(), Predicate()) != sounds_.end());
    }

    ::SDL_UnlockMutex(mutex_);

    return result;
}

bool AudioMixer::is_non_music_playing() const
{
    if (!is_initialized())
        return false;

    bool result = false;

    ::SDL_LockMutex(mutex_);

    if (!sounds_.empty()) {
        class Predicate {
        public:
            bool operator()(const Sound& sound)
            {
                return sound.type != ST_ADLIB_MUSIC;
            }
        }; // class Predicate

        result = (std::find_if(
            sounds_.begin(), sounds_.end(), Predicate()) != sounds_.end());
    }

    ::SDL_UnlockMutex(mutex_);

    return result;
}

bool AudioMixer::is_anything_playing() const
{
    if (!is_initialized())
        return false;

    bool result = false;

    ::SDL_LockMutex(mutex_);
    result = !sounds_.empty();
    ::SDL_UnlockMutex(mutex_);

    return result;
}

// (static)
int AudioMixer::get_min_rate()
{
    return 11025;
}

// (static)
int AudioMixer::get_max_channels()
{
    return 2;
}

void AudioMixer::callback(
    Uint8* dst_data,
    int dst_length)
{
    if (!mute_ && is_data_available_) {
        std::uninitialized_copy(
            reinterpret_cast<const Uint8*>(&buffer_[0]),
            reinterpret_cast<const Uint8*>(&buffer_[0]) + dst_length,
            dst_data);

        is_data_available_ = false;
    } else
        std::uninitialized_fill_n(dst_data, dst_length, 0);
}

void AudioMixer::mix()
{
    while (!quit_thread_) {
        if (!mute_ && !is_data_available_) {
            ::SDL_LockMutex(mutex_);

            for (SoundsIt i = sounds_.begin(); i != sounds_.end(); ) {
                int count = i->decoder->decode(
                    mix_samples_count_, &i->buffer[0]);

                while (count < mix_samples_count_ &&
                    i->type == ST_ADLIB_MUSIC)
                {
                    sqPlayedOnce = true;

                    int remain_count = mix_samples_count_ - count;

                    remain_count = i->decoder->decode(
                        remain_count, &i->buffer[count]);

                    if (remain_count > 0)
                        count += remain_count;
                    else {
                        if (!i->decoder->reset())
                            break;
                    }
                }

                if (count > 0)
                    ++i;
                else
                    i = sounds_.erase(i);
            }

            if (!sounds_.empty()) {
                mix_samples();
                is_data_available_ = true;
            }

            ::SDL_UnlockMutex(mutex_);
        }

        ::SDL_Delay(1);
    }
}

void AudioMixer::mix_samples()
{
    for (int i = 0; i < mix_samples_count_; ++i) {
        int sample = 0;

        for (SoundsCIt sound = sounds_.begin();
            sound != sounds_.end(); ++sound)
        {
            int scale;

            switch (sound->type) {
            case ST_ADLIB_MUSIC:
            case ST_ADLIB_SFX:
                scale = 8;
                break;

            default:
                scale = 1;
                break;
            }

            sample += scale * sound->buffer[i];
        }

        sample /= static_cast<int>(sounds_.size());

        buffer_[(2 * i) + 0] = static_cast<int16_t>(sample);
        buffer_[(2 * i) + 1] = static_cast<int16_t>(sample);
    }
}

bool AudioMixer::play_sound(
    SoundType sound_type,
    const void* data,
    int data_size)
{
    if (!is_initialized())
        return false;

    if (data == NULL)
        return false;

    if (data_size <= 0)
        return false;

    AudioDecoder* decoder = NULL;

    switch (sound_type) {
    case ST_ADLIB_MUSIC:
        decoder = new AdlibMusicDecoder();
        break;

    case ST_ADLIB_SFX:
        decoder = new AdlibSfxDecoder();
        break;

    case ST_PCM:
        decoder = new PcmDecoder();
        break;

    default:
        return false;
    }

    if (!decoder->initialize(data, data_size, dst_rate_)) {
        delete decoder;
        return false;
    }

    ::SDL_LockMutex(mutex_);

    sounds_.push_back(Sound());

    Sound& sound = sounds_.back();

    sound.type = sound_type;
    sound.data = data;
    sound.data_size = data_size;
    sound.buffer.resize(mix_samples_count_);
    sound.decoder = decoder;

    ::SDL_UnlockMutex(mutex_);

    return true;
}

// (static)
void AudioMixer::callback_proxy(
    void* user_data,
    Uint8* dst_data,
    int dst_length)
{
    assert(user_data != NULL);

    AudioMixer* mixer = static_cast<AudioMixer*>(user_data);
    mixer->callback(dst_data, dst_length);
}

// (static)
int AudioMixer::mix_proxy(
    void* user_data)
{
    assert(user_data != NULL);

    AudioMixer* mixer = static_cast<AudioMixer*>(user_data);
    mixer->mix();

    return 0;
}

// (static)
int AudioMixer::calculate_mix_samples_count(
    int dst_rate)
{
    if (dst_rate <= 11025)
        return 256;
    else if (dst_rate <= 22050)
        return 512;
    else
        return 1024;
}


} // namespace bstone
