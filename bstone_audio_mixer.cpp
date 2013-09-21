#include "bstone_audio_mixer.h"

#include <cassert>

#include <algorithm>
#include <functional>

#include "3d_def.h"

#include "bstone_adlib_music_decoder.h"
#include "bstone_adlib_sfx_decoder.h"
#include "bstone_pcm_decoder.h"


extern boolean sqPlayedOnce;


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

        delete decoder;

        if (that.decoder != NULL)
            decoder = that.decoder->clone();
        else
            decoder = NULL;
    }

    return *this;
}

bool AudioMixer::CacheItem::is_decoded() const
{
    return decoded_count == samples_count;
}

AudioMixer::AudioMixer() :
    is_initialized_(false),
    dst_rate_(0),
    device_id_(0),
    mutex_(NULL),
    thread_(NULL),
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
        mix_buffer_.resize(mix_samples_count_);

        adlib_music_cache_.resize(LASTMUSIC);
        adlib_sfx_cache_.resize(NUMSOUNDS);
        pcm_cache_.resize(NUMSOUNDS);

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

    if (thread_ != NULL) {
        int status;
        ::SDL_WaitThread(thread_, &status);
        thread_ = NULL;
    }

    if (mutex_ != NULL) {
        ::SDL_DestroyMutex(mutex_);
        mutex_ = NULL;
    }

    dst_rate_ = 0;
    mix_samples_count_ = 0;
    Samples().swap(buffer_);
    MixSamples().swap(mix_buffer_);
    is_data_available_ = false;
    quit_thread_ = false;
    Sounds().swap(sounds_);
    PlayCommands().swap(commands_);
    PlayCommands().swap(commands_queue_);
    mute_ = false;
    Cache().swap(adlib_music_cache_);
    Cache().swap(adlib_sfx_cache_);
    Cache().swap(pcm_cache_);
}

bool AudioMixer::is_initialized() const
{
    return is_initialized_;
}

bool AudioMixer::play_adlib_music(
    int music_index,
    const void* data,
    int data_size)
{
    return play_sound(ST_ADLIB_MUSIC, music_index, data, data_size);
}

bool AudioMixer::play_adlib_sound(
    int sound_index,
    const void* data,
    int data_size,
    int actor_index,
    ActorType actor_type,
    ActorChannel actor_channel)
{
    return play_sound(ST_ADLIB_SFX, sound_index, data, data_size,
        actor_index, actor_type, actor_channel);
}

bool AudioMixer::play_pcm_sound(
    int sound_index,
    const void* data,
    int data_size,
    int actor_index,
    ActorType actor_type,
    ActorChannel actor_channel)
{
    return play_sound(ST_PCM, sound_index, data, data_size,
        actor_index, actor_type, actor_channel);
}

bool AudioMixer::stop_music()
{
    if (!is_initialized())
        return false;

    Command command;
    command.command = CMD_STOP_MUSIC;

    ::SDL_LockMutex(mutex_);
    commands_queue_.push_back(command);
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
            static bool test(const Sound& sound)
            {
                return sound.type == ST_ADLIB_MUSIC;
            }
        }; // class Predicate

        result = (std::find_if(
            sounds_.begin(), sounds_.end(), Predicate::test) !=
                sounds_.end());
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
            static bool test(const Sound& sound)
            {
                return sound.type != ST_ADLIB_MUSIC;
            }
        }; // class Predicate

        result = (std::find_if(
            sounds_.begin(), sounds_.end(), Predicate::test) !=
                sounds_.end());
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
        handle_commands();

        if (!mute_ && !is_data_available_ && !sounds_.empty()) {
            mix_samples();
            is_data_available_ = true;
        }

        ::SDL_Delay(1);
    }
}

