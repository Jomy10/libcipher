// libcipher
//
// (c) Jonas Everaert
//
// Library to encode messages in a plethora of ciphers.
//
// MIT License
//
// Copyright (c) 2025 Jonas Everaert
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

//
// # General concepts
//
// A `char*` string denotes a string in any encoding, usually the platform's
// own encoding, unless otherwise annotated.
//
// A `uint8_t*` denotes a valid unicode string. Function expecting this kind of
// string do not check for valid unicode, it is the caller's responsibility to
// ensure valid unicode. This can be checked using GNU's libunistring's `u8_check`
// function.
//
// All functions return a `ciph_err_t`. `CIPH_OK` (= 0) is returned on success,
// otherwise a `CIPH_ERR_{errtype}` is returned.
//
// Text output is always provided at the end of the function using a non-const pointer.
//
// All parameters are annoteted with `nonnil` and `nilable` attributes. Passing a
// NULL pointer to a `nonnil` parameter is considered undefined behaviour, it is
// the caller's responisibility to ensure this doesn't happen.
//
// Allocations usually happen by the user of the library. Read the documentation of
// each function to determine how much to allocate for an output buffer for example.
//

#include "error.h"
#include "ciphers.h"
