# Table of Contents
1. GPIO Pins
2. GCC Macros
3. Interrupts
4. Time and Idling
5. Serial UART
6. Bootup
7. malloc
8. File/stream I/O
  8.1 File open functions
  8.2 File methods


# 1. GPIO Pins
`pin_bits.h` and `pin_regs.h` copy (and add to) some of the pin definitions
from the Teensyduino library, such as `CORE_PINn_BIT` and
`CORE_PINn_PORTREG`. These can be used in your code; they're also expanded upon
by Micron.

`teensy3*.h` define similar macros which use the aliased bitband I/O
feature of the Cortex-M4. Each individual bit of the GPIO registers is also
accessible via a 32-bit word of its very own. (The lowest bit of that word
corresponds to the bit in question; the other 31 are unused.) This allows
somewhat faster and more convenient access to the individual GPIO bits, as you
don't need to do any bit twiddling; just read/write 0 or 1 to the register.

`pins.h` and `pins.c` define macros and functions for accessing arbitrary GPIO
registers by pin number. These functions take advantage of gcc's
`__builtin_constant_p()` feature to generate optimal code; a statement such as
`digitalWrite(13, n)` optimizes down to a single machine instruction. The pin
number need not be a constant either (though the code will be more than one
instruction in this case). These files also define friendly, readable names
for the Pin Control Register bits.


# 2. GCC Macros
`gcc-macros.h` defines a number of macros which provide hints to the compiler
to help optimize your code and catch bugs.
Some of these macros may not be available on older versions of gcc or on other
compilers. Since they're only optimization hints, you can re-define them to do
nothing if your compiler doesn't support them. Additionally, you can define
`NO_GCC_MACROS` to disable them all.


# 3. Interrupts
`isr.h` and `isr.c` define the default Interrupt Service Routine handlers.
Mostly, these are aliased to `fault_isr()`, which simply hangs the program.
Since these are weak symbols, your program can override them by simply
defining its own handlers with the same names.

`isr.h` also defines the `_VectorsRam` table, which holds the pointer to each
ISR handler. Your program can change these entries at will.

