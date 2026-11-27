# Firmware

Pico 2 W firmware for PiHex. Milestone 0 is a hand-written, pure-C
ST7789 driver -- no USB host involved yet, just proof that the
display, SPI, and backlight wiring all genuinely work.

## Why pure C, not Pimoroni's library

The obvious first move was to reuse Pimoroni's own `pico-graphics`/
`st7789` C++ library -- it's the maintained, known-good driver for
this exact board. It doesn't work here. FreeBSD's `devel/
arm-none-eabi-gcc` port is built with `--disable-libstdcxx`
unconditionally (it's right there in the port's own `Makefile`,
along with a comment explaining why: building libstdc++ properly
would need a two-stage bootstrap the port doesn't do). There's no
C++ standard library on this toolchain at all, not even `<cstdlib>`.
So: plain C, talking to the Pico SDK's own C API directly.

## Building on FreeBSD: the real gotchas

None of these are project bugs. All three are genuine FreeBSD
toolchain limitations, confirmed by reading the actual port sources
and FreeBSD's own bug tracker rather than guessed at. Worth knowing
before you fight them again on a fresh machine.

**1. The stock `pkg install arm-none-eabi-gcc` can't target this
chip at all.** The port's default build (`ML_NONE`, multilib
disabled) only supports one generic baseline ARM architecture --
not Cortex-M33/ARMv8-M-mainline, which is what the RP2350 actually
is. You'll see it as a wall of `conflicting CPU architectures ARM
v4T vs ARM v8-M.mainline` errors at link time, against *every*
object file, including the SDK's own prebuilt `libg.a`. Fix:
rebuild the port from source with the right multilib option.

```sh
doas pkg delete -y arm-none-eabi-gcc arm-none-eabi-newlib
cd /usr/ports/devel/arm-none-eabi-gcc
doas make config   # select ML_CORTEXRM, not the ML_NONE default
doas make install clean
cd /usr/ports/devel/arm-none-eabi-newlib
doas make install clean   # no config needed -- it reads gcc's own multilib list
```

**2. A confirmed, still-open FreeBSD ports bug contaminates the
compiler's own headers.** `fixincludes`, which runs during the
port's build, mistakenly bundles the *build host's* FreeBSD libc
headers into the cross-compiler's `include-fixed/` directory, ahead
of the real newlib ones. You'll see errors like `unknown type name
'__off64_t'` the moment anything includes `<stdio.h>`. This is
[FreeBSD bugzilla 298563](https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=298563)
-- a patch exists, it just hasn't merged yet. The workaround has to
be reapplied every time the compiler gets rebuilt (including step 1
above):

```sh
doas rm -rf \
  /usr/local/lib/gcc/arm-none-eabi/11.3.0/include-fixed/stdio.h \
  /usr/local/lib/gcc/arm-none-eabi/11.3.0/include-fixed/stdlib.h \
  /usr/local/lib/gcc/arm-none-eabi/11.3.0/include-fixed/sys \
  /usr/local/lib/gcc/arm-none-eabi/11.3.0/include-fixed/unistd.h \
  /usr/local/lib/gcc/arm-none-eabi/11.3.0/include-fixed/wchar.h
```

(Check the version number in the path still matches yours first --
`11.3.0` was current as of this writing.)

**3. The Pico SDK itself needs a sliver of C++ support, even for a
pure-C project.** `pico_runtime` unconditionally links
`pico_cxx_options`, whose `new_delete.cpp` pulls in `<cstdlib>` --
which doesn't exist here either. The file's own source comment
says "let user override," so that's what this project does:

```cmake
add_compile_definitions(PICO_CXX_DISABLE_ALLOCATION_OVERRIDES=1)
```

That compiles the file's body out entirely. The final link step
still gets routed through the C++ linker frontend, though (CMake
picks it the moment any `.cpp` file is anywhere in the build), which
then tries to link `-lc++` -- and that genuinely doesn't exist on
this toolchain. `stublibs/` is the fix: a correctly-architected but
otherwise empty stub archive, just enough to satisfy the linker
without needing any real C++ runtime at all.

## Building

Requires `PICO_SDK_PATH` set in your environment, pointing at a
pico-sdk checkout (this project is built and tested against `2.1.1`):

```sh
export PICO_SDK_PATH=~/git/tools/pico-sdk
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

`pico_sdk_import.cmake` resolves this from the environment on its
own -- no need to edit `CMakeLists.txt` to point at a different SDK
location.

## Flashing

Hold **BOOTSEL**, plug the Pico 2 W in via USB, release once
connected -- it mounts as a mass-storage device. Copy the built
`.uf2` onto it, and the board resets into the new firmware
automatically.

```sh
cp build/milestone0.uf2 /path/to/mounted/device/
```
