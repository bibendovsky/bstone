//
// A stub for DOSBox OPL2 emulator.
//


#ifndef BSTONE_DOSBOX_MIXER_H
#define BSTONE_DOSBOX_MIXER_H


#include <algorithm>

#include "dosbox.h"


typedef void (*MIXER_Handler)(uintptr_t len);


class MixerChannel {
public:
    MixerChannel()
    {
        set_buffer(NULL);
        set_scale(get_default_scale());
    }

    void AddSamples_m32(uintptr_t count, const int32_t* buffer)
    {
        if (count < 1)
            return;

        if (buffer == NULL)
            return;

        assert(buffer_ != NULL);

        if (buffer_ == NULL)
            return;

        for (uintptr_t i = 0; i < count; ++i) {
            int32_t value = scale_ * buffer[i];

            value = std::min(value, get_max_sample_value());
            value = std::max(value, get_min_sample_value());

            buffer_[i] = static_cast<int16_t>(value);
        }
    }

    void AddSamples_s32(uintptr_t, const int32_t*)
    {
        assert(!"AddSamples_s32 not implemented.");
    }

    // Sets a buffer to write data to.
    void set_buffer(int16_t* buffer)
    {
        buffer_ = buffer;
    }

    // Sets a scale for output samples.
    void set_scale(int scale)
    {
        scale_ = std::max(scale, get_min_scale());
    }

    static int get_min_scale()
    {
        return 1;
    }

    static int get_default_scale()
    {
        return get_min_scale();
    }

    static int get_min_sample_value()
    {
        return -32768;
    }

    static int get_max_sample_value()
    {
        return 32767;
    }

private:
    int16_t* buffer_;
    int scale_;
}; // class MixerChannel


class MixerObject {
public:
    MixerChannel* Install(
        MIXER_Handler handler,
        Bitu freq,
        const char* name);
}; // class MixerObject


#endif // BSTONE_DOSBOX_MIXER_H