`isr.c` defines two default handlers:
* `fault_isr()`: Handler for entries in the table which aren't normally used.
 (The interrupt vector table on the MK20 chip has some unused slots.) This
 handler never returns, but continues to manually run some other ISRs (if
 they're defined), so that some debugging is still possible.
* `unused_isr()`: Handler for entries in the table which are used, but which
 your program hasn't defined another handler for. The default implementation
 simply calls `fault_isr()`.

A good idea would be to override one or both of these to blink an LED and/or
output some debug message, via whatever means your design has available.

Additionally, the ARM standard(?) macros `__disable_irq()` and
`__enable_irq()` (defined in `kinetis.h`) can be used to disable and
enable interrupts.


# 4. Time and Idling
`time.h` and `time.c` define a few functions for timing and delaying:

`micros()`: Returns the number of microseconds that have elapsed. Note that
 this is a 32-bit counter, so will overflow approximately once every 71
 minutes. Despite the overflow, this should still work:
 ```C
 uint32_t start = micros();
 do_something_real_quick();
 uint32_t end = micros();
 uint32_t elapsed = end - start;
 ```
 It's possible that `end < start`; however, `elapsed` will still be valid, as
 long as fewer than 71 mintues have passed between the two calls to `micros()`.

`millis()`: Returns the number of milliseconds that have elapsed. The same
 caveats as `micros()` apply, except this counter overflows approximately
 once every 49.7 days.

`delayMS(uint32_t n)`: Delay for `n` milliseconds. May not be perfectly precise,
 especially if interrupts are enabled. This is a busy loop, so using it to delay
 for a long period will harm battery life (and heat up the CPU).

`delayUS(uint32_t n)`: Delay for `n` microseconds. Should be precise as long as
 interrupts are disabled. Has only been tested at 72mhz; in particular if using
 a CPU clock less than 3mhz, you should test this first. This macro also won't
 work correctly if you change the CPU speed after startup.

`rtcSet(unsigned long n)`: Sets the real-time clock registers to the time
 `t` (a Unix timestamp). Note that you need to add a crystal to the Teensy board
 to actually use the real-time clock.
 This function is called by the default startup code.

`waitForInterrupt()`: Waits for an interrupt. This consumes less power (and
 heats the CPU less) than a busy loop. It can be used even if interrupts are
 disabled; in that case, when an interrupt would trigger, waiting will stop
 but the ISR won't be called until interrupts are enabled again.
 Note that by default, the systick interrupt fires once per millisecond, so this
 macro won't delay more than ~1ms while that is enabled.

`idle()`: Similar to `waitForInterrupt()`. Uses the ARM `wfe` instruction
 instead of `wfi`. The actual difference between these instructions is somewhat
 vague, but from my understanding, `wfe` is used to stall when there's nothing
 else to do (and can be implemented as a no-op on some chip designs), whereas
 `wfi` is used to explicitly wait for an interrupt.

Note that `millis()`, `micros()`, and `delayMS()` use the `systick_millis_count`
 variable. If you provide your own systick ISR, you will need to make sure to
 update this variable if you want to use these functions.


# 5. Serial UART
The Teensy board has three pairs of pins connected internally to UART modules:
``` ╔════╦═════════╦══════╦═════╦═════╦═════╦═══════════╗
    ║    ║  Pins   ║ Clock║ FIFO║ ISO ║     ║ Base      ║
    ║UART║ Rx   Tx ║  Src ║ Size║ 7816║ LON ║ Address   ║
    ╠════╬════╦════╬══════╬═════╬═════╬═════╬═══════════╣
    ║ #0 ║  0 ║  1 ║ CPU  ║   8 ║ Yes ║ Yes ║ 0x4006A000║
    ║ #1 ║  9 ║ 10 ║ CPU  ║   8 ║ No  ║ No  ║ 0x4006B000║
    ║ #2 ║  7 ║  8 ║ Bus  ║   1 ║ No  ║ No  ║ 0x4006C000║
    ╚════╩════╩════╩══════╩═════╩═════╩═════╩═══════════╝
```
Among other things, these can be useful for connecting to a PC for debugging.
Simply connect the Tx and Rx pins of your serial port (or USB adaptor) to the
appropriate pins on the board and use the serial API to communicate.
On Linux, you can use a command such as: `screen /dev/ttyUSB0 9600` (insert
whatever baud rate you use).

Note that this implementation uses `malloc()` to allocate buffers and state
information when a serial port is opened.

All three ports also support IrDA, but Micron currently does not provide
any API for it.

`int serialInit(uint8_t port, uint32_t baud)`: Initializes a serial port.
`port` is the number of the UART (0..NUM_UART). `baud` is the desired baud rate.
This function returns 0 on success or a negative error code on failure.
It takes care of turning on the UART module and setting up buffers and
interrupts handlers.

`void serialShutdown(uint8_t port)`: Shuts down a serial port. Specifically,
turns off the UART module and frees the memory allocated for buffers. This
function does not change the pin settings.

`int serialSend(uint8_t port, const void *data, uint32_t len)`:
Sends some data out over the serial port. This function does not block if the
buffers are full; it returns the number of bytes sent (or negative error code
on failure). A return of less than `len`, and especially a return of zero,
indicates the transmit buffer is full and you might want to wait for an
interrupt before retrying.

`int serialReceive(uint8_t port, char *data, uint32_t len)`:
Receives up to `len` bytes of buffered data from the serial port into the
given buffer `data`. Like `serialTransmit`, this function does not wait for data
to arrive, but returns the number of bytes received.

`void serialFlush(uint8_t port)`: Waits for all transmitting on the specified
port to finish before returning.

`void serialClear(uint8_t port)`: Discards all buffered data received from the
serial port.

`int serialPutchr(uint8_t port, char c)`: Writes a single character to the
serial port. This function blocks until it's able to send, and returns the
number of bytes sent (which should always be 1), or a negative error code on
failure.

`int serialPuts(uint8_t port, const char *str)`: Writes a string to the
serial port. This function blocks until it's finished sending the entire string.
Unlike the POSIX `puts()` function, this does not append a line break.
On success, it returns the number of bytes sent; on failure it returns a
negative error code.

`int serialGetchr(uint8_t port)`: Reads one character from the serial port.
This function does *not* block, and returns zero if no character was received.
On success it returns the received character; on failure it returns a negative
error code.
You can block until a character is received like so:
`int c; while(!(c = serial_getchr(port))) waitForInterrupt();`

`int serialGets(uint8_t port, char *str, uint32_t len)`: Reads a line from the
serial port. This function blocks until it receives a line break or fills the
given buffer `str`. It appends a NULL terminator to the buffer, but does not
append the line break character. It returns the number of bytes received on
success, or a negative error code on failure.

See `examples/serial` for a more detailed example.


# 6. Bootup
The subdirectory `boot` contains files used during the startup process. These,
too, get compiled into each project, since different code is generated for
different settings. For the most part you won't need to bother with these, but
they're available if you want to change the boot code.

`startup.c` defines the very first code that executes at power-on
(the `ResetHandler` function). This code is mostly copied from Teensyduino,
broken up into individual (inline) functions for clarity. (Teensyduino also
enables some ISRs which this code doesn't.)
In particular, if you want to change how the power control and clock generation
registers are set at bootup, the code to change is here.
This file also defines the default implementations of two hook functions called
during the boot process:
`startupEarlyHook()`: Called early in the boot process, before anything (except
the watchdog timer) is initialized. You can override this function if you want
to use the watchdog timer or do other early initialization. Be careful: this is
called before RAM, CPU clocks, etc are initialized.
`startupLateHook()`: Called just before `main()`.

`isr-vectors.c` defines the default ISR table. Probably you shouldn't need to
change this, since the default ISR handlers are all weak symbols, so your
program can override them by simply defining a function with the same name.
However, this file can be a useful reference to find those names.

`flashconfig.c` defines the Cortex-M4 flash config section. It's unlikely you'd
ever need to change this. If you do, follow the instructions in the file,
carefully.


# 7. malloc
`malloc.c` contains a simple `malloc` implementation, optimized for little
overhead. It also provides a weak symbol:
`WEAK COLD void* on_malloc_fail(size_t len);`
This function is called whenever `malloc` fails, and its return value is the
value returned by `malloc` in that case. The default handler simply returns
`NULL`, but you can override it to do something helpful such as report the
problem to a debug console via UART.


# 8. File/stream I/O
`io.h` describes a simple file/stream I/O API resembling POSIX. This API does
not differentiate between files and streams (since files aren't really used here
anyway). Thus we use the term "file" to refer to both files and streams.
Micron does not implement a filesystem; rather, there are separate functions to
open different types of files. Regardless of their type however, they all share
the same methods.
Internally, the I/O API is implemented using a table of pointers to methods for
each type of file. This makes it relatively simple to write a new I/O driver.
Refer to the serial driver for a simple example.

## 8.1 File open functions
The following functions are defined to open files. Each returns a file
descriptor (a positive integer, sometimes referred to as "fd") on success, or a
negative error code on failure. Note that 0 is a valid file descriptor (and that
unlike on most systems, there are no file descriptors open by default; ie there
are no stdin/stdout/stderr unless you define them yourself).

`int openSerial(uint8_t port, uint32_t baud)`: Opens one of the on-board serial
UART ports. The Teensy board has 3 ports, numbered 0, 1 and 2 in Micron.
(XXX they're numbered 1, 2, 3 on the diagram sheet...)
The maximum supported baud rate depends on the CPU clock.
This method takes care of initializing the port, UART module, etc.

## 8.2 File methods
The following methods are defined for operating on files. Each takes a file
descriptor as its first parameter and, unless otherwise noted, returns a
negative error code on failure.

`int read (int fd, char *dest, size_t len)`: Read up to `len` bytes from the
file into the buffer `dest`. This method does not block. On success, it returns
the number of bytes read (which may be anywhere from zero to `len`).

`int write(int fd, const void *src, size_t len)`: Write up to `len` bytes to
the file from the buffer `src`. This method does not block. On success, it
returns the number of bytes written (which may be anywhere from zero to `len`).

`int close(int fd)`: Close the file. After this, the file descriptor is no
longer valid (and may quickly get recycled and refer to another file). On
success, it returns zero.

`int readSync (int fd, char *dest, size_t len)`
`int writeSync(int fd, const void *src, size_t len)`: These work the same as
`read` and `write`, but they block until they've read/written the requested
number of bytes or encountered an error.
