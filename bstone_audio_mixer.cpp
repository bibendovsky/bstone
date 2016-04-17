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


#include "bstone_audio_mixer.h"
#include <cassert>
#include <algorithm>
#include <functional>
#include "3d_def.h"
#include "bstone_adlib_music_decoder.h"
#include "bstone_adlib_sfx_decoder.h"
#include "bstone_pcm_decoder.h"


const int ATABLEMAX = 15;

extern bool sqPlayedOnce;
extern uint8_t lefttable[ATABLEMAX][ATABLEMAX * 2];
extern uint8_t righttable[ATABLEMAX][ATABLEMAX * 2];


namespace bstone {


AudioMixer::CacheItem::CacheItem() :
        is_active(),
        is_invalid(),
        sound_type(ST_NONE),
        samples_count(),
        decoded_count(),
        decoder()
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
    if (that.decoder) {
        decoder = that.decoder->clone();
    } else {
        decoder = nullptr;
    }
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

        if (that.decoder) {
            decoder = that.decoder->clone();
        } else {
            decoder = nullptr;
        }
    }

    return *this;
}

bool AudioMixer::CacheItem::is_decoded() const
{
    return decoded_count == samples_count;
}

void AudioMixer::Positions::initialize()
{
    player = {};

    actors.clear();
    actors.resize(MAXACTORS);

    doors.clear();
    doors.resize(MAXDOORS);

    wall = {};
}

void AudioMixer::Positions::fixed_copy_to(
    Positions& target)
{
    player = target.player;

    std::copy(actors.cbegin(), actors.cend(), target.actors.begin());
    std::copy(doors.cbegin(), doors.cend(), target.doors.begin());

    wall = target.wall;
}

bool AudioMixer::Sound::is_audible() const
{
    return left_volume > 0.0F && right_volume > 0.0F;
}

AudioMixer::AudioMixer() :
        is_initialized_(),
        dst_rate_(),
        device_id_(),
#if BSTONE_AUDIO_MIXER_USE_THREAD
        mutex_(),
        thread_(),
#endif
        mix_samples_count_(),
        buffer_(),
        mix_buffer_(),
        is_data_available_(),
#if BSTONE_AUDIO_MIXER_USE_THREAD
        quit_thread_(),
#endif
        sounds_(),
        commands_(),
        mute_(),
        adlib_music_cache_(),
        adlib_sfx_cache_(),
        pcm_cache_(),
        positions_(),
        player_channels_state_(),
        is_music_playing_(),
        is_any_sfx_playing_(),
        sfx_volume_(),
        music_volume_(),
        mix_size_ms_()
{
    // Initialize atomic fields
    //

    is_data_available_ = false;
#if BSTONE_AUDIO_MIXER_USE_THREAD
    quit_thread_ = false;
#endif
    player_channels_state_ = 0;
    is_music_playing_ = false;
    is_any_sfx_playing_ = false;
    sfx_volume_ = 1.0F;
    music_volume_ = 1.0F;
}

AudioMixer::~AudioMixer()
{
    uninitialize();
}

