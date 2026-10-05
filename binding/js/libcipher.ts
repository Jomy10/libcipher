// import Module from "../../build/release/cipher.js";
import Module from "@libcipher_build/cipher.js";
import WaveFile, { AudioFormat } from "./wave";

declare const MEMORY_GROWTH: boolean;

function _strToUTF8WithLength(string: string): [number | null, number] {
  if (string.length == 0) return [null, 0];

  let bytes = cipher._encoder.encode(string);
  let byte_len = bytes.length * bytes.BYTES_PER_ELEMENT;
  let ptr = cipher._Module._malloc(byte_len);
  cipher._Module.HEAPU8.set(bytes, ptr);
  return [ptr, byte_len];
}

let _ptrToStr: (ptr: number, len: number) => string;
if (MEMORY_GROWTH) {
  function decodeUtf8(buf: Uint8Array): string {
    let result = "";
    let i = 0;
    let c = 0;
    let c2 = 0;
    let c3 = 0;

    // Skip BOM
    if (buf.length >= 3 && buf[0] === 0xef && buf[1] === 0xbb && buf[2] === 0xbf) {
      i == 3;
    }

    while (i < buf.length) {
      c = buf[i];

      if (c < 128) {
        result += String.fromCharCode(c);
        i += 1;
      } else if (c > 191 && c < 224) {
        if (i + 1 >= buf.length) {
          throw "UTF-8 decode failed: two byte character was truncated";
        }
        c2 = buf[i + 1];
        result += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
        i += 2;
      } else {
        if (i + 2 >= buf.length) {
          throw "UTF-8 decode failed: multi byte character was truncated";
        }
        c2 = buf[i + 1];
        c3 = buf[i + 2];
        result += String.fromCharCode(((c & 15) << 12) | ((c & 63) << 6) | (c3 & 63));
        i += 3;
      }
    }

    return result;
  }

  _ptrToStr = function (ptr: number, len: number): string {
    let outbuffer = cipher._Module.HEAPU8.subarray(ptr, ptr + len);
    return decodeUtf8(outbuffer);
  }
} else {
  _ptrToStr = function (ptr: number, len: number): string {
    let outbuffer = cipher._Module.HEAPU8.subarray(ptr, ptr + len);
    return cipher._decoder.decode(outbuffer, { stream: true });
  };
};

