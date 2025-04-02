require 'etc'
require 'fileutils'

build_dir "build"

Dir.mkdir "deps" unless Dir.exist? "deps"

unless Dir.exist? "deps/CUnit"
  sh "git clone https://gitlab.com/cunity/cunit deps/CUnit"
end

import_cmake "deps/CUnit" # TODO: macos min version

unistring_vendored = !flag("build-unistring")
unistring_dep = nil
if !unistring_vendored
  unistr = "libunistring-1.3"
  unless Dir.exist? "deps/#{unistr}"
    Dir.chdir("deps") do
      unless Dir.exist? "#{unistr}.tar.gz"
        sh "curl -O https://ftp.gnu.org/gnu/libunistring/#{unistr}.tar.gz"
      end
      sh "tar -xvf #{unistr}.tar.gz"
    end
    Dir.chdir("deps/#{unistr}") do
      begin
        sh "./configure"
        sh "make -j #{Etc.nprocessors || 0}"
      rescue => e
        Dir.rmdir("deps/#{unistr}")
        raise e
      end
    end
    Dir.chdir("deps") do
      sh "rm #{unistr}.tar.gz"
    end

    FileUtils.cp("deps/#{unistr}/config.h", "include/cipher/internal/unistring_config.h")
  end

  unistring_dep = flags(
    ["-I#{File.realpath("deps/#{unistr}/lib")}", "-I#{File.realpath("deps/#{unistr}")}"],
    [
      "-L#{File.realpath("deps/#{unistr}/lib/.libs")}",
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

C::Library(
  name: "cipher",
  sources: "src/**/*.c",
  artifacts: [:staticlib, :dynlib],
  headers: "include",
  cflags: ciph_cflags,
  dependencies: [
    unistring_dep,
  ]
)

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

pre "clean" do
  File.delete "include/cipher/internal/unistring_config.h" if File.exist? "include/cipher/internal/unistring_config.h"
  FileUtils.rm_r "deps" if Dir.exist? "deps"
end
