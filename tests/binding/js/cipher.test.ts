import { expect, test } from "bun:test";
import cipher from "../../../binding/js/libcipher";

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
  expect(cipher.copy.morse("ABc DeF. AD", true)).toBe("·- -··· -·-· / -·· · ··-· // ·- -··");
});

test("numbers", () => {
  expect(cipher.copy.numbers("ABCX Z", true)).toBe("1 2 3 24 / 26");
});

test("block method", () => {
  expect(cipher.copy.block_method("PIONIERHOUT")).toBe("PIOXIEUXORTXNHXX");
});

test("alphabet substitution", () => {
  expect(cipher.copy.alphabet_substitution(
    "rechts",
    [
      "NNNO", "NONNO", "NOONO", "OONO", "OOZO", "ZOOZO",
      "ZOZZO", "ZZZO", "ZZZW", "ZWZZW", "ZWWZW", "WWZW",
      "WWNW", "NWWNW", "NWNNW", "NNNW", "NNNO*", "NONNO*",
      "NOONO*", "OONO*", "OOZO*", "ZOOZO*", "ZOZZO*", "ZZZO*",
      "ZZZW*", "ZWZZW*",
    ],
    ".",
    " / ",
    " // ",
    true
  )).toBe("NONNO*.OOZO.NOONO.ZZZO.OONO*.NOONO*")
});

test("year", () => {
  expect(cipher.copy.year(
    "GA NU DADELIJK TERUG NAAR HET LOKAAL",
    "1996",
    cipher.include_bitmasks.letters_and_numbers()
  )).toBe("GAJAOK NKRA UTHA DEEL ARTX DULX EGX LNX IAX")
});
