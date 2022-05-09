#ifndef BEROSET_CRC_H
#define BEROSET_CRC_H
#include <cstdint>
#include <array>
#include <vector>

namespace beroset {
/*!
 * \brief calculate a CRC-16 (polynomial 0x1021) over the given vector
 *
 * \param crc starting value for the crc (a prefix value)
 * \param first an iterator pointing to the first byte of the message
 * \param last an iterator pointing one beyond the last byte of the message
 * \return the CRC of the given message
 */
uint16_t crc16(uint16_t crc, std::vector<uint8_t>::const_iterator first, std::vector<uint8_t>::const_iterator last);

/*!
 * \brief given the length and a target CRC value, calculate the prefix
 *
 * \param len the length of the original message
 * \param target is the received CRC value of the message
 * \return the calculated prefix
 */
uint16_t find_prefix(std::size_t len, uint16_t target);

} // end of namespace beroset
#endif // BEROSET_CRC_H
