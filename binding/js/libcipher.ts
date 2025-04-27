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
    ERR_ENCODING: 1,
  },

  ascii: function (input: string, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }
    let [inputptr, inputbyte_len] = _strToUTF8WithLength(input);

    let outlen = inputbyte_len * 4;
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

  morse: function (input: string, copy_non_encodable_characters: boolean, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }
    let [inputptr, inputlen] = _strToUTF8WithLength(input);
    let og_inputptr = inputptr;

    let outputlen = 0;
    let outputcap = inputlen * 4;
    let outputptr = cipher._Module._malloc(outputcap);

    let intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;
    let input_left_ptr = cipher._Module._malloc(intsize);
    cipher._Module.HEAP32[input_left_ptr / intsize] = inputptr;
    let input_len_left = inputlen;
    let input_len_left_ptr = cipher._Module._malloc(intsize);
    cipher._Module.HEAP32[input_len_left_ptr / intsize] = inputlen;
    let add_output_len_ptr = cipher._Module._malloc(intsize);
    cipher._Module.HEAP32[add_output_len_ptr / intsize] = 0;

    try {
      while (true) {
        let ret = cipher._Module._ciph_morse(
          inputptr, input_len_left,
          outputptr + outputlen, outputcap - outputlen,
          copy_non_encodable_characters,
          input_left_ptr, input_len_left_ptr,
          add_output_len_ptr
        );
        if (ret != cipher.Err.OK) throw new cipher.Error(ret);

        outputlen += cipher._Module.HEAP32[add_output_len_ptr / intsize];

        inputptr = cipher._Module.HEAP32[input_left_ptr / intsize];

        input_len_left = cipher._Module.HEAP32[input_len_left_ptr / intsize];
        if (input_len_left == 0) break;

        outputcap *= 2;
        outputptr = cipher._Module._realloc(outputptr, outputcap);
      }
      output(_ptrToStr(outputptr, outputlen));
    } finally {
      cipher._Module._free(og_inputptr);
      cipher._Module._free(outputptr);
      cipher._Module._free(input_left_ptr);
      cipher._Module._free(input_len_left_ptr);
      cipher._Module._free(add_output_len_ptr);
    }
  },

  // TODO: outputlen not correct
  numbers: function(input: string, copy_non_encodable_characters: boolean, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }
    let [inputptr, inputlen] = _strToUTF8WithLength(input);
    let og_inputptr = inputptr;

    let outputlen = 0;
    let outputcap = inputlen * 4;
    let outputptr = cipher._Module._malloc(outputcap);

    let intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;
    let input_left_ptr = cipher._Module._malloc(intsize);
    cipher._Module.HEAP32[input_left_ptr / intsize] = inputptr;
    let input_len_left = inputlen;
    let input_len_left_ptr = cipher._Module._malloc(intsize);
    cipher._Module.HEAP32[input_len_left_ptr / intsize] = inputlen;
    let add_output_len_ptr = cipher._Module._malloc(intsize);
    cipher._Module.HEAP32[add_output_len_ptr / intsize] = 0;

    try {
      while (true) {
        let ret = cipher._Module._ciph_numbers(
          inputptr, input_len_left,
          outputptr + outputlen, outputcap - outputlen,
          copy_non_encodable_characters,
          input_left_ptr, input_len_left_ptr,
          add_output_len_ptr
        );
        if (ret != cipher.Err.OK) throw new cipher.Error(ret);

        outputlen += cipher._Module.HEAP32[add_output_len_ptr / intsize];

        inputptr = cipher._Module.HEAP32[input_left_ptr / intsize];

        input_len_left = cipher._Module.HEAP32[input_len_left_ptr / intsize];
        if (input_len_left == 0) break;

        outputcap *= 2;
        outputptr = cipher._Module._realloc(outputptr, outputcap);
      }
      output(_ptrToStr(outputptr, outputlen));
    } finally {
      cipher._Module._free(og_inputptr);
      cipher._Module._free(outputptr);
      cipher._Module._free(input_left_ptr);
      cipher._Module._free(input_len_left_ptr);
      cipher._Module._free(add_output_len_ptr);
    }
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
    }
  }
};

export default cipher;
