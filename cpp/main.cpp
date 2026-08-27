/*
 * main.cpp
 *
 * Simple CODAL test harness for MecanumCarDriver.h (Keyestudio KS4031/KS4032).
 *
 * What it does, in order, with a pause between each stage so you can watch
 * the wheels / read the serial log and confirm each subsystem before
 * moving on:
 *
 *   1. Serial "hello" + I2C sanity check
 *   2. Side LEDs on/off (register 0x09 / 0x0A - value meaning unconfirmed,
 *      watch what actually happens and adjust)
 *   3. Each wheel individually, forward direction, low speed
 *   4. Each wheel individually, reverse direction, low speed
 *   5. All four wheels together (drive straight), then stop
 *   6. Ultrasonic distance readout, printed to serial once a second
 *
 * Wire trigPin/echoPin below to match your build (P15 trigger, P16 echo,
 * per the Keyestudio pin table).
 *
 * Adjust TEST_SPEED down further if the car lurches off your desk -
 * start low, these motors don't need much PWM to move on Mecanum wheels.
 */

#include "MicroBit.h"
#include "MecanumCarDriver.h"

MicroBit uBit;
MecanumCarDriver car(uBit); // default addr 0x60 (Python's 0x30 << 1)

static const uint8_t TEST_SPEED = 60; // 0-255, keep low for bench testing
static const int STAGE_PAUSE_MS = 2000;

void logStage(const char *msg)
{
    uBit.serial.printf("\r\n== %s ==\r\n", msg);
}

void testLeds()
{
    logStage("LEDs: left on");
    car.leftLed(1);
    uBit.sleep(1000);
    car.leftLed(0);

    logStage("LEDs: right on");
    car.rightLed(1);
    uBit.sleep(1000);
    car.rightLed(0);
}

void testWheelForward(void (MecanumCarDriver::*fn)(uint8_t, uint8_t), const char *name)
{
    uBit.serial.printf("Forward: %s\r\n", name);
    (car.*fn)(1, TEST_SPEED);
    uBit.sleep(1000);
    (car.*fn)(1, 0);
    uBit.sleep(300);
}

void testWheelReverse(void (MecanumCarDriver::*fn)(uint8_t, uint8_t), const char *name)
{
    uBit.serial.printf("Reverse: %s\r\n", name);
    (car.*fn)(0, TEST_SPEED);
    uBit.sleep(1000);
    (car.*fn)(0, 0);
    uBit.sleep(300);
}

void testAllWheelsForward()
{
    logStage("All wheels forward (drive straight)");
    car.motorUpperL(1, TEST_SPEED);
    car.motorLowerL(1, TEST_SPEED);
    car.motorUpperR(1, TEST_SPEED);
    car.motorLowerR(1, TEST_SPEED);
    uBit.sleep(1500);
    car.stopAll();
}

void testUltrasonic()
{
    logStage("Ultrasonic distance (10 readings)");

    codal::Pin &trig = uBit.io.P15;
    codal::Pin &echo = uBit.io.P16;

    for (int i = 0; i < 10; i++)
    {
        int cm = car.getDistanceCm(trig, echo);
        uBit.serial.printf("distance: %d cm\r\n", cm);
        uBit.sleep(1000);
    }
}

int main()
{
    uBit.init();

    logStage("Boot");
    uBit.serial.printf("MecanumCarDriver test harness starting\r\n");
    uBit.serial.printf("I2C addr: 0x%02x\r\n", 0x60);

    // Make sure everything is stopped before we touch anything else
    car.stopAll();
    uBit.sleep(500);

    testLeds();

    logStage("Individual wheels - forward");
    testWheelForward(&MecanumCarDriver::motorUpperL, "Upper Left");
    testWheelForward(&MecanumCarDriver::motorUpperR, "Upper Right");
    testWheelForward(&MecanumCarDriver::motorLowerL, "Lower Left");
    testWheelForward(&MecanumCarDriver::motorLowerR, "Lower Right");

    logStage("Individual wheels - reverse");
    testWheelReverse(&MecanumCarDriver::motorUpperL, "Upper Left");
    testWheelReverse(&MecanumCarDriver::motorUpperR, "Upper Right");
    testWheelReverse(&MecanumCarDriver::motorLowerL, "Lower Left");
    testWheelReverse(&MecanumCarDriver::motorLowerR, "Lower Right");

    testAllWheelsForward();
    uBit.sleep(STAGE_PAUSE_MS);

    testUltrasonic();

    logStage("Test sequence complete");
    car.stopAll();

    release_fiber();
}
