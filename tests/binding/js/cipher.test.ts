import { expect, test } from "bun:test";
import cipher from "../../../build/js/release/libcipher";
import WaveFile from "../../../binding/js/wave";

test("ascii", () => {
  expect(cipher.copy.ascii("ABCX z")).toBe("065 066 067 088 032 122");
});

test("reverse word", () => {
  expect(cipher.copy.reverse_words("ABC DEF. XYZ. C G")).toBe("CBA FED. ZYX. C G");
});

test("caesar", () => {
  expect(cipher.copy.caesar("ABC àZ".normalize("NFD"), 1).normalize("NFC")).toBe("BCD b̀A");
});

test("atbash", () => {
  let alphabet = cipher.alphabet.atbash();
  expect(cipher.copy.alphabet_lookup("ABCz", alphabet)).toBe("ZYXa");
});

test("vignère", () => {
  let [alphabet, _] = cipher.copy.alphabet.vignere("LIMONADE");
  expect(cipher.copy.alphabet_lookup("ABCzà".normalize("NFD"), alphabet)).toBe("SVWhs̀");
});

test("alphabet lookup", () => {
  let alphabet = "ACDDEFGHIJKLMNOPQRSTUVWXYZ";
  expect(cipher.copy.alphabet_lookup("ABC", alphabet)).toBe("ACD");
});

test("morse", () => {
  expect(cipher.copy.morse("ABc DeF. AD", true)).toBe(".- -... -.-. / -.. . ..-. // .- -..");
});

test("morse audio", async () => {
  const data = cipher.copy.morse_audio("SOS", 0.3, 44100);

  const f = Bun.file("morse-audio.wav");
  const arrbuf = await f.arrayBuffer();
  const farr = new Uint8Array(arrbuf);
  const fdata = WaveFile.fileToData(farr);

  expect(data).toEqual(fdata);
})

test("numbers", () => {
  expect(cipher.copy.numbers("ABCX Z", true)).toBe("1 2 3 24 / 26");
});

test("block method", () => {
  expect(cipher.copy.block_method("PIONIERHOUT")).toBe("PIOXIEUXORTXNHXX");
});

const windroos_sub = {
  "A": "NNNO.",
  "B": "NONNO.",
  "C": "NOONO.",
  "D": "OONO.",
  "E": "OOZO.",
  "F": "ZOOZO.",
  "G": "ZOZZO.",
  "H": "ZZZO.",
  "I": "ZZZW.",
  "J": "ZWZZW.",
  "K": "ZWWZW.",
  "L": "WWZW.",
  "M": "WWNW.",
  "N": "NWWNW.",
  "O": "NWNNW.",
  "P": "NNNW.",
  "Q": "NNNO*.",
  "R": "NONNO*.",
  "S": "NOONO*.",
  "T": "OONO*.",
  "U": "OOZO*.",
  "V": "ZOOZO*.",
  "W": "ZOZZO*.",
  "X": "ZZZO*.",
  "y": "ZZZW*.",
  "Z": "ZWZZW*."
};

const windroos_cat_sub = [
  [cipher.character_category.is_sentence_terminal(), " // "],
  [cipher.character_category.is_wrdbrk(), " / "],
];

test("substitution", () => {
  cipher.sub_parse(
    windroos_sub,
    windroos_cat_sub,
    false,
    (sub: number) => {
      expect(cipher.copy.sub("rechts".toUpperCase(), sub)).toBe("NONNO*.OOZO.NOONO.ZZZO.OONO*.NOONO*.");
    }
  );
});

test("substitution sentence", () => {
  cipher.sub_parse(
    windroos_sub,
    windroos_cat_sub,
    true,
    (sub: number) => {
      expect(cipher.copy.sub("rechts rechts. rechts".toUpperCase(), sub)).toBe("NONNO*.OOZO.NOONO.ZZZO.OONO*.NOONO*. / NONNO*.OOZO.NOONO.ZZZO.OONO*.NOONO*. // NONNO*.OOZO.NOONO.ZZZO.OONO*.NOONO*.");
    }
  )
});

test("substitution kenny", () => {
  cipher.substitution.kenny((sub: number) => {
    expect(cipher.copy.sub("This is kenny language", sub).toBe("Fmpmfpmfffmm mfffmm pmpmppppppppffm pmfmmmpppmfmfmfmmmmfmmpp"));
  });
});

test("year", () => {
  expect(cipher.copy.year(
    "GA NU DADELIJK TERUG NAAR HET LOKAAL",
    "1996",
    cipher.include_bitmasks.letters() | cipher.include_bitmasks.numbers()
  )).toBe("GAJAOK NKRA UTHA DEEL ARTX DULX EGX LNX IAX")
});
