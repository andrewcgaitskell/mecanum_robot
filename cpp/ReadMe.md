
That Python driver is exactly what I was guessing at — nails down the motor protocol. It's a simple I2C register interface, not PWM/GPIO pins at all:

    I2C address 0x30, single-byte register + single-byte value writes, no repeated start (STOP after each write)
    Registers 1–8: PWM channels for the four motors, each motor gets two registers (one per direction) — you write the speed to one and zero to the other depending on direction, which is how the driver board's H-bridges are being toggled
    Register 0x09/0x0A: left/right "seven-colour" side lights
    Ultrasonic is plain bit-banged GPIO (trigger P15, echo pulse-width on P16, ~35ms timeout, distance = pulse_us × 0.017 cm) — nothing exotic, ports straight to CODAL

The only gotcha porting to CODAL: MicroPython's i2c.write takes a 7-bit address (0x30), but CODAL/mbed's I2C::write expects the 8-bit shifted form — so that becomes 0x60 in your C++ code.

I've put together a CODAL C++ port of this driver, keeping the same register map and method names so it's easy to cross-check against the Python.
Ran a command, created a file, read a file

A few things worth flagging before you wire this into your CODAL project:

    Address shift: 0x30 (7-bit, MicroPython convention) → 0x60 (8-bit, mbed/CODAL convention). Get this wrong and every write silently NACKs.
    uBit.i2c.write signature varies a bit by CODAL/microbit-v2-samples version — some expect a ManagedBuffer, others a raw char* + length. Check your exact header before compiling; I've used the common write(addr, buf, len, repeated) form.
    sleep(5) in the Python __init__ is 5 seconds, not 5ms — that's presumably giving the driver board time to boot/settle after power-on. I kept that in the constructor; worth confirming it's actually needed rather than cargo-culted.
    Ultrasonic timing: I wrote a manual pulseIn using system_timer_current_time_us() as a busy-wait polling loop, since CODAL doesn't have a direct equivalent to MicroPython's machine.time_pulse_us. This is functionally right but a naive busy-wait will block your whole event loop for up to 35ms in the worst case (echo timeout) — same as you found with your line-following work, this is a candidate for an interrupt-driven or fiber-yielding approach if it fights your other real-time tasks (radio, PID loop).
    The LED registers (0x09/0x0A) just take a single byte in the Python — I don't know what that byte encodes (on/off? a colour index? PWM brightness?) since Keyestudio's example never varies it beyond 0. Worth probing empirically if you want the lights.

If you want, send over the ultrasonic/other functions from the Python module (if there's more of the file) and I'll fold them in.



A few notes before you flash this:

Wheel identification is a guess — I don't actually know which physical corner "Upper L / Upper R / Lower L / Lower R" corresponds to on the chassis (front-left/right vs rear-left/right). The test runs each one individually with a serial print naming it, so the first run is really a mapping exercise: watch which wheel spins, note it against the printed name, and rename the methods in the driver if you want them to read as front/rear/left/right instead.
TEST_SPEED = 60 is a conservative starting guess — no idea what PWM range this driver board actually expects (could be 0–255 raw duty, could be scaled differently). Start here, bump it if nothing moves.
Direction convention (state 1 vs 0) is untested — the Python only shows the register logic, not which physical direction each state produces. Wheels might all need to invert to get a straight line rather than a slow rotation, especially on Mecanum wheels where forward can look wrong if even one wheel's polarity is flipped.
I put a car.stopAll() right after uBit.init() before anything else runs, in case the board powers up mid-command from a previous session.
The void (MecanumCarDriver::*fn)(uint8_t, uint8_t) member-function-pointer pattern in the two testWheel* helpers is just to avoid repeating the same four lines eight times — functionally identical to calling each motor method directly if you'd rather have it flat and easier to step through in a debugger.

Let me know what you actually see when you run it (especially wheel mapping and whether state=1 is forward or reverse) and I'll fix the driver's naming/semantics to match reality rather than the guesses above.
