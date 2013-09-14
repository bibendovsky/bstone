//
// A wrapper for the DOSBox OPL2 emulator.
//


#ifndef BSTONE_OPL2_H
#define BSTONE_OPL2_H


#include <cstdint>


namespace bstone {


// A wrapper for DOSBox OPL2 emulator.
class Opl2 {
public:
    Opl2();

    ~Opl2();

    // Initializes the emulator with a specified rate for output samples.
    void initialize(
        int sample_rate);

    // Uninitializes the emulator.
    void uninitialize();

    // Returns a status of emulator:
    // true if it initialized or false otherwise.
    bool is_initialized() const;

    // Returns a sample rate for output samples.
    int get_sample_rate() const;

    // Writes a value into a register.
    // Returns false on error.
    void write(
        int fm_port,
        int fm_value);

    // Generates number of mono samples into a specified buffer.
    // Returns false on error.
    bool generate(
        int count,
        int16_t* buffer);

    // Resets the emulator.
    bool reset();

    // Returns a minimum rate for output samples.
    // (Emulator dependant value)
    static int get_min_sample_rate();

private:
    class Context;

    int sample_rate_;
    Context* context_;

    Opl2(
        const Opl2& that);

    Opl2& operator=(
        const Opl2& that);

    // Returns a maximum number of output samples generated at once.
    // (Emulator dependant value)
    static int get_max_samples_count();
}; // class Opl2


} // namespace bstone


#endif // BSTONE_OPL2_H
