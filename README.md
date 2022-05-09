# crc_prefixer
Calculate a message CRC prefix from the message

# Example
```
> crc_prefixer 30FFFFFFFFFFFF50CF5DD9E2C0B80065F5D1A483F0FBBC6F01001E6C043B517E90B286
prefix = 0x5fd6
```

# What it does
The code interprets the string as a series of hex bytes of a message, with the last two being a calculated CRC of the message with an unknown prefix, and calculates the prefix.

It uses the standard CCITT polynomial of x<sup>16</sup> + x<sup>12</sup> + x<sup>5</sup> + 1 and consumes each byte of the message from most significant to least.  The CRC value at the end is interpreted as big-endian byte order, so in the example above the CRC remainder value is 0xb286.

# How to compile
This project uses CMake, so the usual build procedure can be used.  From the top level (`crc_prefixer`) directory:

```
cmake -B build
cmake --build build
sudo cmake --build build -t install
```

This will install the `crc_prefixer` code, typically in your `/usr/local/bin` directory, but this may be altered if desired.
