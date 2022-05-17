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

std::ostream& operator<<(std::ostream& out, const std::vector<uint8_t>& vec) {
    for (const auto& num : vec) {
        out << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned>(num);
    }
    return out;
}

int main()
{
    std::string line;
    unsigned linenum{0};
    std::vector<uint8_t> message;
    while (std::getline(std::cin, line)) {
        ++linenum;
        // convert the string to a message
        try {
            message = hexStringToVector(line);
        } catch (std::invalid_argument err) {
            std::cerr << "ignoring bad line " << std::dec << linenum << '\n';
            std::cerr << message << '\n';
            continue;
        }

        auto zeroprefix{beroset::crc16(0, message.begin(), message.end())};
        uint16_t calculated = beroset::find_prefix(message.size(), zeroprefix);
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(4) 
                  << calculated << '\n';
    }
}