const cipher = {
  _encoder: new TextEncoder(),
  _decoder: new TextDecoder(),
  _Module: await Module(),

  _ciphStr: {
    toStr(strPtr: number): string {
      const intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;
      const data = cipher._Module.HEAPU32[strPtr / intsize];
      const len = cipher._Module.HEAPU32[(strPtr + 8) / intsize];
      if (data == 0 || len == 0) return "";
      return _ptrToStr(data, len);
    },
    check(strPtr: number) {
      if (strPtr == 0) throw new Error("Allocation error");
      const intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;
      const data = cipher._Module.HEAPU32[strPtr / intsize];
      if (data == 0) throw new Error("Allocation error");
    }
  },

  // _ciphStrToStr= (str: any): string {
  //   const intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;
  //   cipher._Module.HEAPU8[str];
  //   const ptr = cipher._Module.HEAP32[str.data / intsize];
  //   const len = str.len;
  // }


  Error: class extends Error {
    code: number;

    constructor(code: number) {
      let messagePtr = cipher._Module._ciph_strerror(code);
      let message = _ptrToStr(messagePtr, cipher._Module._strlen(messagePtr));
      // switch (code) {
      //   case cipher.Err.OK: message = "OK"; break;
      //   case cipher.Err.ERR_ENCODING: message = "input is not valid UTF-8"; break;
      //   case cipher.Err.ERR_YEAR_DIGITS: message = "`year` should contain exactly 4 digits"; break;
      //   case cipher.Err.ERR_MORSE_AUDIO_INVALID_CHAR: message = "Invalid character found in morse input"; break;
      // }
      super(message!);
      this.name = "CipherError";
      this.code = code;
    }
  },

  Err: {
    OK: 0,
    ALLOC: 1,
    ERR_ENCODING: 2,
    ERR_YEAR_DIGITS: 3,
    ERR_MORSE_AUDIO_INVALID_CHAR: 4
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

  _morse_common: function(input: string, copy_non_encodable_characters: boolean): number {
    const [inputptr, inputlen] = _strToUTF8WithLength(input);

    const str: number = cipher._Module._ciph_str_new(128);
    cipher._ciphStr.check(str);

    try {
      const ret = cipher._Module._ciph_morse(
        inputptr, inputlen,
        copy_non_encodable_characters,
        str
      );

      if (ret != cipher.Err.OK) throw new cipher.Error(ret);

      return str;
    } catch {
      cipher._Module._ciph_str_delete(str);
    }
  },

  morse: function(input: string, copy_non_encodable_characters: boolean, output: (result: string) => void) {
    if (input == "") {
      output("");
      return;
    }

    // let [outputptr, outputlen]: [number | null, number | null] = [null, null];
    let str: number;
    try {
      str = cipher._morse_common(input, copy_non_encodable_characters);
      output(cipher._ciphStr.toStr(str));
    } finally {
      if (output != null) cipher._Module._ciph_str_delete(output);
    }
  },

  // TODO: fix with new code
  morse_audio: function(input: string, secs_per_dit: number, sample_rate: number, output: (wave_data: Uint8Array) => void) {
    if (input.length == 0) {
      output(new Uint8Array());
      return;
    }

    let outputStr: number;
    let outputWave: number;
    try {
      const intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;

      outputStr = cipher._morse_common(input, false);

      const outputStrPtr = cipher._Module.HEAPU32[outputStr / intsize];
      const outputStrLen = cipher._Module.HEAPU32[(outputStr + 8) / intsize];

      outputWave = cipher._Module._ciph_str_new(1024);
      const err = cipher._Module._ciph_morse_to_audio(
        outputStrPtr, outputStrLen,
        secs_per_dit, sample_rate,
        outputWave
      );
      if (err != cipher.Err.OK) throw new cipher.Error(err);

      const outputWavePtr = cipher._Module.HEAPU32[outputWave / intsize];
      const outputWaveLen = cipher._Module.HEAPU32[(outputWave + 8) / intsize];

      output(cipher._Module.HEAPU8.subarray(outputWavePtr, outputWavePtr + outputWaveLen));
    } finally {
      if (outputStr != null) cipher._Module._ciph_str_delete(outputStr);
      if (outputWave != null) cipher._Module._ciph_str_delete(outputWave);
    }
  },

  numbers: function(input: string, copy_non_encodable_characters: boolean, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }

    let [inputptr, inputlen] = _strToUTF8WithLength(input);

    const str: number = cipher._Module._ciph_str_new(128);
    cipher._ciphStr.check(str);

    try {
      let ret = cipher._Module._ciph_numbers(
        inputptr, inputlen,
        copy_non_encodable_characters,
        str
      );

      if (ret != cipher.Err.OK) throw new cipher.Error(ret);

      output(cipher._ciphStr.toStr(str))
    } finally {
      cipher._Module._free(inputptr);
      cipher._Module._ciph_str_delete(str);
    }
  },

  block_method: function (input: string, output: (result: string) => void) {
    if (input.length == 0) {
      output("");
      return;
    }
    let [inputptr, inputlen] = _strToUTF8WithLength(input);

    const str: number = cipher._Module._ciph_str_new(128);
    cipher._ciphStr.check(str);

    try {
      let ret = cipher._Module._ciph_block_method(
        inputptr, inputlen,
        str
      );
      if (ret != cipher.Err.OK) throw new cipher.Error(ret);

      output(cipher._ciphStr.toStr(str));
    } finally {
      cipher._Module._free(inputptr);
      cipher._Module._ciph_str_delete(str);
    }
  },

  /// Parse the substitution
  _sub_parse: function (
    substitutions: Map<string, string> | { [key: string]: string },
    cat_subs: Map<(codepoint: number) => boolean, string> | [[(codepoint: number) => number, string]] | null,
    free: boolean,
    singular: boolean,
    // result is a ciph_sub_t (void*)
    output: (result: number) => void
  ) {
    const substitutions_size = (substitutions instanceof Map) ? substitutions.size : Object.entries(substitutions).length;
    const subsptr = cipher._Module._ciph_sub_entries_create(substitutions_size);
    if (subsptr == 0) throw new Error("Allocation error");

    const intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;
    let subptrptr = cipher._Module._calloc(1, intsize);
    if (subptrptr == 0) {
      cipher._Module._ciph_sub_entries_free(subsptr);
      throw new Error("Allocation error");
    }

    let ptrs: number[] = [];

    try {
      // Lookups to C
      let i = 0;
      const subs = (substitutions instanceof Map) ? substitutions : Object.entries(substitutions);
      for (let [lookup, replacement] of subs) {
        const [lookupptr, lookuplen] = _strToUTF8WithLength(lookup);
        const [replacementptr, replacementlen] = _strToUTF8WithLength(replacement);
        ptrs.push(lookupptr);
        ptrs.push(replacementptr);

        cipher._Module._ciph_sub_entries_add_entry(subsptr, lookupptr, lookuplen, replacementptr, replacementlen, i);
        i += 1;
      }

      const err = cipher._Module._ciph_sub_parse(subsptr, substitutions_size, subptrptr);
      if (err != cipher.Err.OK) throw new cipher.Error(err);

      const subptr = cipher._Module.HEAP32[subptrptr / intsize];
      for (let [is_cat, replacement] of cat_subs) {
        const [replacementptr, replacementlen] = _strToUTF8WithLength(replacement);
        ptrs.push(replacementptr);
        cipher._Module._ciph_sub_add_cat(subptr, is_cat, replacementptr, replacementlen);
      }

      cipher._Module._ciph_sub_set_cat_singular(subptr, singular);

      output(subptr);
    } finally {
      const subptr = cipher._Module.HEAP32[subptrptr / intsize];
      if (subptr != 0 && free)
        cipher._Module._ciph_sub_free(subptr);
      cipher._Module._free(subptrptr);

      cipher._Module._ciph_sub_entries_free(subsptr);
      for (let ptr of ptrs) {
        cipher._Module._free(ptr);
      }
    }
  },

  sub_parse: function (
    substitutions: Map<string, string> | { [key: string]: string },
    cat_subs: Map<(codepoint: number) => boolean, string> | [[(codepoint: number) => number, string]] | null,
    singular: boolean,
    // result is a ciph_sub_t (void*)
    output: (result: number) => void
  ) {
    return cipher._sub_parse(substitutions, cat_subs, true, singular, output);
  },

  sub: function (
    input: string,
    // ciph_sub_t
    _sub: number | any /* unmanaged.Substitution */,
    // substitution_alphabet: string[],
    // char_sep: string,
    // word_sep: string,
    // sentence_sep: string,
    // copy_non_encodable_characters: boolean,
    output: (result: string) => void
  ) {
    if (input.length == 0) {
      output("");
      return;
    }

    const sub = (typeof _sub === "number") ? _sub : _sub.ptr;

    const [inputptr, inputlen] = _strToUTF8WithLength(input);
    const outputptr = cipher._Module._ciph_str_new(inputlen);

    try {
      const err = cipher._Module._ciph_sub(
        inputptr, inputlen,
        sub, outputptr
      );
      if (err != cipher.Err.OK) throw new cipher.Error(err);

      const outputstr = cipher._ciphStr.toStr(outputptr)
      output(outputstr);
    } finally {
      cipher._Module._free(input);
      cipher._Module._ciph_str_delete(outputptr);
    }
  },

  substitution: {
    _kenny: function (free: boolean, output: (sub: number) => void): number {
      const intsize = cipher._Module.HEAP32.BYTES_PER_ELEMENT;
      let subptrptr = cipher._Module._calloc(1, intsize);
      if (subptrptr == 0) {
        throw new Error("Allocation error");
      }
      let subptr = 0;

      try {
        const err = cipher._Module._ciph_sub_kenny_lang(subptrptr);
        if (err != cipher.Err.OK) {
          throw new cipher.Error(err);
        }
        subptr = cipher._Module.HEAP32[subptrptr / intsize];
        output(subptr);
      } catch {
        cipher._Module._free(subptrptr);
        subptrptr = 0;
        if (subptr != 0)
          cipher._Module._ciph_sub_free(subptr);
      } finally {
        if (subptrptr != 0)
          cipher._Module._free(subptrptr);
        if (free && subptr != 0) {
          cipher._Module._ciph_sub_free(subptr);
        }
      }

      return subptrptr;
    },
    kenny: function (output: (sub: number) => void) {
      cipher.substitution._kenny(true, output);
    }
  },

  year: function(
    input: string,
    year: string,
    include_bitmask: number,
    output: (result: string) => void
  ) {
    if (input.length == 0) {
      output("");
      return;
    }
    if (year.length != 4)
      throw new cipher.Error(cipher.Err.ERR_YEAR_DIGITS);

    let [inputptr, inputlen] = _strToUTF8WithLength(input);
    const str: number = cipher._Module._ciph_str_new(128);
    cipher._ciphStr.check(str);
    let yearptr = cipher._Module._malloc(4);

    for (let i = 0; i < 4; i += 1) {
      cipher._Module.HEAPU8[yearptr + i] = parseInt(year[i], 10);
    }

    try {
      cipher._Module._ciph_year(
        inputptr, inputlen,
        yearptr,
        include_bitmask,
        str
      );

      output(cipher._ciphStr.toStr(str));
    } finally {
      cipher._Module._free(inputptr);
      cipher._Module._ciph_str_delete(str);
      cipher._Module._free(yearptr);
    }
  },

  include_bitmasks: {
    letters: function () { return cipher._Module._ciph_char_include_letters(); },
    numbers: function () { return cipher._Module._ciph_char_include_numbers(); },
    symbols: function () { return cipher._Module._ciph_char_include_symbols(); },
    dashes: function () { return cipher._Module._ciph_char_include_dashes(); },
  },

  character_category: {
    is_wrdbrk: function () { return cipher._Module._ciph_fnptr_uc_is_wordbreak(); },
    is_sentence_terminal: function () { return cipher._Module._ciph_fnptr_uc_is_sentence_terminal(); },
  },

  /** @module unmanaged
   * These functions return classes that wouldl leak memory if `destroy` is not called
   */
  unmanaged: {
    Substitution: class {
      ptr: number;

      constructor(ptr: number) {
        this.ptr = ptr;
      }

      destroy() {
        cipher._Module._ciph_sub_free(this.ptr);
      }
    },
    sub_parse: function (
      substitutions: Map<string, string> | { [key: string]: string },
      cat_subs: Map<(codepoint: number) => boolean, string> | [[(codepoint: number) => number, string]] | null,
      singular: boolean
    ): any {
      let ptr: number;
      cipher._sub_parse(substitutions, cat_subs, false, singular, (n: number) => { ptr = n; })
      return new cipher.unmanaged.Substitution(ptr);
    },
    substitution: {
      kenny: function (): any {
        let sub: cipher.unmanaged.Substitution;
        cipher.substitution._kenny(false, (ptr: number) => {
          sub = new cipher.unmanaged.Substitution(ptr);
        });
        return sub;
      }
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
      return res;
    },
    reverse_words: function(input: string): string {
      let res: string;
      cipher.reverse_words(input, (e: string) => res = e.repeat(1));
      return res;
    },
    caesar: function(input: string, shift: number): string {
      let res: string;
      cipher.caesar(input, shift, (e: string) => res = e.repeat(1));
      return res;
    },
    alphabet_lookup: function(input: string, lookup: string): string {
      let res: string;
      cipher.alphabet_lookup(input, lookup, (e: string) => res = e.repeat(1));
      return res;
    },
    alphabet: {
      atbash: (): string => cipher.alphabet.atbash(),
      vignere: function(word: string): [string, string] {
        let res: [string, string];
        cipher.alphabet.vignere(word, (alph: string, visualize: string) => res = [alph, visualize]);
        return res;
      }
    },
    morse: function(input: string, copy_non_encodable_characters: boolean): string {
      let res: string;
      cipher.morse(input, copy_non_encodable_characters, (e: string) => res = e.repeat(1));
      return res;
    },
    /** Transform morse to audio wave data */
    morse_audio: function(input: string, secs_per_dit: number, sample_rate: number): Uint8Array {
      let res: Uint8Array;
      cipher.morse_audio(input, secs_per_dit, sample_rate, (wave_data) => {
        res = new Uint8Array(wave_data.byteLength);
        res.set(wave_data, 0);
      });
      return res;
    },
    /** Transform morse to an audio wav file */
    morse_audio_file: function (input: string, secs_per_dit: number, sample_rate: number): Uint8Array {
      let res: Uint8Array;
      cipher.morse_audio(input, secs_per_dit, sample_rate, (wave_data) => {
        const f = new WaveFile(AudioFormat.PCM, 1, sample_rate, 16, wave_data);
        res = f.toBytes();
      });
      return res;
    },
    numbers: function(input: string, copy_non_encodable_characters: boolean): string {
      let res: string;
      cipher.numbers(input, copy_non_encodable_characters, (e: string) => res = e.repeat(1));
      return res;
    },
    block_method: function(input: string): string {
      let res: string;
      cipher.block_method(input, (e: string) => res = e.repeat(1));
      return res;
    },
    sub: function (
      input: string,
      sub: number | any /* unmanged.Substitution */
      // substitution_alphabet: string[],
      // char_sep: string,
      // word_sep: string,
      // sentence_sep: string,
      // copy_non_encodable_characters: boolean,
    ) {
      let res: string;
      cipher.sub(
        input, sub,
        // char_sep, word_sep, sentence_sep,
        // copy_non_encodable_characters,
        (output: string) => res = output.repeat(1)
      );
      return res;
    },
    year: function(
      input: string,
      year: string,
      include_bitmask: number,
    ) {
      let res: string;
      cipher.year(input, year, include_bitmask, (output: string) => res = output.repeat(1));
      return res;
    }
  }
};

export default cipher;
