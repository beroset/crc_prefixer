#include "crc.h"
#include <numeric>
#include <algorithm>
#include <vector>

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
 * \brief given the length and a target CRC value, calculate the prefix
 *
 * \param len the length of the original message
 * \param target is the received CRC value of the message
 * \return the calculated prefix
 */
uint16_t find_prefix(std::size_t len, uint16_t target) {
    std::array<uint16_t, 16> prefixtable{};
    std::array<uint16_t, 16> bitselect{};
    auto v = prefixtable[0] = prefix(1, len);
    auto s = bitselect[0] = 0x0001;
    for (unsigned i{1}; i < 16; ++i) {
        prefixtable[i] = (v << 1) ^ ((v & 0x8000u) ? 0x1021 : 0);
        v = prefixtable[i];
        s = bitselect[i] = (s << 1);
    }
    /* 
     * The goal here is to come up with the combination of 
     * prefixtable entries that xor together to the target value.
     *
     * To minimize the time for these we use a Gray code.
     * Recall that a Gray code is an ordering such that successive
     * terms differ by exactly one bit.
     *
     * We use this so that we only require one XOR operation 
     * of the current testvalue with one of the entries in the
     * prefixtable array.
     *
     * Each iteration, we generate the entry number that should
     * change, which corresponds with a bit position in the prefix
     * and also as an index into the prefixtable array.
     *
     * The recursive definition for generating this sequence for
     * any m-bit Gray code is this:
     *
     *   void gray(int m) {
     *       if (m > 0) {
     *           gray(m-1);
     *           std::cout << (m-1) << '\n';
     *           gray(m-1);
     *       }
     *   }
     *
     * That's compact but not very efficient, so we use a vector
     * as a stack to do the same thing in an iterative rather than
     * recursive manner.
     */
    uint16_t prefix{0};
    uint16_t testvalue{0};
    std::vector<int> work(17);
    std::generate(work.begin(), work.end(), [n=17]() mutable { return --n; });
    while (!work.empty()) {
        int m = work.back();
        work.pop_back();
        if (m == 0) {
            m = work.back() - 1;
            work.pop_back();
            // we have exhausted all possibilities
            if (m > 0x8000u) {
                return 0;
            }
            testvalue ^= prefixtable[m];
            prefix ^= bitselect[m];
            // we've found our target
            if (testvalue == target) {
                return prefix;
            }
            for (auto i{m+1}; i; --i) {
                work.push_back(i-1);
            }
        }
    }
    return 0;   // should never get here
}
}
