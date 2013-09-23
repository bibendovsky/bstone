#include "bstone_audio_mixer.h"

#include <cassert>

#include <algorithm>
#include <functional>

#include "3d_def.h"

#include "bstone_adlib_music_decoder.h"
#include "bstone_adlib_sfx_decoder.h"
#include "bstone_pcm_decoder.h"


const int ATABLEMAX = 15;

extern boolean sqPlayedOnce;
extern Uint8 lefttable[ATABLEMAX][ATABLEMAX * 2];
extern Uint8 righttable[ATABLEMAX][ATABLEMAX * 2];

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

bool AudioMixer::Sound::is_audible() const
{
    return left_volume > 0.0F && right_volume > 0.0F;
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
    mute_(false),
    is_music_playing_(false),
    is_any_sfx_playing_(false)
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
        mix_buffer_.resize(total_samples);

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
    player_channels_state_ = 0;
    is_music_playing_ = false;
    is_any_sfx_playing_ = false;
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
    return play_sound(ST_ADLIB_MUSIC, 0, music_index, data, data_size);
}

bool AudioMixer::play_adlib_sound(
    int sound_index,
    int priority,
    const void* data,
    int data_size,
    int actor_index,
    ActorType actor_type,
    ActorChannel actor_channel)
{
    return play_sound(ST_ADLIB_SFX, priority, sound_index,
        data, data_size, actor_index, actor_type, actor_channel);
}

bool AudioMixer::play_pcm_sound(
    int sound_index,
    int priority,
    const void* data,
    int data_size,
    int actor_index,
    ActorType actor_type,
    ActorChannel actor_channel)
{
    return play_sound(ST_PCM, priority, sound_index,
        data, data_size, actor_index, actor_type, actor_channel);
}

bool AudioMixer::update_positions()
{
    if (!is_initialized())
        return false;

    PositionsState state;

    state.player.view_x = viewx;
    state.player.view_y = viewy;
    state.player.view_cos = viewcos;
    state.player.view_sin = viewsin;

    state.actors.resize(MAXACTORS);

    for (int i = 0; i < MAXACTORS; ++i) {
        Position& actor = state.actors[i];

        actor.x = objlist[i].x;
        actor.y = objlist[i].y;
    }

    state.doors.resize(MAXDOORS);

    for (int i = 0; i < MAXDOORS; ++i) {
        Position& door = state.doors[i];

        door.x = (doorobjlist[i].tilex << TILESHIFT) +
            (1 << (TILESHIFT - 1));

        door.y = (doorobjlist[i].tiley << TILESHIFT) +
            (1 << (TILESHIFT - 1));
    }

    state.wall.x = (pwallx << TILESHIFT) + (1 << (TILESHIFT - 1));
    state.wall.y = (pwally << TILESHIFT) + (1 << (TILESHIFT - 1));

    int wall_offset = (65535 * pwallpos) / 63;

    switch (pwalldir) {
    case di_east:
        state.wall.x += wall_offset;
        break;

    case di_north:
        state.wall.y -= wall_offset;
        break;

    case di_south:
        state.wall.y += wall_offset;
        break;

    case di_west:
        state.wall.x -= wall_offset;
        break;
    }

    ::SDL_LockMutex(mutex_);
    positions_state_queue_.push_back(state);
    ::SDL_UnlockMutex(mutex_);

    return true;
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

    return is_music_playing_;
}

bool AudioMixer::is_any_sfx_playing() const
{
    if (!is_initialized())
        return false;

    return is_any_sfx_playing_;
}

