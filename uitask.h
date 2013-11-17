#ifndef UITASK_H
#define UITASK_H

#include <Arduino.h>
#include <Task.h>

enum UIState { 
    SLEEPING,
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
    private:
        UIState mainstate;
        // Do we need to track substate ? probably...
};

UITask::UITask()
: Task()
{
}

bool UITask::canRun(uint32_t now)
{
    return true;
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
            // pass
            motortimer.freeze();
        }
        else
        {
            // pass
        }
    }

    thisEncoder = AdaEncoder::genie(&clicks, &encoder_id);
    if (thisEncoder != NULL)
    {
        thisEncoder = AdaEncoder::getFirstEncoder();
        // clicks has the movement value
        Serial.print(F("clicks="));
        Serial.println(clicks, DEC);
        
        global_config.global_dimmer_adjust += clicks;
        if (global_config.global_dimmer_adjust > 255)
        {
            global_config.global_dimmer_adjust = 255;
        }
        if (global_config.global_dimmer_adjust < 0 )
        {
            global_config.global_dimmer_adjust = 0;
        }
        Serial.print(F("global_dimmer_adjust = ")); Serial.println(global_config.global_dimmer_adjust, DEC);
        update_shiftpwm_all();
        
    }
}



#endif
