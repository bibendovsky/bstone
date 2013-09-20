#include "bstone_audio_mixer.h"

#include <cassert>

#include <algorithm>
#include <functional>

#include "3d_def.h"

#include "bstone_adlib_music_decoder.h"
#include "bstone_adlib_sfx_decoder.h"
#include "bstone_pcm_decoder.h"


extern boolean sqPlayedOnce;
extern int sound_index;


namespace bstone {


AudioMixer::CacheItem::CacheItem() :
    is_active(false),
    is_invalid(false),
    sound_type(ST_NONE),
    samples_count(0),
    decoded_count(0),
    decoder(NULL)
{
}

AudioMixer::CacheItem::CacheItem(
    const CacheItem& that) :
        is_active(that.is_active),
        is_invalid(that.is_invalid),
        sound_type(that.sound_type),
        samples_count(that.samples_count),
        decoded_count(that.decoded_count),
        samples(that.samples)
{
    if (that.decoder != NULL)
        decoder = that.decoder->clone();
    else
        decoder = NULL;
}

AudioMixer::CacheItem::~CacheItem()
{
    delete decoder;
}

AudioMixer::CacheItem& AudioMixer::CacheItem::operator=(
    const CacheItem& that)
{
    if (&that != this) {
        is_active = that.is_active;
        is_invalid = that.is_invalid;
        sound_type = that.sound_type;
        samples_count = that.samples_count;
        decoded_count = that.decoded_count;
        samples = that.samples;

        if (that.decoder != NULL)
            decoder = that.decoder->clone();
        else
            decoder = NULL;
    }

    return *this;
}

void AudioMixer::CacheItem::purge()
{
    is_active = false;
    is_invalid = false;
    sound_type = ST_NONE;
    samples_count = 0;
    decoded_count = 0;
    Samples().swap(samples);

    delete decoder;
    decoder = NULL;
}

bool AudioMixer::CacheItem::is_finished() const
{
    return decoded_count == samples_count;
}

AudioMixer::AudioMixer() :
    is_initialized_(false),
    dst_rate_(0),
    device_id_(0),
    mix_mutex_(NULL),
    decode_command_mutex_(NULL),
    decode_data_mutex_(NULL),
    mix_thread_(NULL),
    decode_thread_(NULL),
    mix_samples_count_(0),
    is_data_available_(false),
    quit_thread_(false),
    mute_(false)
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
    src_spec.channels = static_cast<Uint8>(get_max_channels());
    src_spec.samples = static_cast<Uint16>(mix_samples_count_);
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
        mix_mutex_ = ::SDL_CreateMutex();
        is_succeed = (mix_mutex_ != NULL);
    }

    if (is_succeed) {
        mix_thread_ = ::SDL_CreateThread(
            mix_proxy, "bstone_mixer_thread", this);

        is_succeed = (mix_thread_ != NULL);
    }

    if (is_succeed) {
        decode_command_mutex_ = ::SDL_CreateMutex();
        is_succeed = (decode_command_mutex_ != NULL);
    }

    if (is_succeed) {
        decode_data_mutex_ = ::SDL_CreateMutex();
        is_succeed = (decode_data_mutex_ != NULL);
    }

    if (is_succeed) {
        decode_thread_ = ::SDL_CreateThread(
            decode_proxy, "bstone_decode_thread", this);

        is_succeed = (decode_thread_ != NULL);
    }

    if (is_succeed) {
        is_initialized_ = true;
        mix_samples_count_ = dst_spec.samples;

        int total_samples = get_max_channels() * mix_samples_count_;

        buffer_.resize(total_samples);
        mix_buffer_.resize(mix_samples_count_);

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

    quit_thread_ = true;

    if (mix_thread_ != NULL) {
        int status;
        ::SDL_WaitThread(mix_thread_, &status);
        mix_thread_ = NULL;
    }

    if (decode_thread_ != NULL) {
        int status;
        ::SDL_WaitThread(decode_thread_, &status);
        decode_thread_ = NULL;
    }

    if (mix_mutex_ != NULL) {
        ::SDL_DestroyMutex(mix_mutex_);
        mix_mutex_ = NULL;
    }

    if (decode_command_mutex_ != NULL) {
        ::SDL_DestroyMutex(decode_command_mutex_);
        decode_command_mutex_ = NULL;
    }

    if (decode_data_mutex_ != NULL) {
        ::SDL_DestroyMutex(decode_data_mutex_);
        decode_data_mutex_ = NULL;
    }

    dst_rate_ = 0;
    mix_samples_count_ = 0;
    Samples().swap(buffer_);
    MixSamples().swap(mix_buffer_);
    is_data_available_ = false;
    quit_thread_ = false;
    Sounds().swap(sounds_);
    PlayCommands().swap(play_commands_);
    PlayCommands().swap(play_commands_queue_);
    mute_ = false;
    Cache().swap(adlib_music_cache_);
    Cache().swap(adlib_sfx_cache_);
    Cache().swap(pcm_cache_);
    CacheCommands().swap(cache_commands_);
    CacheCommands().swap(cache_commands_queue_);
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
    int data_size,
    int actor_index,
    ActorChannel actor_channel)
{
    return play_sound(ST_ADLIB_SFX, data, data_size,
        actor_index, actor_channel);
}

