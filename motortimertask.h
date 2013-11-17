#ifndef MOTORTIMERTASK_H
#define MOTORTIMERTASK_H

#include <Arduino.h>
#include <Task.h>

enum MotorState { 
    UNINITIALIZED,
    RUNNING,
    STOPPED,
    RUN_COUNTDOWN, // If I later implement the UI support for countdown display
    STOP_COUNTDOWN,
    FROZEN
};


// Timed task to blink a LED.
class MotorTimer : public TimedTask
{
    public:
        // Create a new blinker for the specified pin and rate.
        MotorTimer(uint8_t _pin);
        virtual void run(uint32_t now);
        void freeze();
        void thaw();
        boolean is_frozen();
        
    private:
        uint8_t pin;      // LED pin.
        MotorState state;
};

MotorTimer::MotorTimer(uint8_t _pin)
: TimedTask(millis()),
  pin(_pin)
{
    state = UNINITIALIZED;
    pinMode(pin, OUTPUT);     // Set pin for output.
}

void MotorTimer::run(uint32_t now)
{
    switch (state)
    {
        case UNINITIALIZED:
        {
            state = STOPPED;
            return;
            break;
        }
        case STOPPED:
        {
            incRunTime(global_config.motor_run_wait);
            state = RUNNING;
            analogWrite(pin, global_config.motor_speed);
            return;
            break;
        }
        case RUNNING:
        {
            incRunTime(global_config.motor_stop_wait);
            state = STOPPED;
            analogWrite(pin, 0);
            return;
            break;
        }
        case FROZEN:
        {
            // TODO: Overload the canRun method instead of using the timer
            incRunTime(global_config.motor_stop_wait);
            return;
            break;
        }
    }
}

boolean MotorTimer::is_frozen()
{
    return (state == FROZEN);
}


void MotorTimer::freeze()
{
    analogWrite(pin, 0);
    state = FROZEN;
}

void MotorTimer::thaw()
{
    if (state != FROZEN)
    {
        return;
    }
    state = UNINITIALIZED;
}


#endif
