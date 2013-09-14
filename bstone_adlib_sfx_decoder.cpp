//
// A decoder for AdLib sound effects.
//


#include "bstone_adlib_sfx_decoder.h"

#include "SDL.h"


namespace bstone {


void AdlibSfxDecoder::Instrument::reset()
{
    m_char = 0;
    c_char = 0;
    m_scale = 0;
    c_scale = 0;
    m_attack = 0;
    c_attack = 0;
    m_sus = 0;
    c_sus = 0;
    m_wave = 0;
    c_wave = 0;
}

AdlibSfxDecoder::AdlibSfxDecoder() :
    AdlibDecoder(),
    reader_(),
    commands_count_(0),
    command_index_(0),
    samples_per_tick_(0),
    remains_count_(0),
    hf_(0)
{
}

// (virtual)
AdlibSfxDecoder::~AdlibSfxDecoder()
{
    uninitialize();
}

// (virtual)
bool AdlibSfxDecoder::initialize(
    const void* raw_data,
    int raw_data_size,
    int sample_rate)
{
    if (!AdlibDecoder::initialize(
        raw_data,
        raw_data_size,
        sample_rate))
    {
        return false;
    }

    reader_.initialize(raw_data, raw_data_size);

    int sfx_length = SDL_SwapLE32(
        reader_.read_s32());

    if ((sfx_length + get_header_size()) >= raw_data_size)
        return false;

    // Skip priority.
    reader_.skip(2);

    instrument_.m_char = reader_.read_u8();
    instrument_.c_char = reader_.read_u8();
    instrument_.m_scale = reader_.read_u8();
    instrument_.c_scale = reader_.read_u8();
    instrument_.m_attack = reader_.read_u8();
    instrument_.c_attack = reader_.read_u8();
    instrument_.m_sus = reader_.read_u8();
    instrument_.c_sus = reader_.read_u8();
    instrument_.m_wave = reader_.read_u8();
    instrument_.c_wave = reader_.read_u8();

    // Skip nConn, voice, mode and 3 unused octets
    reader_.skip(6);

    if (instrument_.m_sus == 0 && instrument_.c_sus == 0)
        return false;

    hf_ = reader_.read_u8();
    hf_ = ((hf_ & 7) << 2) | 0x20;

    initialize_instrument();

    command_index_ = 0;
    commands_count_ = sfx_length;
    samples_per_tick_ = emulator_.get_sample_rate() / get_tick_rate();
    length_in_samples_ = samples_per_tick_ * sfx_length;
    remains_count_ = 0;

    set_is_initialized(true);

    return true;
}

// (virtual)
void AdlibSfxDecoder::uninitialize()
{
    reader_.uninitialize();
    instrument_.reset();
    commands_count_ = 0;
    command_index_ = 0;
    samples_per_tick_ = 0;
    remains_count_ = 0;
    hf_ = 0;

    AdlibDecoder::uninitialize();
}

// (virtual)
bool AdlibSfxDecoder::reset()
{
    if (!AdlibDecoder::reset())
        return false;

    initialize_instrument();

    command_index_ = 0;
    remains_count_ = 0;

    reader_.set_position(get_header_size());

    return true;
}

// (virtual)
int AdlibSfxDecoder::decode(
    int samples_count,
    int16_t* buffer)
{
    if (!is_initialized())
        return 0;

    if (samples_count < 1)
        return 0;

    if (buffer == NULL)
        return 0;

    if (command_index_ == commands_count_ && remains_count_ == 0)
        return 0;

    int decoded_samples_count = 0;

    for (bool quit = false; !quit; ) {
        if (remains_count_ > 0) {
            int count = std::min(samples_count, remains_count_);

            emulator_.generate(count, buffer);

            buffer += count;
            samples_count -= count;
            remains_count_ -= count;
            decoded_samples_count += count;
        } else {
            if (command_index_ < commands_count_) {
                int lf = reader_.read_u8();

                if (lf > 0) {
                    emulator_.write(AL_FREQ_L, lf);
                    emulator_.write(AL_FREQ_H, hf_);
                } else
                    emulator_.write(AL_FREQ_H, 0x00);

                ++command_index_;

                remains_count_ = samples_per_tick_;
            }
        }

        quit =
            (command_index_ == commands_count_ && remains_count_ == 0) ||
            samples_count == 0;
    }

    return decoded_samples_count;
}

void AdlibSfxDecoder::initialize_instrument()
{
    // carrier
    const int c = 3;

    // modifier
    const int m = 0;

    emulator_.write(m + AL_CHAR, instrument_.m_char);
    emulator_.write(m + AL_SCALE, instrument_.m_scale);
    emulator_.write(m + AL_ATTACK, instrument_.m_attack);
    emulator_.write(m + AL_SUS, instrument_.m_sus);
    emulator_.write(m + AL_WAVE, instrument_.m_wave);
    emulator_.write(c + AL_CHAR, instrument_.c_char);
    emulator_.write(c + AL_SCALE, instrument_.c_scale);
    emulator_.write(c + AL_ATTACK, instrument_.c_attack);
    emulator_.write(c + AL_SUS, instrument_.c_sus);
    emulator_.write(c + AL_WAVE, instrument_.c_wave);

    // AL_FEED_CON
    emulator_.write(AL_FEED_CON, 0);
}

// (static)
int AdlibSfxDecoder::get_tick_rate()
{
    return 70;
}

// (static)
int AdlibSfxDecoder::get_header_size()
{
    return 23;
}


} // namespace bstone
