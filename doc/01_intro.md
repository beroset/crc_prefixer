# Introduction {#mainpage} #

A Cyclic Redundancy Code (CRC) is a kind of mathematical checksum that is commonly used in digital communications protocols to detect, and sometimes correct, errors that may have occurred over a noisy communications channel.  In analyzing an unknown protocol, we sometimes have partial information, such as being able to determine which bytes of the message are the CRC and also which polynomial is being used.  This tool is intended to solve the question of which prefix is being used, given a sample message and known polynomial.  

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

# Theory of operation
To understand how this code works, it's useful to first understand how a CRC is calculated.  First, we can treat a series of bits as though they were coefficients of a polynomal expression.  As an example, consider the hex sequence 0x11021.  Each hexadecimal digit corresponds to four bits, so we can easily translate this to binary, and then interpret each bit as a coefficient of a polynomial:

|   1  |   1  |   0  |   2  |   1  |
|:----:|:----:|:----:|:----:|:----:|
| 0001 | 0001 | 0000 | 0010 | 0001 |
| 0x<sup>19</sup>0x<sup>18</sup>0x<sup>17</sup>1x<sup>16</sup> | 0x<sup>15</sup>0x<sup>14</sup>0x<sup>13</sup>1x<sup>12</sup> | 0x<sup>11</sup>0x<sup>10</sup>0x<sup>9</sup>0x<sup>8</sup> | 0x<sup>7</sup>0x<sup>6</sup>1x<sup>5</sup>0x<sup>4</sup> | 0x<sup>3</sup>0x<sup>2</sup>0x<sup>1</sup>1x<sup>0</sup> |

If we omit all of the terms with a zero coefficient, we can rewrite this as x<sup>16</sup> + x<sup>12</sup> + x<sup>5</sup> + 1 which you may recognize as the polynomial shown above.\footnote{recall that anything to the 0 power = 1}

Since we start with *bits* which each have only a 0 or 1 as possible values, mathematically speaking, our CRC calculation is a polynomial division over the Galois field of order 2, or GF(2).  (See https://en.wikipedia.org/wiki/Mathematics_of_cyclic_redundancy_checks )


For this software, we divide the message **M** by the polynomial **P** and get quotient **Q** (which is discarded) and the remainder **R** which is the value appended to the message and commonly called "the CRC of the message."  If we put a prefix **X** on the message, we have **X | M** divided by **P** = **Q** remainder **R**, where **X | M** here means the bits of **X** followed by the bits of **M** or in other words, concatenation.

We can rewrite this concatenation: **X | M** = **X** * 2<sup>bits(M)</sup> + **M** where *bits(**M**)* is the number of bits in **M**. Since division is *distributive*, we can assert that:

( **X | M** ) / **P** = (**X** * 2<sup>bits(M)</sup> + **M**) / **P** = **X** * 2<sup>bits(M)</sup> / **P** + **M** / **P**

This implies that we can calculate **M** / **P** separately to get our remainder **R1** and then calculate **X** * 2<sup>bits(M)</sup> / **P** to get remainder **R2** and simply add the remainders together to get **R**.

In our case, we know **M**, bits(M), **P** and **R** and want to get **X**.

Since division over GF(2) is distributive, we can factor still further by considering each individual bit of X separately, generating 16 remainders.  Because adding polynomials over GF(2) is identical to the XOR operator, all we need to do is XOR our unprefixed calculated remainder **R1** with the transmitted remainder **R** and try, at worst, 65,536 combinations of those 16 remainders to come up with the prefix.

## Further optimizations ##
There are some further optimizations that could be done.  The first well-known optimization is to use a lookup table rather than bit-shifting and masking to calculate the CRC remainder.  This optimization is already used within the code.  The next optimization is to calculate the the table of remainders in a slightly more efficient manner.  Instead of calculating each from scratch for prefix values of 0x0001, 0x0002, 0x0004, etc. we can just calculate the first and then derive the others from it.
