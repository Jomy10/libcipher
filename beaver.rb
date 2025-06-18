# TODO: try -Os and other opt levels for emscripten

require 'etc'
require 'fileutils'

build_dir "build"

deps_dir = "deps/#{TARGET}"
FileUtils.mkdir_p deps_dir unless Dir.exist? deps_dir

unless TARGET.os == "emscripten"
  cunit_dir = File.join("deps", "CUnit")
  unless Dir.exist?(cunit_dir)
    sh "git clone https://gitlab.com/cunity/cunit #{cunit_dir}"
  end

  import_cmake cunit_dir # TODO: macos min version
end

unistring_vendored = !flag("build-unistring")
unistring_dep = nil
if !unistring_vendored
  unistr = "libunistring-1.3"
  unistr_dir = File.join(deps_dir, unistr)
  unless Dir.exist? unistr_dir
    Dir.chdir(deps_dir) do
      unless File.exist? "#{unistr}.tar.gz"
        begin
          sh "curl -O https://ftp.gnu.org/gnu/libunistring/#{unistr}.tar.gz"
        rescue => e
          puts "Error occurred: #{e}. Trying a mirror..."
          sh "curl -O https://mirror.ibcp.fr/pub/gnu/libunistring/#{unistr}.tar.gz"
        end
      end
      sh "tar -xvf #{unistr}.tar.gz"
    end
    Dir.chdir(unistr_dir) do
      begin
        if TARGET.os == "emscripten"
          sh "emconfigure ./configure \
            --host=#{TARGET} \
            ac_cv_search_nanosleep=no \
            ac_cv_have_decl_sleep=no \
            ac_cv_func_malloc_0_nonnull=yes \
            ac_cv_func_realloc_0_nonnull=yes \
            ac_cv_func_free_0_nonnull=yes \
            --disable-threads \
            --enable-static \
            --disable-shared \
            CFLAGS=\"-O3 -flto -g0\" \
            LDFLAGS=\"-O3 -flto -g0\""
          sh "emmake make -j #{Etc.nprocessors || 0} GL_GNULIB_MALLOC_POSIX=0 GL_GNULIB_FREE_POSIX=0"
        else
          sh "./configure CFLAGS=\"-O3 -flto -g0\" LDFLAGS=\"-O3 -flto -g0\" \
            ac_cv_func_malloc_0_nonnull=yes \
            ac_cv_func_realloc_0_nonnull=yes \
            ac_cv_func_free_0_nonnull=yes \
            --enable-static \
            --disable-shared
            "
          sh "make -j #{Etc.nprocessors || 0}  GL_GNULIB_MALLOC_POSIX=0 GL_GNULIB_FREE_POSIX=0"
        end
      rescue => e
        Dir.rmdir(unistr_dir)
        raise e
      end
    end
    Dir.chdir(deps_dir) do
      sh("rm #{unistr}.tar.gz") if File.exist?("#{unistr}.tar.gz")
    end

    FileUtils.cp(File.join(unistr_dir, "config.h"), "include/cipher/internal/unistring_config.h")
  end

  unistring_dep = flags(
    [
      "-I#{File.realpath(File.join(unistr_dir, "lib"))}",
      "-I#{File.realpath(unistr_dir)}",
      "-D_GL_CONFIG_H_INCLUDED"
    ],
    [
      "-L#{File.realpath(File.join(unistr_dir, "lib/.libs"))}",
      "-lunistring"
    ]
  )
else
  # TODO: unistring_dep = system("unistring")
  unistring_dep = flags([], ["-lunistring"])
end

use_audio = !flag("no-audio")

# if use_audio
#   if !Dir.exist? "deps/libwave"
#     sh "git clone https://github.com/brglng/libwave deps/libwave"
#   end
#   import_cmake "deps/libwave", ["-DBUILD_SHARED_LIBS=NO"]
# end

Project(name: "libcipher")

sanitize = flag("sanitize", default: true)

ciph_cflags = []
if use_audio
  ciph_cflags << "-DCIPH_AUDIO"
end
if unistring_vendored
  ciph_cflags << "-DCIPH_UNISTRING_VENDORED"
end
if TARGET.os == "emscripten"
  ciph_cflags << "-DCIPH_OS_EMSCRIPTEN"
end

if OPT == "debug" && sanitize
  ciph_cflags << "-fsanitize=address"
  # ciph_cflags << "-fsanitize=leak"
  ciph_cflags << "-fno-omit-frame-pointer"
end

ciph_linker_flags = []
if TARGET.os == "emscripten"
  ciph_linker_flags.append *[
    "-s", "EXPORT_ALL=1",
    "-s", "EXPORTED_RUNTIME_METHODS=['ccall', 'stringToNewUTF8', 'UTF8ToString']",
    "-s", "EXPORTED_FUNCTIONS=['_malloc', '_free', '_realloc', '_strlen', '_ciph_strerror']",
    "-s", "EXPORT_ES6=1",
    "-s", "MODULARIZE=1",
    "-s", "ALLOW_MEMORY_GROWTH=1",
    "-s", "STRICT=1",
  ]

  if use_audio
    ciph_linker_flags.append *["-s", "STACK_SIZE=5MB"]
  end

  # When this flag is specified, it will build the js module for running inside
  # of node instead of on the web
  web_mode = !flag("build-for-node")
  if web_mode
    ciph_linker_flags.append *["-s", "ENVIRONMENT=web,worker"]
  else
    ciph_linker_flags.append *["-s", "ENVIRONMENT=node"]
  end
end

if OPT == "debug" && sanitize
  ciph_linker_flags << "-fsanitize=address"
  ciph_linker_flags << "-fno-omit-frame-pointer"
end

ciph_linker_flags.append *(opt("Xlinker")&.split(",") || [])

ciph_deps = [
  unistring_dep,
]

C::Library(
  name: "cipher",
  sources: "src/**/*.c",
  artifacts: TARGET.os == "emscripten" ? [:staticlib, :jslib] : [:staticlib],
  headers: "include",
  cflags: ciph_cflags,
  linker_flags: ciph_linker_flags,
  dependencies: ciph_deps
)

minify_js = flag("minify", default: true)
if TARGET.os == "emscripten"
# TODO: make post "build"
  cmd "build-js" do
    FileUtils.mkdir_p "build/js" unless Dir.exist? "build/js"

    Dir.chdir("binding/js") do
      sh "bun build.ts #{web_mode ? "browser" : "node"} #{minify_js ? "--minify" : "--no-minify"}"
      # sh "tsc -p ./tsconfig.json"
    end
  end
end

unless TARGET.os == "emscripten"
  test_cflags = []
  if flag("no-output")
    test_cflags << "-DTEST_NO_OUTPUT"
  end

  C::Executable(
    name: "cipher_test",
    sources: "tests/**/*.c",
    dependencies: ["cipher", "CUnit:cunit"],
    cflags: test_cflags,
    linker_flags: ["-mmacos-version-min=15.1"] # TODO: configure
  )

  cmd "test" do
    project("libcipher").target("cipher_test").run([])
  end
else
  cmd "test" do
    sh "beaver cipher -o --target #{TARGET} -- --build-unistring --build-for-node"
    Dir.chdir("tests/binding/js") do
      sh "bun test"
    end
  end
end

pre "clean" do
  File.delete "include/cipher/internal/unistring_config.h" if File.exist? "include/cipher/internal/unistring_config.h"
  FileUtils.rm_r "deps" if Dir.exist? "deps"
end