bool AudioMixer::play_pcm_sound(
    const void* data,
    int data_size,
    int actor_index,
    ActorChannel actor_channel)
{
    return play_sound(ST_PCM, data, data_size,
        actor_index, actor_channel);
}

bool AudioMixer::prepare_sound(
    SoundType sound_type,
    int sound_index,
    const void* data,
    int data_size)
{
    if (!is_initialized())
        return false;

    if (!is_sound_index_valid(sound_index, sound_type))
        return false;

    if (data == NULL)
        return false;

    if (data_size <= 0)
        return false;

    CacheCommand cache_command;
    cache_command.sound_type = sound_type;
    cache_command.sound_index = sound_index;
    cache_command.data = data;
    cache_command.data_size = data_size;

    ::SDL_LockMutex(decode_command_mutex_);
    cache_commands_queue_.push_back(cache_command);
    ::SDL_UnlockMutex(decode_command_mutex_);

    return true;
}

bool AudioMixer::stop_music()
{
    if (!is_initialized())
        return false;

    ::SDL_LockMutex(mix_mutex_);

    PlayCommand play_command;
    play_command.command = PT_STOP;
    play_command.sound_type = ST_ADLIB_MUSIC;
    play_command.sound_index = sound_index;
    play_command.actor_index = -1;
    play_command.actor_channel = AC_VOICE;

    ::SDL_LockMutex(mix_mutex_);
    play_commands_queue_.push_back(play_command);
    ::SDL_UnlockMutex(mix_mutex_);

    ::SDL_UnlockMutex(mix_mutex_);

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

    ::SDL_LockMutex(mix_mutex_);

    if (!sounds_.empty()) {
        class Predicate {
        public:
            static bool test(const Sound& sound)
            {
                return sound.type == ST_ADLIB_MUSIC;
            }
        }; // class Predicate

        result = (std::find_if(
            sounds_.begin(), sounds_.end(), Predicate::test) !=
                sounds_.end());
    }

    ::SDL_UnlockMutex(mix_mutex_);

    return result;
}

bool AudioMixer::is_non_music_playing() const
{
    if (!is_initialized())
        return false;

    bool result = false;

    ::SDL_LockMutex(mix_mutex_);

    if (!sounds_.empty()) {
        class Predicate {
        public:
            static bool test(const Sound& sound)
            {
                return sound.type != ST_ADLIB_MUSIC;
            }
        }; // class Predicate

        result = (std::find_if(
            sounds_.begin(), sounds_.end(), Predicate::test) !=
                sounds_.end());
    }

    ::SDL_UnlockMutex(mix_mutex_);

    return result;
}

bool AudioMixer::is_anything_playing() const
{
    if (!is_initialized())
        return false;

    bool result = false;

    ::SDL_LockMutex(mix_mutex_);
    result = !sounds_.empty();
    ::SDL_UnlockMutex(mix_mutex_);

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
        mix_handle_commands();

        if (!mute_ && !is_data_available_ && !sounds_.empty()) {
            mix_process_samples();
            is_data_available_ = true;
        }

        ::SDL_Delay(1);
    }
}

