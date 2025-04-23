# Cipher

A library for encoding messages using a plethora of ciphers

- All functions work with unicode encoding.
- Works in C and C++.

## Ciphers

Currently, this library supports the following ciphers:
- Alphabet lookup (e.g. atbash, vignère)
- Ascii
- Caesar
- Reverse words
- Morse

## Examples

Examples can be found in the [tests](./tests) directory.

## Documentation

All functions are fully documented in the header files.

- See [cipher.h](./include/cipher/cipher.h) for general concepts
- See [ciphers.h](./include/cipher/ciphers.h) for a list of all functions paired
  with their documentation

## Building

To build the library, you need the [beaver build system](https://github.com/Jomy10/beaver),
which can be installed with `cargo install --git https://github.com/Jomy10/beaver`.

To build the library:

```sh
# build only the library
beaver cipher -o
# or
# Build library and testing executable
beaver -o
```

**Build options**
- `build-unistring`: build a fresh copy if libunistring instead of using the one
  installed on the system.
- `no-audio`: don't link with the audio library and provide no functions to
  encode messages to audio.

**using build options**

Build options should be provide after a double dash "`--`". e.g.

```sh
beaver cipher -o -- --no-audio
```

## testing

```sh
beaver test
```

Same build options apply to the test command

## Contributing

Contributions are highly appreciated. This library isn't perfect, but your
contribution can make it better.

## License

The code is licensed under the [MIT license](LICENSE).
