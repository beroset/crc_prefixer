#include "crc.h"
#include <numeric>

#include <iostream>
#include <iomanip>

namespace beroset {

template<typename T, T Poly>
struct CRCTable {
    static constexpr T mask{static_cast<T>(1u << (8 * sizeof(T) - 1))};
    static constexpr unsigned shift{8 * sizeof(T) - 8};
    static constexpr T Generate(T v, int r = 8) {
        return r == 0 ? v : Generate((v << 1) ^ ((v & mask) ? Poly : 0), r - 1);
    }
    T tbl[256];
    template<T ...Is>
    constexpr CRCTable(std::integer_sequence<T, Is...>) : tbl{Generate(Is << shift)...} {}
    constexpr CRCTable() : CRCTable(std::make_integer_sequence<T, 256>()) {}
    constexpr T operator[](int i) const { return tbl[i]; }
};

static constexpr CRCTable<uint16_t, 0x1021> crc16tbl;

/*!
 * \brief helper lambda to assist with CRC calculation
 *
 * This applies the standard CCITT polynomial and consumes the data bits
 * from most significant to least.
 *
 * \param crc is the current value of the crc
 * \param databyte is the data byte to be applied to the crc
 * \return the calculated CRC with the databyte applied
 *
 */
static auto crc_iter = [](uint16_t crc, uint8_t databyte){
    return crc16tbl[crc >> 8 ^ databyte] ^ (crc << 8);
};

/*!
 * \brief calculate a CRC-16 as described above over the given vector
 *
 * \param crc starting value for the crc (a prefix value)
 * \param first an iterator pointing to the first byte of the message
 * \param last an iterator pointing one beyond the last byte of the message
 * \return the CRC of the given message
 */
uint16_t crc16(uint16_t crc, std::vector<uint8_t>::const_iterator first, std::vector<uint8_t>::const_iterator last) {
    return std::accumulate(first, last, crc, crc_iter);
}

/*!
 * \brief calculate a CRC-16 of the given CRC prefix and an empty message
 *
 * This calculates the CRC over an all-zero message of the given length
 *
 * \param crc starting value for the crc (a prefix value)
 * \param msglen the length of the all-zeroes message
 * \return the calculated CRC
 */
constexpr uint16_t prefix(uint16_t crc, std::size_t msglen) {
    for ( ; msglen; --msglen) {
        crc = crc_iter(crc, 0);
    }
    return crc;
}

/*!
 * \brief XOR all values in prefixtable for which there is a 1 in bits
 *
 * For each set bit in bits, this does an XOR of the corresponding value
 * in prefixtable.
 *
 * \param bits each set bit in bits corresponds to a value in prefixtable
 * \param prefixtable a precalculated table of possible 1-bit CRC prefixes
 * \return the calculated XOR value
 */
constexpr uint16_t test_prefix(uint16_t bits, const std::array<uint16_t, 16> prefixtable) {
    uint16_t retval{0}; 
    for (int i=0 ; bits; bits >>= 1, ++i) {
        if (bits & 1) {
            retval ^= prefixtable[i];
        }
    }
    return retval;
}

/*!
 * \brief given the length and a target CRC value, calculate the prefix
 *
 * \param len the length of the original message
 * \param target is the received CRC value of the message
 * \return the calculated prefix
 */
uint16_t find_prefix(std::size_t len, uint16_t target) {
    std::array<uint16_t, 16> prefixtable{};
    auto v = prefixtable[0] = prefix(1, len);
    for (unsigned i{1}; i < 16; ++i) {
        prefixtable[i] = (v << 1) ^ ((v & 0x8000u) ? 0x1021 : 0);
        v = prefixtable[i];
    }
    for (uint16_t prefix{0xffff}; prefix; --prefix) {
        if (target == test_prefix(prefix, prefixtable)) {
            return prefix;
        }
    }
    return 0;
}
}
