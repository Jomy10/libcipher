// import Module from "../../build/release/cipher.js";
import Module from "../../build/wasm32-unknown-emscripten/release/libcipher/artifacts/cipher.js";

function _strToUTF8WithLength(string: string): [number | null, number] {
  if (string.length == 0) return [null, 0];

  let bytes = cipher._encoder.encode(string);
  let byte_len = bytes.length * bytes.BYTES_PER_ELEMENT;
  let ptr = cipher._Module._malloc(byte_len);
  cipher._Module.HEAPU8.set(bytes, ptr);
  return [ptr, byte_len];
}

function _ptrToStr(ptr: number, len: number): string {
  let outbuffer = cipher._Module.HEAPU8.subarray(ptr, ptr + len);
  return cipher._decoder.decode(outbuffer);
}

const cipher = {
  _encoder: new TextEncoder(),
  _decoder: new TextDecoder(),
  _Module: await Module(),

  Error: class extends Error {
    code: number;

    constructor(code: number) {
      let message: string;
      switch (code) {
        case cipher.Err.OK: message = "OK"; break;
        case cipher.Err.ERR_ENCODING: message = "input is not valid UTF-8"; break;
      }
      super(message!);
      this.name = "CipherError";
      this.code = code;
    }
  },

  Err: {
    OK: 0,
    GROW: 1,
    ERR_ENCODING: 2,
    ERR_YEAR_DIGITS: 3
  },

  ascii: function (input: string, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }
    let [inputptr, inputbyte_len] = _strToUTF8WithLength(input);

    let outlen = inputbyte_len * 4 - 1;
    let outptr = cipher._Module._malloc(outlen);
    let outstring: string;

    try {
      let ret: number = cipher._Module._ciph_ascii(inputptr, inputbyte_len, outptr);
      if (ret != cipher.Err.OK) throw new cipher.Error(ret); // shouldn't occur

      outstring = _ptrToStr(outptr, outlen);
      output(outstring);
      // let outbuffer = HEAPU8.subarray(outptr, outptr + (outlen - 1));
      // outstring = cipher._decoder.decode(outbuffer);
    } finally {
      cipher._Module._free(inputptr);
      cipher._Module._free(outptr);
    }
  },

  reverse_words: function (input: string, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }
    let [inputptr, inputbyte_len] = _strToUTF8WithLength(input);

    let outlen = inputbyte_len;
    let outptr = cipher._Module._malloc(outlen);
    let outstring: string;

    try {
      let ret = cipher._Module._ciph_reverse_words(inputptr, inputbyte_len, outptr);
      if (ret != cipher.Err.OK) throw new cipher.Error(ret);

      outstring = _ptrToStr(outptr, outlen);
      output(outstring);
    } finally {
      cipher._Module._free(inputptr);
      cipher._Module._free(outptr);
    }
  },

  caesar: function (input: string, shift: number, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }
    let [inputptr, inputbyte_len] = _strToUTF8WithLength(input);

    let outlen = inputbyte_len;
    let outptr = cipher._Module._malloc(outlen);
    let outstring: string;

    try {
      let ret = cipher._Module._ciph_caesar(inputptr, inputbyte_len, shift, outptr);
      if (ret != cipher.Err.OK) throw new cipher.Error(ret);

      outstring = _ptrToStr(outptr, outlen);
      output(outstring);
    } finally {
      cipher._Module._free(inputptr);
      cipher._Module._free(outptr);
    }
  },

  // Alphabet Lookup //

  alphabet_lookup: function (input: string, lookup: string, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }
    let [inputptr, inputbyte_len] = _strToUTF8WithLength(input);

    let [alphabetptr, alphabetlen] = _strToUTF8WithLength(lookup);
    if (alphabetlen != 26) throw new Error(`Invalid alphabet length ${alphabetlen}`);

    let outlen = inputbyte_len;
    let outptr = cipher._Module._malloc(outlen);
    let outstring: string;

    try {
      let ret = cipher._Module._ciph_alphabet_lookup(inputptr, inputbyte_len, alphabetptr, outptr);
      if (ret != cipher.Err.OK) throw new cipher.Error(ret);

      outstring = _ptrToStr(outptr, outlen);
      output(outstring);
    } finally {
      cipher._Module._free(inputptr);
      cipher._Module._free(outptr);
    }
  },

  alphabet: {
    atbash: function (): string {
      let buffer = cipher._Module._malloc(26);
      try {
        cipher._Module._ciph_alphabet_atbash(buffer);
        return _ptrToStr(buffer, 26).repeat(1); // repeat(1) simply copies the string
      } finally {
        cipher._Module._free(buffer);
      }
    },
    /**
    * Create an alphabet from a keyword for vignere encoding
    * @param word: the word to use as a code for the vignere cipher
    * @returns The alphabet to use for the alphabet lookup and an optional
    *          alphabet which can be used to visualize the cipher.
    */
    vignere: function (word: string, output: (result: string, alphabet_visualize: string) => void) {
      let [wordptr, wordlen] = _strToUTF8WithLength(word);
      let outptr = cipher._Module._malloc(26);
      let alphabetptr = cipher._Module._malloc(26);

      try {
        let validation_result = cipher._Module._ciph_alphabet_vignere_validate(wordptr, wordlen);
        if (validation_result != cipher.alphabet.Validation.OK) {
          throw new cipher.alphabet.ValidationError(validation_result);
        }
        cipher._Module._ciph_alphabet_vignere(wordptr, wordlen, outptr, alphabetptr);
        output(_ptrToStr(outptr, 26), _ptrToStr(alphabetptr, 26));
      } finally {
        cipher._Module._free(wordptr);
        cipher._Module._free(outptr);
        cipher._Module._free(alphabetptr);
      }
    },
    Validation: {
      OK: 0,
      DOUBLE_CHAR: 1,
      TOO_LONG: 2,
    },
    ValidationError: class extends Error {
      code: number;

      constructor(code: number) {
        let message: string;
        switch (code) {
          case cipher.alphabet.Validation.OK: message = "OK"; break;
          case cipher.alphabet.Validation.DOUBLE_CHAR: message = "Character used twice in output"; break;
          case cipher.alphabet.Validation.TOO_LONG: message = "Vignere word is too long, must be at most 26 characters"; break;
        }
        super(message!);
        this.name = "CipherError";
        this.code = code;
      }
    }
  },

  // End Alphabet Lookup //

  morse: function(input: string, copy_non_encodable_characters: boolean, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }
    let [inputptr, inputlen] = _strToUTF8WithLength(input);

    let intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;
    let outputptrptr = cipher._Module._malloc(intsize);
    let outputlenptr = cipher._Module._malloc(intsize);

    try {
      let ret = cipher._Module._ciph_alloc_morse(
        inputptr, inputlen,
        copy_non_encodable_characters,
        outputptrptr, outputlenptr
      );

      if (ret != cipher.Err.OK) throw new Error(ret);

      let outputptr = cipher._Module.HEAP32[outputptrptr / intsize];
      let outputlen = cipher._Module.HEAP32[outputlenptr / intsize];
      output(_ptrToStr(outputptr, outputlen));
    } finally {
      cipher._Module._free(inputptr);
      let outputptr = cipher._Module.HEAP32[outputptrptr / intsize];
      cipher._Module._free(outputptr);
      cipher._Module._free(outputptrptr);
      cipher._Module._free(outputlenptr);
    }
  },

  numbers: function(input: string, copy_non_encodable_characters: boolean, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }

    let [inputptr, inputlen] = _strToUTF8WithLength(input);

    let intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;
    let outputptrptr = cipher._Module._malloc(intsize);
    let outputlenptr = cipher._Module._malloc(intsize);

    try {
      let ret = cipher._Module._ciph_alloc_numbers(
        inputptr, inputlen,
        copy_non_encodable_characters,
        outputptrptr, outputlenptr
      );

      if (ret != cipher.Err.OK) throw new cipher.Error(ret);

      let outputptr = cipher._Module.HEAP32[outputptrptr / intsize];
      let outputlen = cipher._Module.HEAP32[outputlenptr / intsize];
      output(_ptrToStr(outputptr, outputlen));
    } finally {
      cipher._Module._free(inputptr);
      let outputptr = cipher._Module.HEAP32[outputptrptr / intsize];
      cipher._Module._free(outputptr);
      cipher._Module._free(outputptrptr);
      cipher._Module._free(outputlenptr);
    }
  },

  block_method: function (input: string, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }
    let [inputptr, inputlen] = _strToUTF8WithLength(input);

    let intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;
    let outputptrptr = cipher._Module._malloc(intsize);
    let outputlenptr = cipher._Module._malloc(intsize);

    try {
      let ret = cipher._Module._ciph_alloc_block_method(
        inputptr, inputlen,
        outputptrptr, outputlenptr
      );
      if (ret != cipher.Err.OK) throw new cipher.Error(ret);

      let outputptr = cipher._Module.HEAP32[outputptrptr / intsize];
      let outputlen = cipher._Module.HEAP32[outputlenptr / intsize];
      output(_ptrToStr(outputptr, outputlen));
    } finally {
      cipher._Module._free(inputptr);
      let outputptr = cipher._Module.HEAP32[outputptrptr / intsize];
      cipher._Module._free(outputptr);
      cipher._Module._free(outputptrptr);
      cipher._Module._free(outputlenptr);
    }
  },

  alphabet_substitution: function (
    input: string,
    substitution_alphabet: string[],
    char_sep: string,
    word_sep: string,
    sentence_sep: string,
    copy_non_encodable_characters: boolean,
    output: (result: string) => void
  ) {
    if (input.length == 0) {
      output("");
      return;
    }

    let intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;

    let [inputptr, inputlen] = _strToUTF8WithLength(input);

    let outputptrptr = cipher._Module._malloc(intsize);
    let outputlenptr = cipher._Module._malloc(intsize);

    let subptr = cipher._Module._malloc(26 * intsize * 2);
    let free_list: number[] = [];
    for (let i: number = 0; i < 26; i++) {
      let [ptr, len] = _strToUTF8WithLength(substitution_alphabet[i]);
      cipher._Module.HEAP32[(subptr / intsize) + i * 2] = ptr!;
      cipher._Module.HEAP32[(subptr / intsize) + i * 2 + 1] = len;
      free_list.push(ptr!);
    }

    let [char_sep_ptr, char_sep_len] = _strToUTF8WithLength(char_sep);
    let [word_sep_ptr, word_sep_len] = _strToUTF8WithLength(word_sep);
    let [sentence_sep_ptr, sentence_sep_len] = _strToUTF8WithLength(sentence_sep);

    try {
      let err = cipher._Module._ciph_alloc_char_alph_sub(
        inputptr, inputlen,
        subptr,
        char_sep_ptr, char_sep_len,
        word_sep_ptr, word_sep_len,
        sentence_sep_ptr, sentence_sep_len,
        copy_non_encodable_characters,
        outputptrptr, outputlenptr
      );
      if (err != cipher.Err.OK) throw new cipher.Error(err);

      let outputptr = cipher._Module.HEAP32[outputptrptr / intsize];
      let outputlen = cipher._Module.HEAP32[outputlenptr / intsize];
      output(_ptrToStr(outputptr, outputlen));
    } finally {
      cipher._Module._free(inputptr);
      let outputptr = cipher._Module.HEAP32[outputptrptr / intsize];
      cipher._Module._free(outputptr);
      cipher._Module._free(outputptrptr);
      cipher._Module._free(outputlenptr);
      cipher._Module._free(char_sep_ptr);
      cipher._Module._free(word_sep_ptr);
      cipher._Module._free(sentence_sep_ptr);
      for (let ptr of free_list) {
        cipher._Module._free(ptr);
      }
    }
  },

  year: function(
    input: string,
    year: string,
    include_bitmask: number,
    output: (result: string) => void
  ) {
    if (year.length != 4)
      return cipher.Err.ERR_YEAR_DIGITS;
    if (input.length == 0) {
      output("");
      return;
    }

    let [inputptr, inputlen] = _strToUTF8WithLength(input);
    let intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;
    let outputptrptr = cipher._Module._malloc(intsize);
    let outputlen = cipher._Module._malloc(intsize);
    let yearptr = cipher._Module._malloc(4);

    for (let i = 0; i < 4; i += 1) {
      cipher._Module.HEAPU8[yearptr + i] = parseInt(year[i], 10);
    }

    try {
      cipher._Module._ciph_alloc_year(
        inputptr, inputlen,
        yearptr,
        include_bitmask,
        outputptrptr, outputlen
      );

      let outputptr = cipher._Module.HEAP32[outputptrptr / intsize];
      output(_ptrToStr(outputptr, cipher._Module.HEAP32[outputlen / intsize]));
    } finally {
      cipher._Module._free(inputptr);
      let outputptr = cipher._Module.HEAP32[outputptrptr / intsize];
      cipher._Module._free(outputptr);
      cipher._Module._free(outputptrptr);
      cipher._Module._free(outputlen);
      cipher._Module._free(yearptr);
    }
  },
  include_bitmasks: {
    letters: function () { return cipher._Module._ciph_year_include_mask_letters(); },
    letters_and_numbers: function () { return cipher._Module._ciph_year_include_mask_letters_and_numbers(); },
    with_symbols: function () { return cipher._Module._ciph_year_include_mask_with_symbols(); },
    with_symbols_and_dashes: function () { return cipher._Module._ciph_year_include_mask_with_symbols_and_dashes(); },
  },

  /** @module copy
   * Copy versions of the cipher functions. Here the output is copied before being
   * returned which eliminates the need for scoping, but adds an extra allocation.
   * For large inputs, the non-copy versions are recommended.
   */
  copy: {
    ascii: function(input: string): string {
      let res: string;
      cipher.ascii(input, (e: string) => res = e.repeat(1));
      // @ts-ignore
      return res;
    },
    reverse_words: function(input: string): string {
      let res: string;
      cipher.reverse_words(input, (e: string) => res = e.repeat(1));
      // @ts-ignore
      return res;
    },
    caesar: function(input: string, shift: number): string {
      let res: string;
      cipher.caesar(input, shift, (e: string) => res = e.repeat(1));
      // @ts-ignore
      return res;
    },
    alphabet_lookup: function(input: string, lookup: string): string {
      let res: string;
      cipher.alphabet_lookup(input, lookup, (e: string) => res = e.repeat(1));
      // @ts-ignore
      return res;
    },
    alphabet: {
      atbash: (): string => cipher.alphabet.atbash(),
      vignere: function(word: string): [string, string] {
        let res: [string, string];
        cipher.alphabet.vignere(word, (alph: string, visualize: string) => res = [alph, visualize]);
        // @ts-ignore
        return res;
      }
    },
    morse: function(input: string, copy_non_encodable_characters: boolean): string {
      let res: string;
      cipher.morse(input, copy_non_encodable_characters, (e: string) => res = e.repeat(1));
      // @ts-ignore
      return res;
    },
    numbers: function(input: string, copy_non_encodable_characters: boolean): string {
      let res: string;
      cipher.numbers(input, copy_non_encodable_characters, (e: string) => res = e.repeat(1));
      // @ts-ignore
      return res;
    },
    block_method: function(input: string): string {
      let res: string;
      cipher.block_method(input, (e: string) => res = e.repeat(1));
      // @ts-ignore
      return res;
    },
    alphabet_substitution: function (
      input: string,
      substitution_alphabet: string[],
      char_sep: string,
      word_sep: string,
      sentence_sep: string,
      copy_non_encodable_characters: boolean,
    ) {
      let res: string;
      cipher.alphabet_substitution(
        input, substitution_alphabet,
        char_sep, word_sep, sentence_sep,
        copy_non_encodable_characters,
        (output: string) => res = output.repeat(1)
      );
      // @ts-ignore
      return res;
    },
    year: function(
      input: string,
      year: string,
      include_bitmask: number,
    ) {
      let res: string;
      cipher.year(input, year, include_bitmask, (output: string) => res = output.repeat(1));
      // @ts-ignore
      return res;
    }
  }
};

export default cipher;
