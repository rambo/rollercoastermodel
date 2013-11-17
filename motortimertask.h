#ifndef MOTORTIMERTASK_H
#define MOTORTIMERTASK_H

#include <Arduino.h>
#include <Task.h>

// Timed task to blink a LED.
class MotorTimer : public TimedTask
{
    public:
        // Create a new blinker for the specified pin and rate.
        MotorTimer(uint8_t _pin);
        virtual void run(uint32_t now);
    private:
        uint8_t pin;      // LED pin.
};

MotorTimer::MotorTimer(uint8_t _pin)
: TimedTask(millis()),
  pin(_pin)
{
    pinMode(pin, OUTPUT);     // Set pin for output.
}

void MotorTimer::run(uint32_t now)
{

    incRunTime(3000);
}


#endif
