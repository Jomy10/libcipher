require 'etc'

build_dir "build"

Dir.mkdir "deps" unless Dir.exist? "deps"

unless Dir.exist? "deps/CUnit"
  sh "git clone https://gitlab.com/cunity/cunit deps/CUnit"
end

import_cmake "deps/CUnit"

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
end

Project(name: "libcipher")

C::Library(
  name: "cipher",
  sources: "src/**/*.c",
  artifacts: [:staticlib, :dynlib],
  headers: "include",
  dependencies: [
    flags(
      ["-Ideps/#{unistr}/lib"],
      [
        "-Ldeps/#{unistr}/lib/.libs",
        "-lunistring"
      ]
    )
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
  cflags: test_cflags
)

cmd "test" do
  project("libcipher").target("test").run([])
end
