# Micron: Minimal operating system for Teensy microcontrollers
Copyright 2021 Rena Kunisaki.
Published under the BSD License.
Includes lots of code from the [Teensyduino project](http://www.pjrc.com/teensy/teensyduino.html).

This is a work in progress. It includes drivers for most of the onboard
peripherals, but not all. Some (particularly USB, SPI, I2C) are buggy and
unfinished. Expect APIs to change.

## What?
This is a library designed to assist in programming for Teensy microcontrollers
without the Arduino IDE or Teensyduino libraries. All you need is gcc, compiled
for `arm-none-eabi` target.

## Why?
1. Arduino IDE is annoying
2. Low-level coding is fun

## Installing
The OS is distributed as a set of C source and header files. Because of the
wide variety of possible configurations (e.g. for different CPU speeds and
different MCUs), these source files are compiled into each project. Therefore,
there's nothing to compile to install the library itself; you just need to
run `make install`. (Probably you'll need to run it as root if you want to use
the default install path, `/usr/local/micron`.)

## Usage
1. Create a makefile which defines at least a project name and includes the
  appropriate .mk file. (See the examples.)
2. In your code, `#include <micron.h>`.
3. Write your `int main(void)` function as usual.

If you're used to Arduino's `setup()` and `loop()` functions, they can be
implemented like so:
```C
int main() {
	setup();
	while(1) loop();
}
```

This code is designed to be easily readable, so a quick look at the source
files and included examples should help give an idea of what's available and
how to use it. (Proper documentation still to come... sorry!) The file
[REFERENCE.md](REFERENCE.md) offers more details.

The code is also designed to be lean, modular and extensible. Many of the
built-in functions are weak symbols, so your program can override them as
needed.

## Credits
Much of this code was taken directly from the excellent
[Teensyduino library](https://github.com/PaulStoffregen/cores). (Some of it was
reformatted and tidied up, but otherwise mostly unchanged.) That code is by
Paul J Stoffregen and Robin C Coon, who also designed the Teensy board.
