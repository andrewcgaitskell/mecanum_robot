/*
 * MecanumCarDriver.h
 *
 * CODAL C++ port of Keyestudio's MicroPython "Mecanum_Car_Driver_V2" class
 * for the KS4031 / KS4032 4WD Mecanum Robot driver board.
 *
 * Protocol reverse-engineered from Keyestudio's own MicroPython example:
 *   - I2C address 0x30 (7-bit) -> 0x60 (8-bit, CODAL/mbed convention)
 *   - Each write is [register, value], no repeated start (STOP after each)
 *   - Registers 1-8: motor PWM channels, two per motor (direction pair)
 *   - Register 0x09 / 0x0A: left / right side "seven-colour" LEDs
 *   - Ultrasonic (P15 trigger / P16 echo) is plain bit-banged GPIO,
 *     NOT part of this I2C driver - handle it separately with
 *     uBit.io.P15 / uBit.io.P16 and a pulse-width timeout loop.
 *
 * NOTE: adjust the codal::I2C / Pin references below to match how your
 * project constructs them (uBit.i2c vs a standalone codal::I2C instance,
 * uBit.io.P8 etc). This is written against the common microbit-v2-samples
 * style (MicroBit uBit;  uBit.i2c.write(...)).
 */

#ifndef MECANUM_CAR_DRIVER_H
#define MECANUM_CAR_DRIVER_H

#include "MicroBit.h"

class MecanumCarDriver
{
public:
    // addr is 8-bit (shifted) I2C address: Python's 0x30 -> 0x60 here
    explicit MecanumCarDriver(MicroBit &ubit, uint16_t addr = 0x60)
        : uBit(ubit), i2cAddr(addr), lastEchoDuration(0)
    {
        setAllPwm(0);
        leftLed(0);
        rightLed(0);
        uBit.sleep(5000); // Python does sleep(5) -> 5 seconds, not 5 ms
    }

    // --- low-level register access -----------------------------------

    void setPwm(uint8_t channel, uint8_t value)
    {
        uint8_t buf[2] = { channel, value };
        uBit.i2c.write(i2cAddr, (const char *)buf, 2, false); // false = send STOP
    }

    void setAllPwm(uint8_t value)
    {
        for (uint8_t ch = 0x01; ch <= 0x08; ch++)
            setPwm(ch, value);
    }

    // --- side lights ----------------------------------------------------

    void leftLed(uint8_t state)  { setPwm(0x09, state); }
    void rightLed(uint8_t state) { setPwm(0x0A, state); }

    // --- motors -----------------------------------------------------------
    // stateL/stateR: 1 or 0 selects direction; speed: 0-255 (register is uint8_t)

    void motorUpperL(uint8_t state, uint8_t speed)
    {
        if (state == 1) { setPwm(3, 0);     setPwm(4, speed); }
        else             { setPwm(3, speed); setPwm(4, 0);     }
    }

    void motorLowerL(uint8_t state, uint8_t speed)
    {
        if (state == 1) { setPwm(7, 0);     setPwm(8, speed); }
        else             { setPwm(7, speed); setPwm(8, 0);     }
    }

    void motorUpperR(uint8_t state, uint8_t speed)
    {
        if (state == 1) { setPwm(1, 0);     setPwm(2, speed); }
        else             { setPwm(1, speed); setPwm(2, 0);     }
    }

    void motorLowerR(uint8_t state, uint8_t speed)
    {
        if (state == 1) { setPwm(5, 0);     setPwm(6, speed); }
        else             { setPwm(5, speed); setPwm(6, 0);     }
    }

    void stopAll() { setAllPwm(0); }

    // --- ultrasonic (bit-banged GPIO, not I2C) -----------------------------
    // trigPin/echoPin: pass uBit.io.P15 / uBit.io.P16 (or your project's pins)
    // Returns distance in cm, same 0.017 scale factor as the Python original.

    int getDistanceCm(codal::Pin &trigPin, codal::Pin &echoPin)
    {
        trigPin.setDigitalValue(0);
        system_timer_wait_us(2);
        trigPin.setDigitalValue(1);
        system_timer_wait_us(15);
        trigPin.setDigitalValue(0);

        uint32_t t = pulseIn(echoPin, 1, 35000); // 35ms timeout, matches Python

        if (t == 0 && lastEchoDuration > 0)
            t = lastEchoDuration;

        lastEchoDuration = t;
        return (int)(t * 0.017f + 0.5f);
    }

private:
    MicroBit &uBit;
    uint16_t i2cAddr;
    uint32_t lastEchoDuration;

    // Minimal pulseIn helper: waits for pin to reach `state`, times how long
    // it stays there, aborts after timeoutUs. Mirrors MicroPython's
    // machine.time_pulse_us(). If your CODAL version doesn't expose a
    // microsecond-resolution busy wait, swap system_timer_wait_us for your
    // own tight polling loop using system_timer_current_time_us().
    uint32_t pulseIn(codal::Pin &pin, int state, uint32_t timeoutUs)
    {
        uint32_t start = system_timer_current_time_us();
        while (pin.getDigitalValue() != state)
        {
            if (system_timer_current_time_us() - start > timeoutUs)
                return 0;
        }
        uint32_t pulseStart = system_timer_current_time_us();
        while (pin.getDigitalValue() == state)
        {
            if (system_timer_current_time_us() - pulseStart > timeoutUs)
                return 0;
        }
        return system_timer_current_time_us() - pulseStart;
    }
};

#endif // MECANUM_CAR_DRIVER_H
