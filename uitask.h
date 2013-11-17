#ifndef UITASK_H
#define UITASK_H

#include <Arduino.h>
#include <Task.h>

enum UIState { 
    SLEEPING,
    WAKEUP,
    ROOT,
    MOTORMENU,
    LEDMENU
};

class UITask : public Task
{
    public:
        UITask();
        virtual void run(uint32_t now);
        virtual bool canRun(uint32_t now);
        void reset_sleep_timer();
        void sleep();
        void wakeup();
    private:
        void check_sleep_timer();
        UIState mainstate;
        // Do we need to track substate ? probably...
        uint32_t last_activity;
};

UITask::UITask()
: Task()
{
}

bool UITask::canRun(uint32_t now)
{
    return true;
}

void UITask::sleep()
{
    mainstate = SLEEPING;
    lcd.off();
}

void UITask::wakeup()
{
    lcd.on();
    mainstate = WAKEUP;
}

void UITask::check_sleep_timer()
{
    if (millis() - last_activity > global_config.ui_idle_timeout)
    {
        sleep();
    }
}

void UITask::reset_sleep_timer()
{
    last_activity = millis();
    if (mainstate == SLEEPING)
    {
        wakeup();
    }
}

void UITask::run(uint32_t now)
{
    if (bouncer.update())
    {
        bouncer_value = bouncer.read();
        Serial.print(F("bouncer_value=0x"));
        Serial.println(bouncer_value, HEX);
        // Switch connects to ground, pin is pulled up internally
        if (bouncer_value == LOW)
        {
            reset_sleep_timer();
            // pass
            motortimer.freeze();
        }
        else
        {
            reset_sleep_timer();
            // pass
        }
    }

    thisEncoder = AdaEncoder::genie(&clicks, &encoder_id);
    if (thisEncoder != NULL)
    {
        reset_sleep_timer();
        thisEncoder = AdaEncoder::getFirstEncoder();
        
        // clicks has the movement value
        Serial.print(F("clicks="));
        Serial.println(clicks, DEC);
        
        global_config.global_dimmer_adjust += clicks;
        if (global_config.global_dimmer_adjust > 255)
        {
            global_config.global_dimmer_adjust = 255;
        }
        if (global_config.global_dimmer_adjust < -255 )
        {
            global_config.global_dimmer_adjust = -255;
        }
        Serial.print(F("global_dimmer_adjust = ")); Serial.println(global_config.global_dimmer_adjust, DEC);
        update_shiftpwm_all();
        
    }

    check_sleep_timer();
    // The monster state machine
    switch (mainstate)
    {
        case SLEEPING:
            // Do nothing, the wakeup routines will take care of everything
        break;
        case WAKEUP:
            // Discard UI interaction used to wake us up and wake up fully
            mainstate = ROOT;
        break;
    }
}



#endif
