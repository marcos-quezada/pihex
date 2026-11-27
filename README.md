# PiHex

A pocket-sized FreeBSD cyberdeck. A Raspberry Pi Zero W runs my own
GEM desktop port; a Raspberry Pi Pico 2 W handles the display and
input as a dedicated USB co-processor, driving a Pimoroni Display
Pack 2.0 so I never have to deal with FreeBSD's total lack of a
native ST7789 driver. There's also a `pass`-backed password manager
living in here, built as a GEM desktop app rather than its own
separate gadget.

## Why "PiHex"

My daily FreeBSD laptop is called `auryn`. This one needed a name in
the same spirit, so: PiHex.

It's a Raspberry Pi, running something in the shape of Terry
Pratchett's Hex -- the Unseen University's own computer, from the
Discworld novels -- except where Hex ran on an actual ant colony,
this one runs on silicon. And then there's the extra, unplanned
coincidence: PiHex is also the real name of a late-90s distributed
computing project that calculated hexadecimal digits of π. I didn't
go looking for that one, I just got lucky.

## Status

Milestone 0 is done: a hand-written, pure-C ST7789 driver, confirmed
working on actual hardware -- no USB host involved yet, just a static
test pattern lighting up the screen. Getting there took longer than
the firmware itself. FreeBSD's packaged `arm-none-eabi-gcc` doesn't
ship a working C++ standard library at all, and the stock toolchain
can't even target this exact chip's CPU architecture out of the box.
Both got fixed; more on that as this repo's own documentation fills
in.

## Hardware

- Raspberry Pi Zero W
- Raspberry Pi Pico 2 W (RP2350)
- Pimoroni Display Pack 2.0 (320x240 ST7789 IPS, 4 buttons, 1 RGB LED)
- Still to source: Solder Party KeebDeck Basic, PiSugar 3, an
  RTL8188EU USB Wi-Fi dongle, Pimoroni Trackball Breakout

## Why FreeBSD

Because my daily driver already is one. I'd already ported GEM to
FreeBSD and built out my own TTY/font/shell dotfiles before any of
this started -- this project leans on both rather than reinventing
them.
