# Cipher

A library for encoding messages using a plethora of ciphers

- All functions work with unicode encoding.
- Works in C and C++.
- Bindings for js/ts through wasm, works on the web and node/bun/deno.

## Ciphers

Currently, this library supports the following ciphers:
- Alphabet lookup (e.g. atbash, vignère)
- Ascii
- Block method
- Caesar
- Morse (as text or audio)
- Letter to number
- Reverse words
- Substitution cipher
- "year" cipher

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

**compiling for the web**

The following will build a release build for use on the web.

```sh
beaver --target wasm32-unkown-emscripten -o -- --build-unistring
```

In the [binding/js](binding/js) directory you can find a binding file written
in typescript.

**compiling for node/bun/deno**

When you want to use this library in a node environment rather than on the web,
the `--build-for-node` option can be used.

```sh
beaver --target wasm32-unknown-emscripten -o -- --build-unistring --build-for-node
```

*You might have to enable node compatibility layer when running in deno*

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
