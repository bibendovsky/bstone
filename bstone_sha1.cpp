//
// SHA-1 implementation based on RFC 3174 sample code
// http://www.ietf.org/rfc/rfc3174.txt
//


#include "bstone_sha1.h"
#include <utility>


namespace bstone {


namespace {


template<int TBits>
constexpr uint32_t sha1_circular_shift(
    uint32_t word)
{
    return (word << TBits) | (word >> (32 - TBits));
}


} // namespace


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Sha1Exception definitions

Sha1Exception::Sha1Exception(
    const std::string& message) :
        std::runtime_error(message)
{
}

Sha1Exception::Sha1Exception(
    const char* message) :
        std::runtime_error(message)
{
}

Sha1Exception::~Sha1Exception()
{
}

// Sha1Exception definitions
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Sha1 definitions

Sha1::Sha1() :
        digest_(),
        digest32_(),
        length_low_(),
        length_high_(),
        block_index_(),
        block_(),
        is_computed_(),
        is_corrupted_()
{
    reset();
}

Sha1::Sha1(
    const Sha1& that) :
        digest_(that.digest_),
        digest32_(that.digest32_),
        length_low_(that.length_low_),
        length_high_(that.length_high_),
        block_index_(that.block_index_),
        block_(that.block_),
        is_computed_(that.is_computed_),
        is_corrupted_(that.is_corrupted_)
{
}

Sha1::Sha1(
    Sha1&& that) :
        digest_(std::move(that.digest_)),
        digest32_(std::move(that.digest32_)),
        length_low_(std::move(that.length_low_)),
        length_high_(std::move(that.length_high_)),
        block_index_(std::move(that.block_index_)),
        block_(std::move(that.block_)),
        is_computed_(std::move(that.is_computed_)),
        is_corrupted_(std::move(that.is_corrupted_))
{
}

Sha1& Sha1::operator=(
    Sha1 that)
{
    swap(*this, that);
    return *this;
}

Sha1::~Sha1()
{
}

void Sha1::reset()
{
    length_low_ = 0;
    length_high_ = 0;
    block_index_ = 0;

    digest32_ = {
        0x67452301,
        0xEFCDAB89,
        0x98BADCFE,
        0x10325476,
        0xC3D2E1F0,
    };

    is_computed_ = false;
    is_corrupted_ = false;
}

void Sha1::process(
    const void* message,
    int message_size)
{
    if (message_size < 0) {
        throw Sha1Exception("Negative message size.");
    }

    if (is_computed_) {
        is_corrupted_ = true;

        throw Sha1Exception("Already computed.");
    }

    if (is_corrupted_) {
        throw Sha1Exception("Digest is corrupted.");
    }

    auto octets = static_cast<const uint8_t*>(message);

    for (int i = 0; i < message_size; ++i) {
        block_[block_index_++] = octets[i] & 0xFF;

        length_low_ += 8;

        if (length_low_ == 0) {
            ++length_high_;

            if (length_high_ == 0) {
                // Message is too long.
                is_corrupted_ = true;

                throw Sha1Exception("Message is too long.");
            }
        }

        if (block_index_ == 64) {
            process_block();
        }
    }
}

void Sha1::finish()
{
    if (is_corrupted_) {
        throw Sha1Exception("Digest is corrupted.");
    }

    if (!is_computed_) {
        pad_message();

        block_.fill(0);

        length_low_ = 0;
        length_high_ = 0;

        is_computed_ = true;
    }

    for (int i = 0; i < hash_size; ++i) {
        digest_[i] = static_cast<uint8_t>(
            digest32_[i >> 2] >> 8 * (3 - (i & 0x3)));
    }
}

const Sha1::Digest& Sha1::get_digest() const
{
    if (is_corrupted_) {
        throw Sha1Exception("Digest is corrupted.");
    }

    if (!is_computed_) {
        throw Sha1Exception("Digest not computed.");
    }

    return digest_;
}

std::string Sha1::get_digest_string() const
{
    const auto& digest = get_digest();

    std::string digest_string;
    digest_string.reserve(hash_size * 2);

    for (auto digest_octet : digest) {
        const char nibbles[2] = {
            static_cast<char>((digest_octet / 16) & 0xF),
            static_cast<char>(digest_octet & 0xF),
        };

        for (int i = 0; i < 2; ++i) {
            char nibble_char;
            const auto nibble = nibbles[i];

            if (nibble <= 9) {
                nibble_char = '0' + nibble;
            } else {
                nibble_char = 'a' + (nibble - 10);
            }

            digest_string += nibble_char;
        }
    }

    return digest_string;
}

void Sha1::swap(
    Sha1& a,
    Sha1& b)
{
    std::swap(a.digest_, b.digest_);
    std::swap(a.digest32_, b.digest32_);
    std::swap(a.length_low_, b.length_low_);
    std::swap(a.length_high_, b.length_high_);
    std::swap(a.block_index_, b.block_index_);
    std::swap(a.block_, b.block_);
    std::swap(a.is_computed_, b.is_computed_);
    std::swap(a.is_corrupted_, b.is_corrupted_);
}

void Sha1::pad_message()
{
    // Check to see if the current message block is too small to hold
    // the initial padding bits and length.  If so, we will pad the
    // block, process it, and then continue padding into a second
    // block.
    //

    if (block_index_ > 55) {
        block_[block_index_] = 0x80;
        ++block_index_;

        while (block_index_ < 64) {
            block_[block_index_] = 0;
            ++block_index_;
        }

        process_block();

        while (block_index_ < 56) {
            block_[block_index_] = 0;
            ++block_index_;
        }
    } else {
        block_[block_index_] = 0x80;
        ++block_index_;

        while (block_index_ < 56) {
            block_[block_index_] = 0;
            ++block_index_;
        }
    }

    // Store the message length as the last 8 octets
    //
    block_[56] = static_cast<uint8_t>(length_high_ >> 24);
    block_[57] = static_cast<uint8_t>(length_high_ >> 16);
    block_[58] = static_cast<uint8_t>(length_high_ >> 8);
    block_[59] = static_cast<uint8_t>(length_high_);
    block_[60] = static_cast<uint8_t>(length_low_ >> 24);
    block_[61] = static_cast<uint8_t>(length_low_ >> 16);
    block_[62] = static_cast<uint8_t>(length_low_ >> 8);
    block_[63] = static_cast<uint8_t>(length_low_);

    process_block();
}

void Sha1::process_block()
{
    // Constants defined in SHA-1
    const uint32_t k[] = {
        0x5A827999,
        0x6ED9EBA1,
        0x8F1BBCDC,
        0xCA62C1D6
    };

    // Word buffers
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    uint32_t e;

    // Word sequence
    std::array<uint32_t, 80> w;

    // Initialize the first 16 words in the array W
    //
    for (int i = 0; i < 16; ++i) {
        w[i] = block_[i * 4] << 24;
        w[i] |= block_[i * 4 + 1] << 16;
        w[i] |= block_[i * 4 + 2] << 8;
        w[i] |= block_[i * 4 + 3];
    }

    for (int i = 16; i < 80; ++i) {
        w[i] = sha1_circular_shift<1>(
            w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]);
    }

