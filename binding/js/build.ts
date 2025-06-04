import { parseArgs } from "util";

const { values, positionals } = parseArgs({
  args: Bun.argv,
  options: {
    minify: {
      type: "boolean"
    },
    "no-minify": {
      type: "boolean"
    }
  },
  strict: true,
  allowPositionals: true
});

let minify = (values.minify || !values["no-minify"]) ?? true;
let mode: Bun.Target = positionals[2] as Bun.Target;

console.log("Building libcipher javascript bindings");
console.log("- mode:", mode);
console.log("- minify:", minify);

let outputs = await Bun.build({
  entrypoints: ["./libcipher.ts"],
  outdir: "../../build/js",
  minify: minify,
  target: mode
});

console.log(outputs);
