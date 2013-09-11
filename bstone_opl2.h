//
// A wrapper for the DOSBox OPL2 emulator.
//


#ifndef BSTONE_OPL2_H
#define BSTONE_OPL2_H


#include <cstdint>


namespace bstone {


class Opl2 {
public:
    Opl2();
    ~Opl2();

    // Initializes the emulator with a specified rate for output samples.
    void initialize(int sample_rate);

    // Uninitializes the emulator.
    void uninitialize();

    // Returns a status of emulator:
    // true if it initialized or false otherwise.
    bool is_initialized() const;

    // Returns a current scale for output samples.
    int get_scale() const;

    // Scales output samples by a specified factor.
    void set_scale(int scale);

    // Writes a value into a register.
    // Returns false on error.
    void write(int fm_port, int fm_value);

    // Generates number of mono samples into a specified buffer.
    // Returns number of generated samples.
    int generate(int count, int16_t* buffer);

    // A minimum rate for output samples.
    static int get_min_sample_rate();

    // A minimum scale for output samples.
    static int get_min_scale();

    // A default scale for output samples.
    static int get_default_scale();

    // A maximum number of output samples generated at once.
    static int get_max_samples_count();

private:
    class Context;

    int scale_;
    Context* context_;

    Opl2(const Opl2& that);
    Opl2& operator=(const Opl2& that);
}; // class Opl2


} // namespace bstone


#endif // BSTONE_OPL2_H