bool AudioMixer::is_player_channel_playing(
    ActorChannel channel) const
{
    return (player_channels_state_ & (1 << channel)) != 0;
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
    spatialize_sounds();

    float min_left_sample = 32767;
    float max_left_sample = -32768;

    float min_right_sample = 32767;
    float max_right_sample = -32768;

    for (int i = 0; i < mix_samples_count_; ++i) {
        float left_sample = 0.0F;
        float right_sample = 0.0F;

        for (SoundsIt sound = sounds_.begin(); sound != sounds_.end(); ) {
            if (!decode_sound(*sound)) {
                set_player_channel_state(*sound, false);
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

            if (sound->is_audible()) {
                float sample = scale *
                    cache_item->samples[sound->decode_offset];

                left_sample += sound->left_volume * sample;
                right_sample += sound->right_volume * sample;
            }

            ++sound->decode_offset;

            if (sound->decode_offset == cache_item->decoded_count) {
                if (cache_item->is_decoded()) {
                    if (sound->type == ST_ADLIB_MUSIC) {
                        sqPlayedOnce = true;
                        sound->decode_offset = 0;
                    } else {
                        set_player_channel_state(*sound, false);
                        sound = sounds_.erase(sound);
                        continue;
                    }
                }
            }

            ++sound;
        }

        mix_buffer_[(2 * i) + 0] = left_sample;
        min_left_sample = std::min(left_sample, min_left_sample);
        max_left_sample = std::max(left_sample, max_left_sample);

        mix_buffer_[(2 * i) + 1] = right_sample;
        min_right_sample = std::min(right_sample, min_right_sample);
        max_right_sample = std::max(right_sample, max_right_sample);
    }

    //
    // Calculate normalizations factors.
    //

    bool normalize_left = false;
    float normalize_left_scale = 1.0F;

    if (min_left_sample < -32768 &&
        -min_left_sample > max_left_sample)
    {
        normalize_left = true;
        normalize_left_scale = -32768.0F / min_left_sample;
    } else if (max_left_sample > 32767 &&
        max_left_sample >= -min_left_sample)
    {
        normalize_left = true;
        normalize_left_scale = 32767.0F / max_left_sample;
    }

    bool normalize_right = false;
    float normalize_right_scale = 1.0F;

    if (min_right_sample < -32768 &&
        -min_right_sample > max_right_sample)
    {
        normalize_right = true;
        normalize_right_scale = -32768.0F / min_right_sample;
    } else if (max_right_sample > 32767 &&
        max_right_sample >= -min_right_sample)
    {
        normalize_right = true;
        normalize_right_scale = 32767.0F / max_right_sample;
    }


    //
    // Normalize and output.
    //

    for (int i = 0; i < mix_samples_count_; ++i) {
        float left_sample = mix_buffer_[(2 * i) + 0];
        float right_sample = mix_buffer_[(2 * i) + 1];

        if (normalize_left)
            left_sample *= normalize_left_scale;

        left_sample = std::min(left_sample, 32767.0F);
        left_sample = std::max(left_sample, -32768.0F);

        if (normalize_right)
            right_sample *= normalize_right_scale;

        right_sample = std::min(right_sample, 32767.0F);
        right_sample = std::max(right_sample, -32768.0F);

        buffer_[(2 * i) + 0] = static_cast<int16_t>(left_sample);
        buffer_[(2 * i) + 1] = static_cast<int16_t>(right_sample);
    }

    int music_count = is_music_playing() ? 1 : 0;
    is_any_sfx_playing_ = ((sounds_.size() - music_count) > 0);
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

    if (!positions_state_queue_.empty()) {
        positions_state_ = positions_state_queue_.back();
        positions_state_queue_.clear();
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

    if (command.sound.type != ST_ADLIB_MUSIC &&
        command.sound.actor_index >= 0)
    {
        // Search existing sound which can override a
        // new one because of priority.

        for (SoundsIt i = sounds_.begin(); i != sounds_.end(); ++i) {
            if (i->priority > command.sound.priority &&
                i->actor_index == command.sound.actor_index &&
                i->actor_type == command.sound.actor_type &&
                i->actor_channel == command.sound.actor_channel)
            {
                return;
            }
        }

        // Remove sounds which will be overritten.

        for (SoundsIt i = sounds_.begin(); i != sounds_.end(); ) {
            if (i->actor_index == command.sound.actor_index &&
                i->actor_type == command.sound.actor_type &&
                i->actor_channel == command.sound.actor_channel)
            {
                set_player_channel_state(*i, false);
                i = sounds_.erase(i);
            } else
                ++i;
        }
    }

    if (command.sound.type == ST_ADLIB_MUSIC)
        is_music_playing_ = true;
    else
        is_any_sfx_playing_ = true;

    Sound sound = command.sound;
    sound.decode_offset = 0;
    sounds_.push_back(sound);

    set_player_channel_state(sound, true);
}

void AudioMixer::handle_stop_music_command()
{
    is_music_playing_ = false;

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

void AudioMixer::spatialize_sound(
    Sound& sound)
{
    sound.left_volume = 1.0F;
    sound.right_volume = 1.0F;

    if (sound.type == ST_ADLIB_MUSIC)
        return;

    if (sound.actor_index <= 0)
        return;

    Position* position = NULL;

    switch (sound.actor_type) {
    case AT_ACTOR:
        position = &positions_state_.actors[sound.actor_index];
        break;

    case AT_DOOR:
        position = &positions_state_.doors[sound.actor_index];
        break;

    case AT_WALL:
        position = &positions_state_.wall;
        break;

    default:
        return;
    }

    int gx = position->x;
    int gy = position->y;

    //
    // translate point to view centered coordinates
    //
    gx -= positions_state_.player.view_x;
    gy -= positions_state_.player.view_y;

    //
    // calculate newx
    //
    int xt = ::FixedByFrac(gx, positions_state_.player.view_cos);
    int yt = ::FixedByFrac(gy, positions_state_.player.view_sin);
    int x = (xt - yt) >> TILESHIFT;

    //
    // calculate newy
    //
    xt = ::FixedByFrac(gx, positions_state_.player.view_sin);
    yt = ::FixedByFrac(gy, positions_state_.player.view_cos);
    int y = (yt + xt) >> TILESHIFT;

    if (y >= ATABLEMAX)
        y = ATABLEMAX - 1;
    else if (y <= -ATABLEMAX)
        y = -ATABLEMAX;

    if (x < 0)
        x = -x;
    if (x >= ATABLEMAX)
        x = ATABLEMAX - 1;

    int left = 9 - lefttable[x][y + ATABLEMAX];
    int right = 9 - righttable[x][y + ATABLEMAX];

    sound.left_volume = left / 9.0F;
    sound.right_volume = right / 9.0F;
}

void AudioMixer::spatialize_sounds()
{
    for (SoundsIt i = sounds_.begin(); i != sounds_.end(); ++i)
        spatialize_sound(*i);
}

bool AudioMixer::play_sound(
    SoundType sound_type,
    int priority,
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

    if (priority < 0)
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
    case AC_HIT_WALL:
    case AC_NO_WAY:
        break;

    default:
        return false;
    }

    Command command;
    command.command = CMD_PLAY;
    command.sound.type = sound_type;
    command.sound.priority = priority;
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

void AudioMixer::set_player_channel_state(
    const Sound& sound,
    bool state)
{
    if (sound.type == ST_ADLIB_MUSIC)
        return;

    if (sound.actor_type != AT_ACTOR)
        return;

    if (sound.actor_index > 0)
        return;

    int mask = 1 << sound.actor_channel;

    if (state)
        player_channels_state_ |= mask;
    else
        player_channels_state_ &= ~mask;
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