void AudioMixer::mix_process_samples()
{
    ::SDL_LockMutex(decode_data_mutex_);

    int min_sample = 32767;
    int max_sample = -32768;

    for (int i = 0; i < mix_samples_count_; ++i) {
        int sample = 0;

        for (SoundsIt sound = sounds_.begin(); sound != sounds_.end(); ) {
            CacheItem* cache_item = get_cache_item(
                sound->type, sound->index);

            if (cache_item == NULL) {
                sound = sounds_.erase(sound);
                continue;
            }

            if (!cache_item->is_active) {
                ++sound;
                continue;
            }

            if (sound->decode_offset == cache_item->decoded_count) {
                ++sound;
                continue;
            }

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

            sample += scale * cache_item->samples[sound->decode_offset];

            ++sound->decode_offset;

            if (sound->decode_offset == cache_item->decoded_count) {
                if (cache_item->is_finished()) {
                    if (sound->type == ST_ADLIB_MUSIC) {
                        sqPlayedOnce = true;
                        sound->decode_offset = 0;
                    } else {
                        sound = sounds_.erase(sound);
                        continue;
                    }
                }
            }

            ++sound;
        }

        mix_buffer_[i] = sample;
        min_sample = std::min(sample, min_sample);
        max_sample = std::max(sample, max_sample);
    }

    ::SDL_UnlockMutex(decode_data_mutex_);

    for (int i = 0; i < mix_samples_count_; ++i) {
        int sample = mix_buffer_[i];

        sample = std::min(sample, 32767);
        sample = std::max(sample, -32768);

        buffer_[(i * 2) + 0] = static_cast<int16_t>(sample);
        buffer_[(i * 2) + 1] = static_cast<int16_t>(sample);
    }
}

void AudioMixer::mix_handle_command(
    const PlayCommand& command)
{
    switch (command.command) {
    case PT_PLAY:
        Sound sound;

        sound.type = command.sound_type;
        sound.index = command.sound_index;
        sound.decode_offset = 0;
        sound.samples_count = 0;
        sound.actor_index = command.actor_index;
        sound.actor_channel = command.actor_channel;

        sounds_.push_back(sound);


        if (command.sound_type != ST_ADLIB_MUSIC) {
            // Remove sounds which will be overritten.

            for (SoundsIt i = sounds_.begin(); i != sounds_.end(); ) {
                if (command.actor_index >= 0 &&
                    i->actor_index == command.actor_index &&
                    i->actor_channel == command.actor_channel)
                {
                    i = sounds_.erase(i);
                } else
                    ++i;
            }
        }

        break;

    case PT_STOP:
        if (sound.type != ST_ADLIB_MUSIC) {
            ::SDL_LockMutex(decode_data_mutex_);

            for (SoundsIt i = sounds_.begin(); i != sounds_.end(); ) {
                if (i->type != ST_ADLIB_MUSIC)
                    ++i;
                else {
                    CacheItem& cache_item = adlib_music_cache_[i->index];
                    i = sounds_.erase(i);
                    cache_item.purge();
                }
            }

            ::SDL_UnlockMutex(decode_data_mutex_);
        }
        break;
    }
}

void AudioMixer::mix_handle_commands()
{
    ::SDL_LockMutex(mix_mutex_);

    play_commands_.insert(
        play_commands_.end(),
        play_commands_queue_.begin(),
        play_commands_queue_.end());

    play_commands_queue_.clear();

    ::SDL_UnlockMutex(mix_mutex_);


    for (PlayCommandsCIt i = play_commands_.begin();
        i != play_commands_.end(); ++i)
    {
        mix_handle_command(*i);
    }

    play_commands_.clear();
}

void AudioMixer::decode()
{
    adlib_music_cache_.clear();
    adlib_music_cache_.resize(LASTMUSIC);

    adlib_sfx_cache_.clear();
    adlib_sfx_cache_.resize(NUMSOUNDS);

    pcm_cache_.clear();
    pcm_cache_.resize(NUMSOUNDS);

    while (!quit_thread_) {
        decode_handle_commands();
        decode_cache(adlib_music_cache_);
        decode_cache(adlib_sfx_cache_);
        decode_cache(pcm_cache_);
        ::SDL_Delay(1);
    }
}

void AudioMixer::decode_cache(
    Cache& cache)
{
    for (CacheIt i = cache.begin(); i != cache.end(); ++i) {
        if (!i->is_active)
            continue;

        if (i->samples_count <= 0)
            continue;

        if (i->samples.empty())
            continue;

        if (i->decoder == NULL)
            continue;

        if (!i->decoder->is_initialized())
            continue;

        if (i->is_finished())
            continue;

        ::SDL_LockMutex(decode_data_mutex_);

        int planed_count = std::min(
            i->samples_count - i->decoded_count, mix_samples_count_);

        int actual_count = i->decoder->decode(
            planed_count, &i->samples[i->decoded_count]);

        i->decoded_count += actual_count;

        if (actual_count == 0)
            i->decoder->uninitialize();

        ::SDL_UnlockMutex(decode_data_mutex_);
    }
}

