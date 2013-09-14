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

    Context()
    {
    }

    ~Context()
    {
    }

private:
    Context(
        const Context& that);

    Context& operator=(
        const Context& that);
}; // class Opl2::Context


Opl2::Opl2() :
    context_(NULL),
    sample_rate_(0)
{
}

Opl2::~Opl2()
{
    uninitialize();
}

void Opl2::initialize(
    int sample_rate)
{
    uninitialize();

    sample_rate_ = std::max(sample_rate, get_min_sample_rate());

    context_ = new Context();
    context_->emulator.Init(sample_rate_);
}

void Opl2::uninitialize()
{
    delete context_;
    context_ = NULL;

    sample_rate_ = 0;
}

bool Opl2::is_initialized() const
{
    return context_ != NULL;
}

int Opl2::get_sample_rate() const
{
    return sample_rate_;
}

void Opl2::write(
    int fm_port,
    int fm_value)
{
    if (!is_initialized())
        return;

    context_->emulator.WriteReg(
        static_cast<Bit32u>(fm_port),
        static_cast<Bit8u>(fm_value));
}

bool Opl2::generate(
    int count,
    int16_t* buffer)
{
    if (!is_initialized())
        return false;

    if (count < 1)
        return false;

    if (buffer == NULL)
        return false;

    while (count > 0) {
        int generate_count = std::min(count, get_max_samples_count());

        context_->channel.set_buffer(buffer);

        context_->emulator.Generate(
            &context_->channel,
            static_cast<Bitu>(generate_count));

        count -= generate_count;
        buffer += generate_count;
    }

    return true;
}

bool Opl2::reset()
{
    if (!is_initialized())
        return false;

    context_->emulator.Init(get_sample_rate());

    return true;
}

// (static)
int Opl2::get_min_sample_rate()
{
    return 8000;
}

// (static)
int Opl2::get_max_samples_count()
{
    return 512;
}


} // namespace bstone
