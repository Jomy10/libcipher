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

let tsconfig = {
  "files": ["libcipher.ts"],
  "compilerOptions": {
    "declaration": true,
    "declarationMap": true,
    "sourceMap": true,
    "target": "es2024",
    "module": "es2022",
    "removeComments": true,
    "outDir": `../../build/js/${config}`,
    "baseUrl": ".",
    // "skipLibCheck": true,
    "paths": {
      // TODO: change to debug when debug build!!
      "@libcipher_build/*": [`../../build/wasm32-unknown-emscripten/${config}/libcipher/artifacts/*`]
    }
  }
};

await Bun.write(
  "tsconfig.json",
  JSON.stringify(tsconfig)
);

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