    a = digest32_[0];
    b = digest32_[1];
    c = digest32_[2];
    d = digest32_[3];
    e = digest32_[4];

    for (int i = 0; i < 20; ++i) {
        uint32_t temp = sha1_circular_shift<5>(a) +
            ((b & c) | ((~b) & d)) + e + w[i] + k[0];

        e = d;
        d = c;
        c = sha1_circular_shift<30>(b);
        b = a;
        a = temp;
    }

    for (int i = 20; i < 40; ++i) {
        uint32_t temp = sha1_circular_shift<5>(a) +
            (b ^ c ^ d) + e + w[i] + k[1];

        e = d;
        d = c;
        c = sha1_circular_shift<30>(b);
        b = a;
        a = temp;
    }

    for (int i = 40; i < 60; ++i) {
        uint32_t temp = sha1_circular_shift<5>(a) +
            ((b & c) | (b & d) | (c & d)) + e + w[i] + k[2];

        e = d;
        d = c;
        c = sha1_circular_shift<30>(b);
        b = a;
        a = temp;
    }

    for (int i = 60; i < 80; ++i) {
        uint32_t temp = sha1_circular_shift<5>(a) +
            (b ^ c ^ d) + e + w[i] + k[3];

        e = d;
        d = c;
        c = sha1_circular_shift<30>(b);
        b = a;
        a = temp;
    }

    digest32_[0] += a;
    digest32_[1] += b;
    digest32_[2] += c;
    digest32_[3] += d;
    digest32_[4] += e;

    block_index_ = 0;
}

// Sha1 definitions
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Sha1 functions

bool operator==(
    const bstone::Sha1& a,
    const std::string& b)
{
    if (b.size() != (bstone::Sha1::hash_size * 2)) {
        throw bstone::Sha1Exception("Invalid size of SHA-1 string.");
    }

    const auto& digest = a.get_digest();

    for (int i = 0; i < bstone::Sha1::hash_size; ++i) {
        int nibbles[2];

        for (int j = 0; j < 2; ++j) {
            auto nibble_char = b[(i * 2) + j];

            switch (nibble_char) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                nibbles[j] = nibble_char - '0';
                break;

            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                nibbles[j] = 10 + (nibble_char - 'a');
                break;

            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                nibbles[j] = 10 + (nibble_char - 'A');
                break;

            default:
                throw bstone::Sha1Exception("Invalid SHA-1 string.");
            }
        }

        auto digest_octet =
            static_cast<uint8_t>((nibbles[0] * 16) + nibbles[1]);

        if (digest[i] != digest_octet) {
            return false;
        }
    }

    return true;
}

bool operator!=(
    const bstone::Sha1& sha1,
    const std::string& sha1_string)
{
    return !(sha1 == sha1_string);
}

// Sha1 functions
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