void AudioMixer::decode_handle_commands()
{
    ::SDL_LockMutex(decode_command_mutex_);

    cache_commands_.insert(
        cache_commands_.end(),
        cache_commands_queue_.begin(),
        cache_commands_queue_.end());

    ::SDL_UnlockMutex(decode_command_mutex_);


    ::SDL_LockMutex(decode_data_mutex_);

    for (CacheCommandsCIt i = cache_commands_.begin();
        i != cache_commands_.end(); ++i) {
        if (!is_sound_index_valid(i->sound_index,
            i->sound_type))
        {
            continue;
        }

        if (i->data == NULL)
            continue;

        if (i->data_size <= 0)
            continue;

        CacheItem* cache_item = get_cache_item(
            i->sound_type, i->sound_index);

        if (cache_item == NULL)
            continue;

        if (cache_item->is_active)
            continue;

        if (cache_item->is_invalid)
            continue;

        cache_item->is_active = true;
        cache_item->is_invalid = true;
        cache_item->sound_type = i->sound_type;
        cache_item->decoded_count = 0;

        if (cache_item->decoder == NULL) {
            cache_item->decoder = create_decoder_by_sound_type(
                i->sound_type);
        }

        if (!cache_item->decoder->initialize(
            i->data, i->data_size, dst_rate_))
        {
            continue;
        }

        cache_item->samples_count =
            cache_item->decoder->get_dst_length_in_samples();

        if (cache_item->samples_count == 0)
            continue;

        if (cache_item->samples.empty())
            cache_item->samples.resize(cache_item->samples_count);

        cache_item->is_invalid = false;
    }

    ::SDL_UnlockMutex(decode_data_mutex_);

    cache_commands_.clear();
}

bool AudioMixer::play_sound(
    SoundType sound_type,
    const void* data,
    int data_size,
    int actor_index,
    ActorChannel actor_channel)
{
    if (!is_initialized())
        return false;

    if (actor_index >= MAXACTORS)
        return false;

    switch (actor_channel) {
    case AC_VOICE:
    case AC_WEAPON:
    case AC_ITEM:
    case AC_WALL_HIT:
        break;

    default:
        return false;
    }

    if (!prepare_sound(sound_type, sound_index, data, data_size))
        return false;

    PlayCommand sound_command;
    sound_command.command = PT_PLAY;
    sound_command.sound_type = sound_type;
    sound_command.sound_index = sound_index;
    sound_command.actor_index = actor_index;
    sound_command.actor_channel = actor_channel;

    ::SDL_LockMutex(mix_mutex_);
    play_commands_queue_.push_back(sound_command);
    ::SDL_UnlockMutex(mix_mutex_);

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
int AudioMixer::decode_proxy(
    void* user_data)
{
    assert(user_data != NULL);

    AudioMixer* mixer = static_cast<AudioMixer*>(user_data);
    mixer->decode();

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

AudioMixer::CacheItem* AudioMixer::get_cache_item(
    SoundType sound_type,
    int sound_index)
{
    if (!is_sound_index_valid(sound_index, sound_type))
        return NULL;

    switch (sound_type) {
    case ST_ADLIB_MUSIC:
        return &adlib_music_cache_[sound_index];

    case ST_ADLIB_SFX:
        return &adlib_sfx_cache_[sound_index];

    case ST_PCM:
        return &pcm_cache_[sound_index];

    default:
        return NULL;
    }
}

// (static)
AudioDecoder* AudioMixer::create_decoder_by_sound_type(
    SoundType sound_type)
{
    switch (sound_type) {
    case ST_ADLIB_MUSIC:
        return new AdlibMusicDecoder();

    case ST_ADLIB_SFX:
        return new AdlibSfxDecoder();

    case ST_PCM:
        return new PcmDecoder();

    default:
        return NULL;
    }
}

// (static)
bool AudioMixer::is_sound_index_valid(
    int sound_index,
    SoundType sound_type)
{
    switch (sound_type) {
    case ST_ADLIB_MUSIC:
        return sound_index >= 0 && sound_index < LASTMUSIC;

    case ST_ADLIB_SFX:
    case ST_PCM:
        return sound_index >= 0 && sound_index < NUMSOUNDS;

    default:
        return false;
    }
}


} // namespace bstone
