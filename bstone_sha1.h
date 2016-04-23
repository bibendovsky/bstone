//
// SHA-1 implementation based on RFC 3174 sample code
// http://www.ietf.org/rfc/rfc3174.txt
//


#ifndef BSTONE_SHA1_INCLUDED
#define BSTONE_SHA1_INCLUDED


#include <cstdint>
#include <array>
#include <stdexcept>
#include <string>


namespace bstone {


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Sha1Exception declaration

class Sha1Exception :
    std::runtime_error
{
public:
    explicit Sha1Exception(
        const std::string& message);

    explicit Sha1Exception(
        const char* message);

    virtual ~Sha1Exception();
}; // Sha1Exception

// Sha1Exception declaration
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Sha1 declaration

class Sha1 {
public:
    static const int hash_size = 20;

    using Digest = std::array<uint8_t, hash_size>;


    Sha1();

    Sha1(
        const Sha1& that);

    Sha1(
        Sha1&& that);

    Sha1& operator=(
        Sha1 that);

    ~Sha1();


    void reset();

    void process(
        const void* data,
        int data_size);

    void finish();

    const Digest& get_digest() const;

    std::string get_digest_string() const;

    static void swap(
        Sha1& a,
        Sha1& b);


private:
    using Block = std::array<uint8_t, 64>;
    using Digest32 = std::array<uint32_t, hash_size / 4>;


    // Message digest.
    Digest digest_;

    // Message digest as words.
    Digest32 digest32_;

    // Message length in bits.
    uint32_t length_low_;

    // Message length in bits.
    uint32_t length_high_;

    // Index into message block array.
    int_least16_t block_index_;

    // 512-bit message block.
    Block block_;

    // Is the digest computed?
    bool is_computed_;

    // Is the digest corrupted?
    bool is_corrupted_;


    void pad_message();

    void process_block();
}; // Sha1

// Sha1 declaration
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Sha1 functions

bool operator==(
    const bstone::Sha1& sha1,
    const std::string& sha1_string);

bool operator!=(
    const bstone::Sha1& sha1,
    const std::string& sha1_string);

// Sha1 functions
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


#endif // BSTONE_SHA1_INCLUDED
