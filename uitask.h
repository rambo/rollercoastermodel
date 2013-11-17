#ifndef UITASK_H
#define UITASK_H

#include <Arduino.h>
#include <Task.h>

enum UIState { 
    SLEEPING,
    WAKEUP,
    STATUS,
    ROOT,
    MOTORMENU,
    LEDMENU
};



#define __PROG_TYPES_COMPAT__
#include <avr/pgmspace.h>
const prog_char string_back[] PROGMEM = "Back";
const prog_char string_motor_ctrl[] PROGMEM = "Motor control";



/**
 * Menu structure plan

  - Motor
    - back
    - freeze/unfreeze
      - confirm
    - Timer
      - back
      - On time
         - back
         - edit
      - off time
         - back
         - edit
    - speed
      - back
      - edit
  - Global dimmer
    - back
    - edit
  - LEDS
    - index for each
       - back
       - edit
  - Save settings
    - back
    - Confirm
  
 
 
 
 */

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
        UIState current_state;
        UIState prevstate;
        int8_t current_menu_index;
        // Do we need to track substate ? probably...
        uint32_t last_activity;
        boolean redraw_needed;
};

UITask::UITask()
: Task()
{
    current_state = STATUS;
    redraw_needed = true;
}

bool UITask::canRun(uint32_t now)
{
    return true;
}

void UITask::sleep()
{
    current_state = SLEEPING;
    lcd.off();
}

void UITask::wakeup()
{
    lcd.on();
    current_state = WAKEUP;
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
    if (current_state == SLEEPING)
    {
        wakeup();
    }
}

void UITask::run(uint32_t now)
{
    boolean input_seen = false;
    boolean button_clicked = false;
    if (bouncer.update())
    {
        bouncer_value = bouncer.read();
        Serial.print(F("bouncer_value=0x"));
        Serial.println(bouncer_value, HEX);
        // Switch connects to ground, pin is pulled up internally
        if (bouncer_value == LOW)
        {
            reset_sleep_timer();
            button_clicked = true;
            input_seen = true;
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
        
        current_menu_index += clicks;
        input_seen = true;

    }

    check_sleep_timer();
    // The monster state machine
    switch (current_state)
    {
        case SLEEPING:
            // Do nothing, the wakeup routines will take care of everything
        break;
        case WAKEUP:
            // Discard UI interaction used to wake us up and wake up fully
            current_state = STATUS;
            redraw_needed = true;
        break;
        
        case STATUS:
        {
            if (redraw_needed)
            {
                lcd.clear();
                lcd.setCursor(2, 0); // cols, rows
                lcd.print(F("Coaster Ctrl"));
                lcd.setCursor(0, 1); // cols, rows
                lcd.print(F("Mot "));
                if (motortimer.is_frozen())
                {
                    lcd.print(F("OFF"));
                }
                else
                {
                    lcd.print(F("ON"));
                }
                lcd.setCursor(8, 1); // cols, rows
                lcd.print(F("Dim "));
                lcd.print(global_config.global_dimmer_adjust, DEC);
                
                redraw_needed = false;
            }
            if (input_seen)
            {
                current_state = ROOT;
                redraw_needed = true;
            }
            break;
        }
        case ROOT:
        {
            if (redraw_needed)
            {
                lcd.clear();
                redraw_needed = false;
            }
            break;
        }
    }
}



#endif