void AudioMixer::mix_samples()
{
    float min_sample = 32767;
    float max_sample = -32768;

    for (int i = 0; i < mix_samples_count_; ++i) {
        float sample = 0;

        for (SoundsIt sound = sounds_.begin(); sound != sounds_.end(); ) {
            if (!decode_sound(*sound)) {
                sound = sounds_.erase(sound);
                continue;
            }

            CacheItem* cache_item = sound->cache;

            if (sound->decode_offset == cache_item->decoded_count) {
                ++sound;
                continue;
            }

            float scale;

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
                if (cache_item->is_decoded()) {
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

    bool normalize = false;
    float normalize_scale;

    if (min_sample < -32768 && -min_sample > max_sample) {
        normalize = true;
        normalize_scale = -32768.0F / min_sample;
    } else if (max_sample > 32767 && max_sample >= -min_sample) {
        normalize = true;
        normalize_scale = 32767.0F / max_sample;
    }

    for (int i = 0; i < mix_samples_count_; ++i) {
        float sample = mix_buffer_[i];

        if (normalize)
            sample *= normalize_scale;

        sample = std::min(sample, 32767.0F);
        sample = std::max(sample, -32768.0F);

        buffer_[(i * 2) + 0] = static_cast<int16_t>(sample);
        buffer_[(i * 2) + 1] = static_cast<int16_t>(sample);
    }
}

void AudioMixer::handle_commands()
{
    ::SDL_LockMutex(mutex_);

    if (!commands_queue_.empty()) {
        commands_.insert(
            commands_.end(),
            commands_queue_.begin(),
            commands_queue_.end());

        commands_queue_.clear();
    }

    ::SDL_UnlockMutex(mutex_);


    for (PlayCommandsCIt i = commands_.begin();
        i != commands_.end(); ++i)
    {
        switch (i->command) {
        case CMD_PLAY:
            handle_play_command(*i);
            break;

        case CMD_STOP_MUSIC:
            handle_stop_music_command();
            break;
        }
    }

    commands_.clear();
}

void AudioMixer::handle_play_command(
    const Command& command)
{
    CacheItem* cache_item = command.sound.cache;

    if (cache_item == NULL)
        return;

    if (!initialize_cache_item(command, *cache_item))
        return;

    if (command.sound.type != ST_ADLIB_MUSIC) {
        // Remove sounds which will be overritten.

        for (SoundsIt i = sounds_.begin(); i != sounds_.end(); ) {
            if (command.sound.actor_index >= 0 &&
                i->actor_index == command.sound.actor_index &&
                i->actor_type == command.sound.actor_type &&
                i->actor_channel == command.sound.actor_channel)
            {
                i = sounds_.erase(i);
            } else
                ++i;
        }
    }

    Sound sound = command.sound;
    sound.decode_offset = 0;

    sounds_.push_back(sound);
}

void AudioMixer::handle_stop_music_command()
{
    for (SoundsIt i = sounds_.begin(); i != sounds_.end(); ) {
        if (i->type != ST_ADLIB_MUSIC)
            ++i;
        else {
            *(i->cache) = CacheItem();
            i = sounds_.erase(i);
        }
    }
}

bool AudioMixer::initialize_cache_item(
    const Command& command,
    CacheItem& cache_item)
{
    if (cache_item.is_active)
        return !cache_item.is_invalid;

    cache_item = CacheItem();

    bool is_succeed = true;
    AudioDecoder* decoder = NULL;

    if (is_succeed) {
        decoder = create_decoder_by_sound_type(command.sound.type);
        is_succeed = (decoder != NULL);
    }

    if (is_succeed) {
        is_succeed = decoder->initialize(
            command.data, command.data_size, dst_rate_);
    }

    int samples_count = 0;

    if (is_succeed) {
        samples_count = decoder->get_dst_length_in_samples();
        is_succeed = (samples_count > 0);
    }

    cache_item.is_active = true;

    if (is_succeed) {
        cache_item.sound_type = command.sound.type;
        cache_item.samples_count = samples_count;
        cache_item.samples.resize(samples_count);
        cache_item.decoder = decoder;
    } else {
        delete decoder;
        cache_item.is_invalid = true;
    }

    return is_succeed;
}

bool AudioMixer::decode_sound(
    const Sound& sound)
{
    CacheItem* cache_item = sound.cache;

    if (cache_item == NULL)
        return false;

    if (!cache_item->is_active)
        return false;

    if (cache_item->is_invalid)
        return false;

    if (cache_item->is_decoded())
        return true;

    int ahead_count = std::min(
        sound.decode_offset + mix_samples_count_,
        cache_item->samples_count);

    if (ahead_count <= cache_item->decoded_count)
        return true;

    int planned_count = std::min(
        cache_item->samples_count - cache_item->decoded_count,
        mix_samples_count_);

    int actual_count = cache_item->decoder->decode(
        planned_count, &cache_item->samples[cache_item->decoded_count]);

    cache_item->decoded_count += actual_count;

    return true;
}

bool AudioMixer::play_sound(
    SoundType sound_type,
    int sound_index,
    const void* data,
    int data_size,
    int actor_index,
    ActorType actor_type,
    ActorChannel actor_channel)
{
    if (!is_initialized())
        return false;

    if (!is_sound_type_valid(sound_type))
        return false;

    if (data == NULL)
        return false;

    if (data_size <= 0)
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

    Command command;
    command.command = CMD_PLAY;
    command.sound.type = sound_type;
    command.sound.cache = get_cache_item(sound_type, sound_index);
    command.sound.actor_index = actor_index;
    command.sound.actor_type = actor_type;
    command.sound.actor_channel = actor_channel;
    command.data = data;
    command.data_size = data_size;

    ::SDL_LockMutex(mutex_);
    commands_queue_.push_back(command);
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
bool AudioMixer::is_sound_type_valid(
    SoundType sound_type)
{
    switch (sound_type) {
    case ST_ADLIB_MUSIC:
    case ST_ADLIB_SFX:
    case ST_PCM:
        return true;

    default:
        return false;
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
