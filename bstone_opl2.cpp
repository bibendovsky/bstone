//
// A wrapper for the DOSBox OPL2 emulator.
//


#include "bstone_opl2.h"

#include "dbopl.h"


namespace bstone {


class Opl2::Context {
public:
    DBOPL::Handler emulator;
    MixerChannel channel;
}; // class Opl2::Context


Opl2::Opl2() :
    context_(NULL)
{
    set_scale(get_default_scale());
}

Opl2::~Opl2()
{
    uninitialize();
}

void Opl2::initialize(int sample_rate)
{
    uninitialize();

    sample_rate = std::max(sample_rate, get_min_sample_rate());

    context_ = new Context();
    context_->emulator.Init(sample_rate);
    context_->channel.set_scale(get_scale());
}

void Opl2::uninitialize()
{
    if (context_ != NULL) {
        delete context_;
        context_ = NULL;
    }
}

bool Opl2::is_initialized() const
{
    return context_ != NULL;
}

int Opl2::get_scale() const
{
    return scale_;
}

void Opl2::set_scale(int scale)
{
    scale_ = std::max(scale, get_min_scale());

    if (is_initialized())
        context_->channel.set_scale(get_scale());
}

void Opl2::write(int fm_port, int fm_value)
{
    if (!is_initialized())
        return;

    context_->emulator.WriteReg(
        static_cast<Bit32u>(fm_port),
        static_cast<Bit8u>(fm_value));
}

int Opl2::generate(int count, int16_t* buffer)
{
    if (!is_initialized())
        return 0;

    if (count < 1)
        return 0;

    if (buffer == NULL)
        return 0;

    count = std::min(count, get_max_samples_count());

    context_->channel.set_buffer(buffer);

    context_->emulator.Generate(
        &context_->channel,
        static_cast<Bitu>(count));

    return count;
}

// (static)
int Opl2::get_min_sample_rate()
{
    return 8000;
}

// (static)
int Opl2::get_min_scale()
{
    return 1;
}

// (static)
int Opl2::get_default_scale()
{
    return 8;
}

// (static)
int Opl2::get_max_samples_count()
{
    return 512;
}


} // namespace bstone
