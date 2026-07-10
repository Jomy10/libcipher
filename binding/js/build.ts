import { parseArgs } from "util";

const { values, positionals } = parseArgs({
  args: Bun.argv,
  options: {
    config: {
      type: "string"
    },
    "always-minify": {
      type: "boolean"
    }
  },
  strict: true,
  allowPositionals: true
});

let config = values.config;
let minify = config["always-minify"] || config == "release";
// let minify = (values.minify || !values["no-minify"]) ?? true;
// let config = (values)
let mode: Bun.Target = positionals[2] as Bun.Target;

console.log("Building libcipher javascript bindings");
console.log("- mode:", mode);
console.log("- minify:", minify);

let outputs = await Bun.build({
  entrypoints: ["./libcipher.ts"],
  outdir: `../../build/js/${config}`,
  minify: minify,
  target: mode,
});

console.log(outputs);
