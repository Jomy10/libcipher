puts "compiling for #{TARGET}"

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
          sh "emmake make -j #{Etc.nprocessors || 0}"
        else
          sh "./configure CFLAGS=\"-O3 -flto -g0\" LDFLAGS=\"-O3 -flto -g0\""
          sh "make -j #{Etc.nprocessors || 0}"
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

  # /opt/homebrew/Cellar/emscripten/4.0.7/libexec/emcc -DHAVE_CONFIG_H -DNO_XMALLOC -I. -I..  -I. -I. -I.. -I.. -DIN_LIBUNISTRING -DDEPENDS_ON_LIBICONV=1 -I/opt/homebrew/opt/llvm/include -I/opt/homebrew/opt/icu4c@76/include -I/opt/homebrew/opt/sqlite/include  -g -O2 -c unictype/pr_titlecase.c
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

Project(name: "libcipher")

ciph_cflags = []
if !flag("no-audio")
  ciph_cflags << "-DCIPH_AUDIO"
end
if unistring_vendored
  ciph_cflags << "-DCIPH_UNISTRING_VENDORED"
end
if TARGET.os == "emscripten"
  ciph_cflags << "-DCIPH_OS_EMSCRIPTEN"
end

ciph_linker_flags = []
if TARGET.os == "emscripten"
  ciph_linker_flags.append *[
    "-s", "EXPORT_ALL=1",
    "-s", "EXPORTED_RUNTIME_METHODS=['ccall', 'stringToNewUTF8', 'UTF8ToString']",
    "-s", "EXPORTED_FUNCTIONS=['_malloc', '_free', '_realloc']",
    "-s", "EXPORT_ES6=1",
    "-s", "MODULARIZE=1",
    "-s", "ALLOW_MEMORY_GROWTH=1",
  ]

  # When this flag is specified, it will build the js module for running inside
  # of node instead of on the web
  if !flag("build-for-node")
    ciph_linker_flags.append *["-s", "ENVIRONMENT=web"]
  end
end

C::Library(
  name: "cipher",
  sources: "src/**/*.c",
  artifacts: TARGET.os == "emscripten" ? [:staticlib, :jslib] : [:staticlib, :dynlib],
  headers: "include",
  cflags: ciph_cflags,
  linker_flags: ciph_linker_flags,
  dependencies: [
    unistring_dep,
  ]
)

unless TARGET.os == "emscripten"
  test_cflags = []
  if flag("no-output")
    test_cflags << "-DTEST_NO_OUTPUT"
  end

  C::Executable(
    name: "test",
    sources: "tests/**/*.c",
    dependencies: ["cipher", "CUnit:cunit"],
    cflags: test_cflags,
    linker_flags: ["-mmacos-version-min=15.1"] # TODO: configure
  )

  cmd "test" do
    project("libcipher").target("test").run([])
  end
end

pre "clean" do
  File.delete "include/cipher/internal/unistring_config.h" if File.exist? "include/cipher/internal/unistring_config.h"
  FileUtils.rm_r "deps" if Dir.exist? "deps"
end
