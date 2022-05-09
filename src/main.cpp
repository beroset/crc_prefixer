#include "crc.h"
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
    
/*!
 * \brief convert a hex string into a vector of uint8_t
 *
 * \param str the string containing hex digits
 * \return vector of uint8_t with the byte values
 */
std::vector<uint8_t> hexStringToVector(std::string str) {
    std::vector<uint8_t> retval;
    // now convert each hex pair into a uint8_t
    for (auto it{str.begin()}; it != str.end(); it += 2) {
        std::string small{it, it+2};
        retval.push_back(static_cast<uint8_t>(std::stoul(small, nullptr, 16)));
    }
    return retval;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cout << "Usage: crc_prefixer hexmessage\n";
        return 1;
    }
    // convert the string to a message
    auto message{hexStringToVector(argv[1])};
    // remove the crc value from the last two bytes
    uint16_t crc = message.back();
    message.pop_back();
    crc += 256 * message.back();
    message.pop_back();

    auto zeroprefix{beroset::crc16(0, message.begin(), message.end())};
    uint16_t calculated = beroset::find_prefix(message.size(), zeroprefix ^ crc);
    std::cout << "prefix = 0x" << std::hex << std::setfill('0') << std::setw(4) 
              << calculated << '\n';
}
