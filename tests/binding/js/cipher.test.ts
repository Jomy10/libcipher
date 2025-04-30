import { expect, test } from "bun:test";
import cipher from "../../../binding/js/libcipher";

test("ascii", () => {
  expect(cipher.copy.ascii("ABCX z")).toBe("065 066 067 088 032 122");
});

test("reverse word", () => {
  expect(cipher.copy.reverse_words("ABC DEF. XYZ. C G")).toBe("CBA FED. ZYX. C G");
});
