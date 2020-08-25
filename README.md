# X-Mode: Extending Block Ciphers

This is a simple way to extend AES (or almost any 256-bit+ block cipher) to use 512+ bit keyspace as a means to add additional resistance to cryptanalysis. We're using OpenSSL in CTR mode and adding a simple XOR-encoding program wrapped up in a shell script (a cherry on top is optional). Adding in blocks of cryptographically-secure random data using simple XOR techniques is expected to add a great deal of extra effort and confusion to cryptanalysis efforts.

**This idea is really just proof-of-concept and lots of testing and analysis has yet to be performed. You with caution.**

This idea is really just proof-of-concept and lots of testing and analysis has yet to be performed. You with caution.

## Prerequisites

You'll need GCC. There are numerous tutorials online to assist you with this step, if you're new to the GCC toolchain.

* In the 'xor' directory, use the included 'make' file to create this utility. Copy the cpmpiled binary to your chosen working directory.
* In the 'testgen' directory, use the included 'make' file to create this utility. Copy the cpmpiled binary to your chosen working directory.

You'll need 'dieharder' to use the optional included statistical analysis script. This is mostly used to test the quality of encrypted data, which should test as statistically random.

You'll need PHP (with CLI support) to use the optional included 'keymaker' utility. This is a handy tool for creating as muich mathematical "distance" between a user-supplied password and a generated key set.

## Author

**Rick Pelletier** - [Gannett Co., Inc. (USA Today Network)](https://www.usatoday.com/)

## License

Copyright (C) 2016, Richard Pelletier

```
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