bool AudioMixer::initialize(
    int dst_rate,
    int mix_size_ms)
{
    uninitialize();

    if (dst_rate == 0) {
        dst_rate_ = get_default_rate();
    } else {
        dst_rate_ = std::max(dst_rate, get_min_rate());
    }

    if (mix_size_ms == 0) {
        mix_size_ms_ = get_default_mix_size_ms();
    } else {
        mix_size_ms_ = std::max(mix_size_ms, get_min_mix_size_ms());
    }

    mix_samples_count_ = calculate_mix_samples_count(
        dst_rate_,
        mix_size_ms_);

    SDL_AudioSpec src_spec;
    src_spec.freq = dst_rate_;
    src_spec.format = AUDIO_S16SYS;
    src_spec.channels = static_cast<uint8_t>(get_max_channels());
    src_spec.samples = static_cast<uint16_t>(mix_samples_count_);
    src_spec.callback = callback_proxy;
    src_spec.userdata = this;

    SDL_AudioSpec dst_spec;

    device_id_ = ::SDL_OpenAudioDevice(
        nullptr,
        0,
        &src_spec,
        &dst_spec,
        SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    if (device_id_ == 0) {
        return false;
    }

    bool is_succeed = true;


    if (is_succeed) {
        positions_.initialize();
    }

#if BSTONE_AUDIO_MIXER_USE_THREAD
    std::thread thread;

    if (is_succeed) {
        thread = std::thread(mix_proxy, this);
    }
#endif

    if (is_succeed) {
        is_initialized_ = true;
        mix_samples_count_ = dst_spec.samples;

        int total_samples = get_max_channels() * mix_samples_count_;

        buffer_.resize(total_samples);
        mix_buffer_.resize(total_samples);

        adlib_music_cache_.resize(LASTMUSIC);
        adlib_sfx_cache_.resize(NUMSOUNDS);
        pcm_cache_.resize(NUMSOUNDS);

        commands_.initialize(get_max_commands());

#if BSTONE_AUDIO_MIXER_USE_THREAD
        thread_ = std::move(thread);
#endif

        ::SDL_PauseAudioDevice(device_id_, 0);
    } else {
        uninitialize();
    }

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

#if BSTONE_AUDIO_MIXER_USE_THREAD
    if (thread_.joinable()) {
        quit_thread_ = true;
        thread_.join();
    }
#endif

    dst_rate_ = 0;
    mix_samples_count_ = 0;
    Samples().swap(buffer_);
    MixSamples().swap(mix_buffer_);
    is_data_available_ = false;
#if BSTONE_AUDIO_MIXER_USE_THREAD
    quit_thread_ = false;
#endif
    Sounds().swap(sounds_);
    commands_.uninitialize();
    mute_ = false;
    Cache().swap(adlib_music_cache_);
    Cache().swap(adlib_sfx_cache_);
    Cache().swap(pcm_cache_);
    player_channels_state_ = 0;
    is_music_playing_ = false;
    is_any_sfx_playing_ = false;
    mix_size_ms_ = 0;
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
    return play_sound(
        ST_ADLIB_MUSIC,
        0,
        music_index,
        data,
        data_size);
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
    return play_sound(
        ST_ADLIB_SFX,
        priority,
        sound_index,
        data,
        data_size,
        actor_index,
        actor_type,
        actor_channel);
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
    return play_sound(
        ST_PCM,
        priority,
        sound_index,
        data,
        data_size,
        actor_index,
        actor_type,
        actor_channel);
}

bool AudioMixer::update_positions()
{
    if (!is_initialized()) {
        return false;
    }


    auto&& state = positions_;

    state.player.view_x = viewx;
    state.player.view_y = viewy;
    state.player.view_cos = viewcos;
    state.player.view_sin = viewsin;

    state.actors.resize(MAXACTORS);

    for (int i = 0; i < MAXACTORS; ++i) {
        auto& actor = state.actors[i];

        actor.x = objlist[i].x;
        actor.y = objlist[i].y;
    }

    state.doors.resize(MAXDOORS);

    for (int i = 0; i < MAXDOORS; ++i) {
        auto& door = state.doors[i];

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

    default:
        break;
    }

    return true;
}

bool AudioMixer::stop_music()
{
    if (!is_initialized()) {
        return false;
    }

    commands_.push(
        [] (Command& command)
        {
            command.command = CMD_STOP_MUSIC;
        }
    );

    return true;
}

bool AudioMixer::stop_all_sfx()
{
    if (!is_initialized()) {
        return false;
    }

    commands_.push(
        [] (Command& command)
        {
            command.command = CMD_STOP_ALL_SFX;
        }
    );

    return true;
}

bool AudioMixer::set_mute(
    bool value)
{
    if (!is_initialized()) {
        return false;
    }

    mute_ = value;

    return true;
}

bool AudioMixer::set_sfx_volume(
    float volume)
{
    if (!is_initialized()) {
        return false;
    }

    if (volume < 0.0F) {
        volume = 0.0F;
    }

    if (volume > 1.0F) {
        volume = 1.0F;
    }

    sfx_volume_ = volume;

    return true;
}

bool AudioMixer::set_music_volume(
    float volume)
{
    if (!is_initialized()) {
        return false;
    }

    if (volume < 0.0F) {
        volume = 0.0F;
    }

    if (volume > 1.0F) {
        volume = 1.0F;
    }

    music_volume_ = volume;

    return true;
}

bool AudioMixer::is_music_playing() const
{
    if (!is_initialized()) {
        return false;
    }

    return is_music_playing_;
}

bool AudioMixer::is_any_sfx_playing() const
{
    if (!is_initialized()) {
        return false;
    }

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
int AudioMixer::get_default_rate()
{
    return 44100;
}

// (static)
int AudioMixer::get_min_mix_size_ms()
{
    return 20;
}

// (static)
int AudioMixer::get_default_mix_size_ms()
{
    return 40;
}

// (static)
int AudioMixer::get_max_channels()
{
    return 2;
}

// (static)
int AudioMixer::get_max_commands()
{
    return 128;
}

void AudioMixer::callback(
    uint8_t* dst_data,
    int dst_length)
{
    if (!mute_ && is_data_available_) {
        std::uninitialized_copy_n(
            reinterpret_cast<const uint8_t*>(buffer_.data()),
            dst_length,
            dst_data);
    } else {
        std::uninitialized_fill_n(dst_data, dst_length, 0);
    }

    is_data_available_ = false;
}

void AudioMixer::mix()
{
#if BSTONE_AUDIO_MIXER_USE_THREAD
    while (!quit_thread_) {
#endif
        handle_commands();

        if (!is_data_available_ && !sounds_.empty()) {
            mix_samples();
            is_data_available_ = true;
#if BSTONE_AUDIO_MIXER_USE_THREAD
        } else {
            ::sys_default_sleep_for();
#endif
        }
#if BSTONE_AUDIO_MIXER_USE_THREAD
    }
#endif
}

void AudioMixer::mix_samples()
{
    spatialize_sounds();

    float sfx_volume = sfx_volume_;
    float music_volume = music_volume_;

    float min_left_sample = 32767;
    float max_left_sample = -32768;

    float min_right_sample = 32767;
    float max_right_sample = -32768;

    for (int i = 0; i < mix_samples_count_; ++i) {
        float left_sample = 0.0F;
        float right_sample = 0.0F;

        for (auto sound = sounds_.begin(); sound != sounds_.end(); ) {
            if (!decode_sound(*sound)) {
                set_player_channel_state(*sound, false);
                sound = sounds_.erase(sound);
                continue;
            }

            auto cache_item = sound->cache;

            if (sound->decode_offset == cache_item->decoded_count) {
                ++sound;
                continue;
            }

            float volume_scale;

            switch (sound->type) {
            case ST_ADLIB_MUSIC:
                volume_scale = 8.0F * music_volume;
                break;

            case ST_ADLIB_SFX:
                volume_scale = 8.0F * sfx_volume;
                break;

            default:
                volume_scale = sfx_volume;
                break;
            }

            if (sound->is_audible()) {
                float sample = volume_scale *
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

    if (min_left_sample < -32768 && -min_left_sample > max_left_sample) {
        normalize_left = true;
        normalize_left_scale = -32768.0F / min_left_sample;
    } else if (max_left_sample > 32767 && max_left_sample >= -min_left_sample) {
        normalize_left = true;
        normalize_left_scale = 32767.0F / max_left_sample;
    }

    bool normalize_right = false;
    float normalize_right_scale = 1.0F;

    if (min_right_sample < -32768 && -min_right_sample > max_right_sample) {
        normalize_right = true;
        normalize_right_scale = -32768.0F / min_right_sample;
    } else if (max_right_sample > 32767 && max_right_sample >= -min_right_sample) {
        normalize_right = true;
        normalize_right_scale = 32767.0F / max_right_sample;
    }


    //
    // Normalize and output.
    //

    for (int i = 0; i < mix_samples_count_; ++i) {
        float left_sample = mix_buffer_[(2 * i) + 0];
        float right_sample = mix_buffer_[(2 * i) + 1];

        if (normalize_left) {
            left_sample *= normalize_left_scale;
        }

        left_sample = std::min(left_sample, 32767.0F);
        left_sample = std::max(left_sample, -32768.0F);

        if (normalize_right) {
            right_sample *= normalize_right_scale;
        }

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
    Command command;

    while (commands_.pop(command)) {
        switch (command.command) {
        case CMD_PLAY:
            handle_play_command(command);
            break;

        case CMD_STOP_MUSIC:
            handle_stop_music_command();
            break;

        default:
            break;
        }
    }
}

void AudioMixer::handle_play_command(
    const Command& command)
{
    auto cache_item = command.sound.cache;

    if (!cache_item) {
        return;
    }

    if (!initialize_cache_item(command, *cache_item)) {
        return;
    }

    if (command.sound.type != ST_ADLIB_MUSIC &&
        command.sound.actor_index >= 0)
    {
        // Search existing sound which can override a
        // new one because of priority.

        for (const auto& sound : sounds_) {
            if (sound.priority > command.sound.priority &&
                sound.actor_index == command.sound.actor_index &&
                sound.actor_type == command.sound.actor_type &&
                sound.actor_channel == command.sound.actor_channel)
            {
                return;
            }
        }

        // Remove sounds which will be overwritten.

        for (auto i = sounds_.begin(); i != sounds_.end(); ) {
            if (i->actor_index == command.sound.actor_index &&
                i->actor_type == command.sound.actor_type &&
                i->actor_channel == command.sound.actor_channel)
            {
                set_player_channel_state(*i, false);
                i = sounds_.erase(i);
            } else {
                ++i;
            }
        }
    }

    if (command.sound.type == ST_ADLIB_MUSIC) {
        is_music_playing_ = true;
    } else {
        is_any_sfx_playing_ = true;
    }

    auto sound = command.sound;
    sound.decode_offset = 0;
    sounds_.push_back(sound);

    set_player_channel_state(sound, true);
}

void AudioMixer::handle_stop_music_command()
{
    is_music_playing_ = false;

    for (auto i = sounds_.begin(); i != sounds_.end(); ) {
        if (i->type != ST_ADLIB_MUSIC) {
            ++i;
        } else {
            *(i->cache) = CacheItem();
            i = sounds_.erase(i);
        }
    }
}

void AudioMixer::handle_stop_all_sfx_command()
{
    is_any_sfx_playing_ = false;

    sounds_.remove_if(
        [] (const Sound& sound)
        {
            return sound.type != ST_ADLIB_MUSIC;
        }
    );
}

bool AudioMixer::initialize_cache_item(
    const Command& command,
    CacheItem& cache_item)
{
    if (cache_item.is_active) {
        return !cache_item.is_invalid;
    }

    cache_item = CacheItem();

    bool is_succeed = true;
    AudioDecoder* decoder = nullptr;

    if (is_succeed) {
        decoder = create_decoder_by_sound_type(command.sound.type);
        is_succeed = (decoder != nullptr);
    }

    if (is_succeed) {
        is_succeed = decoder->initialize(
            command.data,
            command.data_size,
            dst_rate_);
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
    auto cache_item = sound.cache;

    if (!cache_item) {
        return false;
    }

    if (!cache_item->is_active) {
        return false;
    }

    if (cache_item->is_invalid) {
        return false;
    }

    if (cache_item->is_decoded()) {
        return true;
    }

    int ahead_count = std::min(
        sound.decode_offset + mix_samples_count_,
        cache_item->samples_count);

    if (ahead_count <= cache_item->decoded_count)
        return true;

    int planned_count = std::min(
        cache_item->samples_count - cache_item->decoded_count,
        mix_samples_count_);

    int actual_count = cache_item->decoder->decode(
        planned_count,
        &cache_item->samples[cache_item->decoded_count]);

    cache_item->decoded_count += actual_count;

    return true;
}

void AudioMixer::spatialize_sound(
    Sound& sound)
{
    sound.left_volume = 1.0F;
    sound.right_volume = 1.0F;

    if (sound.type == ST_ADLIB_MUSIC) {
        return;
    }

    if (sound.actor_index <= 0) {
        return;
    }

    Location* location = nullptr;

    switch (sound.actor_type) {
    case AT_ACTOR:
        location = &positions_.actors[sound.actor_index];
        break;

    case AT_DOOR:
        location = &positions_.doors[sound.actor_index];
        break;

    case AT_WALL:
        location = &positions_.wall;
        break;

    default:
        return;
    }

    int gx = location->x;
    int gy = location->y;

    //
    // translate point to view centered coordinates
    //
    gx -= positions_.player.view_x;
    gy -= positions_.player.view_y;

    //
    // calculate newx
    //
    int xt = ::FixedByFrac(gx, positions_.player.view_cos);
    int yt = ::FixedByFrac(gy, positions_.player.view_sin);
    int x = (xt - yt) >> TILESHIFT;

    //
    // calculate newy
    //
    xt = ::FixedByFrac(gx, positions_.player.view_sin);
    yt = ::FixedByFrac(gy, positions_.player.view_cos);
    int y = (yt + xt) >> TILESHIFT;

    if (y >= ATABLEMAX) {
        y = ATABLEMAX - 1;
    } else if (y <= -ATABLEMAX) {
        y = -ATABLEMAX;
    }

    if (x < 0) {
        x = -x;
    } if (x >= ATABLEMAX) {
        x = ATABLEMAX - 1;
    }

    int left = 9 - lefttable[x][y + ATABLEMAX];
    int right = 9 - righttable[x][y + ATABLEMAX];

    sound.left_volume = left / 9.0F;
    sound.right_volume = right / 9.0F;
}

void AudioMixer::spatialize_sounds()
{
    for (auto& sound : sounds_) {
        spatialize_sound(sound);
    }
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
    if (!is_initialized()) {
        return false;
    }

    if (!is_sound_type_valid(sound_type)) {
        return false;
    }

    if (priority < 0) {
        return false;
    }

    if (!data) {
        return false;
    }

    if (data_size <= 0) {
        return false;
    }

    if (actor_index >= MAXACTORS) {
        return false;
    }

    switch (actor_channel) {
    case AC_VOICE:
    case AC_WEAPON:
    case AC_ITEM:
    case AC_HIT_WALL:
    case AC_NO_WAY:
    case AC_INTERROGATION:
        break;

    default:
        throw std::invalid_argument("Invalid actor channel.");
    }

    commands_.push(
        [&] (Command& command)
        {
            command.command = CMD_PLAY;
            command.sound.type = sound_type;
            command.sound.priority = priority;
            command.sound.cache = get_cache_item(sound_type, sound_index);
            command.sound.actor_index = actor_index;
            command.sound.actor_type = actor_type;
            command.sound.actor_channel = actor_channel;
            command.data = data;
            command.data_size = data_size;
        }
    );

    return true;
}

void AudioMixer::lock()
{
#if BSTONE_AUDIO_MIXER_USE_THREAD
    mutex_.lock();
#else
    ::SDL_LockAudioDevice(device_id_);
#endif
}

void AudioMixer::unlock()
{
#if BSTONE_AUDIO_MIXER_USE_THREAD
    mutex_.unlock();
#else
    ::SDL_UnlockAudioDevice(device_id_);
#endif
}

// (static)
void AudioMixer::callback_proxy(
    void* user_data,
    uint8_t* dst_data,
    int dst_length)
{
    assert(user_data);

    auto mixer = static_cast<AudioMixer*>(user_data);
#if !BSTONE_AUDIO_MIXER_USE_THREAD
    mixer->mix();
#endif
    mixer->callback(dst_data, dst_length);
}

// (static)
int AudioMixer::mix_proxy(
    void* user_data)
{
    assert(user_data);

    auto mixer = static_cast<AudioMixer*>(user_data);
    mixer->mix();

    return 0;
}

// (static)
int AudioMixer::calculate_mix_samples_count(
    int dst_rate,
    int mix_size_ms)
{
    int exact_count = (dst_rate * mix_size_ms) / 1000;
    int actual_count = 1;

    while (actual_count < exact_count) {
        actual_count *= 2;
    }

    if (actual_count > 65536) {
        actual_count = 65536;
    }

    return actual_count;
}

AudioMixer::CacheItem* AudioMixer::get_cache_item(
    SoundType sound_type,
    int sound_index)
{
    if (!is_sound_index_valid(sound_index, sound_type))
        return nullptr;

    switch (sound_type) {
    case ST_ADLIB_MUSIC:
        return &adlib_music_cache_[sound_index];

    case ST_ADLIB_SFX:
        return &adlib_sfx_cache_[sound_index];

    case ST_PCM:
        return &pcm_cache_[sound_index];

    default:
        return nullptr;
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
        return nullptr;
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


} // bstone
