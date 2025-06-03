let mode: Bun.Target = "browser";
if (Bun.argv.length == 3)
  mode = Bun.argv[2] as Bun.Target;

let outputs = await Bun.build({
  entrypoints: ["./libcipher.ts"],
  outdir: "../../build/js",
  minify: true,
  target: mode
});

console.log(outputs);
