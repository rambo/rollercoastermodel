#ifndef UITASK_H
#define UITASK_H
/**
 * This file needs lots of refactoring, but as stated I ran out of time
 */
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <Task.h>

enum UIState { 
    SLEEPING,
    WAKEUP,
    STATUS,
    ROOT,
    MOTORMENU,
    DIMMERMENU,
    LEDMENU,
    SAVEMENU,
    SUBMENU
};



// Menu strings
const char string_back[] PROGMEM = { "Back" };
const char string_motor_ctrl[] PROGMEM = "Motor control";
const char string_freeze[] PROGMEM = "Freeze";
const char string_unfreeze[] PROGMEM = "Unfreeze";
const char string_timer[] PROGMEM = "Timer";
const char string_ontime[] PROGMEM = "On time";
const char string_offtime[] PROGMEM = "Off time";
const char string_speed[] PROGMEM = "Speed";
const char string_edit[] PROGMEM = "Edit";
const char string_save[] PROGMEM = "Save settings";
const char string_confirm[] PROGMEM = "Confirm";
const char string_dimmer[] PROGMEM = "Dimmer control";
const char string_leds[] PROGMEM = "LED ch control";

// Root menu items
const char* const root_menu[] PROGMEM =
{
    string_back,
    string_motor_ctrl,
    string_dimmer,
    string_leds,
    string_save
};
const uint8_t root_menu_last_item = ARRAY_SIZE(root_menu)-1;

// Dimmer menu items
const char* const dimmer_menu[] PROGMEM =
{
    string_back,
    string_edit
};
const uint8_t dimmer_menu_last_item = ARRAY_SIZE(dimmer_menu)-1;

// Dimmer menu items
const char* const motor_menu[] PROGMEM =
{
    string_back,
    string_speed,
    string_ontime,
    string_offtime,
    string_freeze,
    string_unfreeze
};
const uint8_t motor_menu_last_item = ARRAY_SIZE(motor_menu)-1;

// Dimmer menu items
const char* const save_menu[] PROGMEM =
{
    string_back,
    string_confirm
};
const uint8_t save_menu_last_item = ARRAY_SIZE(save_menu)-1;


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
    - all on/off  ??
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
        int8_t current_menu_index;
        uint8_t menu_selected_stack[5]; // Keep track of the selected items
        UIState menu_state_stack[5]; // Keep track substates
        uint8_t current_menu_level;
        // Do we need to track substate ? probably...
        uint32_t last_activity;
        boolean redraw_needed;
};

UITask::UITask()
: Task()
{
    menu_state_stack[0] = STATUS;
    redraw_needed = true;
}

bool UITask::canRun(uint32_t now)
{
    return true;
}

void UITask::sleep()
{
    menu_state_stack[0] = SLEEPING;
    lcd.off();
}

void UITask::wakeup()
{
    lcd.on();
    menu_state_stack[0] = WAKEUP;
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
    if (menu_state_stack[0] == SLEEPING)
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
        /*
        Serial.print(F("bouncer_value=0x"));
        Serial.println(bouncer_value, HEX);
        */
        // Switch connects to ground, pin is pulled up internally
        if (bouncer_value == LOW)
        {
            reset_sleep_timer();
            button_clicked = true;
            input_seen = true;
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
        
        input_seen = true;

    }

    check_sleep_timer();
    // The monster state machine (there probably would be a more elegant library for handling this but I do not have the time to learn one right now)
    switch (menu_state_stack[0])
    {
        case SLEEPING:
            // Do nothing, the wakeup routines will take care of everything
            return;
        break;
        case WAKEUP:
            // Discard UI interaction used to wake us up and wake up fully
            menu_state_stack[0] = STATUS;
            menu_selected_stack[0] = 0;
            current_menu_level = 0;
            current_menu_index = 0;
            redraw_needed = true;
            return;
        break;
        
        case STATUS:
        {
            if (redraw_needed)
            {
                Serial.println(F("Status menu"));
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
                menu_state_stack[0] = ROOT;
                menu_selected_stack[0] = 0;
                current_menu_level = 0;
                current_menu_index = 0;
                redraw_needed = true;
                return;
            }
            break;
        }
        case ROOT:
        {
            if (   input_seen
                && !button_clicked)
            {
                current_menu_index += clicks;
            }
            if (current_menu_index < 0)
            {
                current_menu_index = root_menu_last_item;
            }
            if (current_menu_index > root_menu_last_item)
            {
                current_menu_index = 0;
            }
            if (input_seen)
            {
                redraw_needed = true;
            }
            if (button_clicked)
            {
                button_clicked = false;
                menu_selected_stack[0] = current_menu_index;
                Serial.print(F("Selected root menu index ")); Serial.println(current_menu_index, DEC);
                switch(current_menu_index)
                {
                    case 0: // Back
                        Serial.println(F("Back from root"));
                        current_menu_level = 0;
                        menu_state_stack[0] = STATUS;
                        return;
                    break;
                    case 1: // Motor
                        Serial.println(F("To dimmer from motor"));
                        current_menu_index = 1;
                        current_menu_level = 1;
                        menu_state_stack[0] = MOTORMENU;
                        return;
                    break;
                    case 2: // Dimmer
                        Serial.println(F("To dimmer from root"));
                        current_menu_index = 1;
                        current_menu_level = 1;
                        menu_state_stack[0] = DIMMERMENU;
                        // Unimplemented
                        return;
                    break;
                    case 3: // LEDS
                        // Unimplemented
                        current_menu_level = 0;
                        menu_state_stack[0] = STATUS;
                        return;
                    break;
                    case 4: // Save
                        Serial.println(F("To save from root"));
                        current_menu_level = 1;
                        menu_state_stack[0] = SAVEMENU;
                        return;
                    break;
                }
            }
            if (redraw_needed)
            {
                redraw_needed = false;
                Serial.println(F("Root menu"));
                Serial.print(F("current_menu_index=")); Serial.println(current_menu_index, DEC);
                Serial.print(F("current_menu_item=")); Serial.println(FSA(root_menu[current_menu_index]));

                lcd.clear();
                lcd.print(FSA(root_menu[current_menu_index]));
                lcd.setCursor(0, 1); // cols, rows
                if (current_menu_index < root_menu_last_item)
                {
                    lcd.print(FSA(root_menu[(current_menu_index+1)]));
                }
                else
                {
                    lcd.print(FSA(root_menu[0]));
                }
                lcd.setCursor(0, 0); // cols, rows
                lcd.blink();
                
            }
            break;
        }
        case DIMMERMENU:
        {
            switch (current_menu_level)
            {
                case 1: // Dimmer menu
                {
                    if (   input_seen
                        && !button_clicked)
                    {
                        current_menu_index += clicks;
                    }
                    if (current_menu_index < 0)
                    {
                        current_menu_index = dimmer_menu_last_item;
                    }
                    if (current_menu_index > dimmer_menu_last_item)
                    {
                        current_menu_index = 0;
                    }
                    break;
                }
                case 2: // Edit mode
                {
                    if (   input_seen
                        && !button_clicked)
                    {
                        global_config.global_dimmer_adjust += (clicks*5);
                    }
                    if (global_config.global_dimmer_adjust > 255)
                    {
                        global_config.global_dimmer_adjust = 255;
                    }
                    if (global_config.global_dimmer_adjust < -255 )
                    {
                        global_config.global_dimmer_adjust = -255;
                    }
                    update_shiftpwm_all();
                    break;
                }
            }
            if (input_seen)
            {
                redraw_needed = true;
            }
            if (button_clicked)
            {
                button_clicked = false;
                switch (current_menu_level)
                {
                    case 1:
                    {
                        menu_selected_stack[1] = current_menu_index;
                        Serial.print(F("Selected dimmer menu index ")); Serial.println(current_menu_index, DEC);
                        switch(current_menu_index)
                        {
                            case 0: // Back
                                Serial.println(F("Back from dimmer"));
                                current_menu_level = 0;
                                current_menu_index = menu_selected_stack[0];
                                menu_state_stack[0] = ROOT;
                                return;
                            break;
                            case 1: // edit
                                Serial.println(F("Entering dimmer edit mode"));
                                current_menu_level = 2;
                                return;
                            break;
                        }
                        break;
                    }
                    case 2:
                    {
                        Serial.println(F("Back from dimmer edit"));
                        current_menu_level = 1;
                        current_menu_index = 0;
                        return;
                        break;
                    }
                }
            }
            if (redraw_needed)
            {
                Serial.print(F("current_menu_level=")); Serial.println(current_menu_level, DEC);
                redraw_needed = false;
                switch (current_menu_level)
                {
                    case 1:
                    {
                        Serial.println(F("Dimmer menu"));
                        Serial.print(F("current_menu_index=")); Serial.println(current_menu_index, DEC);
                        Serial.print(F("current_menu_item=")); Serial.println(FSA(dimmer_menu[current_menu_index]));
                        lcd.clear();
                        lcd.print(FSA(dimmer_menu[current_menu_index]));
                        lcd.setCursor(0, 1); // cols, rows
                        if (current_menu_index < dimmer_menu_last_item)
                        {
                            lcd.print(FSA(dimmer_menu[(current_menu_index+1)]));
                        }
                        else
                        {
                            lcd.print(FSA(dimmer_menu[0]));
                        }
                        lcd.setCursor(0, 0); // cols, rows
                        lcd.blink();
                        break;
                    }
                    case 2:
                    {
                        lcd.clear();
                        lcd.print(F("Edit dimmer"));
                        Serial.println(F("Edit dimmer"));
                        Serial.print(F("global_dimmer_adjust = ")); Serial.println(global_config.global_dimmer_adjust, DEC);
                        lcd.setCursor(0, 1); // cols, rows
                        lcd.print(global_config.global_dimmer_adjust, DEC);
                        lcd.setCursor(0, 1); // cols, rows
                        lcd.blink();
                        break;
                    }
                }
            }
            break;
        }
        case MOTORMENU:
        {
            switch (current_menu_level)
            {
                case 1: // Motor menu
                {
                    if (   input_seen
                        && !button_clicked)
                    {
                        current_menu_index += clicks;
                    }
                    if (current_menu_index < 0)
                    {
                        current_menu_index = motor_menu_last_item;
                    }
                    if (current_menu_index > motor_menu_last_item)
                    {
                        current_menu_index = 0;
                    }
                    break;
                }
                case 2:
                {
                    // One of the actions
                    switch (menu_selected_stack[1])
                    {
                        case 1: // speed
                        {
                            if (   input_seen
                                && !button_clicked)
                            {
                                global_config.motor_speed += (clicks*5);
                            }
                            if (global_config.motor_speed > 255)
                            {
                                global_config.motor_speed = 255;
                            }
                            if (global_config.motor_speed < 0)
                            {
                                global_config.motor_speed = 0;
                            }
                            break;
                        }
                        case 2: // on time
                        {
                            if (   input_seen
                                && !button_clicked)
                            {
                                global_config.motor_run_wait += (clicks*50);
                            }
                            if (global_config.motor_run_wait > 60000)
                            {
                                global_config.motor_run_wait = 60000;
                            }
                            if (global_config.motor_run_wait < 100 )
                            {
                                global_config.motor_run_wait = 100;
                            }
                            break;
                        }
                        case 3: // off time
                        {
                            if (   input_seen
                                && !button_clicked)
                            {
                                global_config.motor_stop_wait += (clicks*50);
                            }
                            if (global_config.motor_stop_wait > 60000)
                            {
                                global_config.motor_stop_wait = 60000;
                            }
                            if (global_config.motor_stop_wait < 100 )
                            {
                                global_config.motor_stop_wait = 100;
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            if (input_seen)
            {
                redraw_needed = true;
            }
            if (button_clicked)
            {
                button_clicked = false;
                switch (current_menu_level)
                {
                    case 1: // Motor menu
                    {
                        menu_selected_stack[1] = current_menu_index;
                        Serial.print(F("Selected motor menu index ")); Serial.println(current_menu_index, DEC);
                        switch(current_menu_index)
                        {
                            case 0: // Back
                                Serial.println(F("Back from motor"));
                                current_menu_level = 0;
                                current_menu_index = menu_selected_stack[0];
                                menu_state_stack[0] = ROOT;
                                return;
                            break;
                            case 1: // on time
                                Serial.println(F("Entering speed"));
                                current_menu_level = 2;
                                return;
                            break;
                            case 2: // on time
                                Serial.println(F("Entering on-time"));
                                current_menu_level = 2;
                                return;
                            break;
                            case 3: // off time
                                Serial.println(F("Entering off-time"));
                                current_menu_level = 2;
                                return;
                            break;
                            case 4: // freeze
                                Serial.println(F("freeze!"));
                                motortimer.freeze();
                                // And drop directly back to status (a timed message display would be cool, no time for that now)
                                current_menu_level = 0;
                                menu_state_stack[0] = STATUS;
                                return;
                            break;
                            case 5: // unfreeze
                                Serial.println(F("unfreeze"));
                                motortimer.thaw();
                                // And drop directly back to status (a timed message display would be cool, no time for that now)
                                current_menu_level = 0;
                                menu_state_stack[0] = STATUS;
                                return;
                            break;
                        }
                        break;
                    }
                    case 2: // One of the submenus
                    {
                        // Just drop back
                        Serial.println(F("Dropping one level back"));
                        dump_config();
                        current_menu_level = 1;
                        return;
                        break;
                    }
                }
            }
            if (redraw_needed)
            {
                Serial.print(F("current_menu_level=")); Serial.println(current_menu_level, DEC);
                redraw_needed = false;
                switch (current_menu_level)
                {
                    case 1:
                    {
                        Serial.println(F("Motor menu"));
                        Serial.print(F("current_menu_index=")); Serial.println(current_menu_index, DEC);
                        Serial.print(F("current_menu_item=")); Serial.println(FSA(motor_menu[current_menu_index]));
                        lcd.clear();
                        lcd.print(FSA(motor_menu[current_menu_index]));
                        lcd.setCursor(0, 1); // cols, rows
                        if (current_menu_index < motor_menu_last_item)
                        {
                            lcd.print(FSA(motor_menu[(current_menu_index+1)]));
                        }
                        else
                        {
                            lcd.print(FSA(motor_menu[0]));
                        }
                        lcd.setCursor(0, 0); // cols, rows
                        lcd.blink();
                        break;
                    }
                    case 2:
                    {
                        // One of the actions
                        switch (menu_selected_stack[1])
                        {
                            case 1: // speed
                            {
                                lcd.clear();
                                lcd.print(F("Speed (0-255)"));
                                Serial.println(F("Speed (0-255)"));
                                Serial.print(F("motor_speed = ")); Serial.println(global_config.motor_speed, DEC);
                                lcd.setCursor(0, 1); // cols, rows
                                lcd.print(global_config.motor_speed, DEC);
                                lcd.setCursor(0, 1); // cols, rows
                                lcd.blink();
                                break;
                            }
                            case 2: // on time
                            {
                                lcd.clear();
                                lcd.print(F("Run time (ms)"));
                                Serial.println(F("Run time (ms)"));
                                Serial.print(F("motor_run_wait = ")); Serial.println(global_config.motor_run_wait, DEC);
                                lcd.setCursor(0, 1); // cols, rows
                                lcd.print(global_config.motor_run_wait, DEC);
                                lcd.setCursor(0, 1); // cols, rows
                                lcd.blink();
                                break;
                            }
                            case 3: // off time
                            {
                                lcd.clear();
                                lcd.print(F("Wait time (ms)"));
                                Serial.println(F("Wait time (ms)"));
                                Serial.print(F("motor_stop_wait = ")); Serial.println(global_config.motor_stop_wait, DEC);
                                lcd.setCursor(0, 1); // cols, rows
                                lcd.print(global_config.motor_stop_wait, DEC);
                                lcd.setCursor(0, 1); // cols, rows
                                lcd.blink();
                                break;
                            }
                        }
                        break;
                    }
                }
             }
            
            break;
        }
        case SAVEMENU:
        {
            switch (current_menu_level)
            {
                case 1: // Save menu
                {
                    if (   input_seen
                        && !button_clicked)
                    {
                        current_menu_index += clicks;
                    }
                    if (current_menu_index < 0)
                    {
                        current_menu_index = dimmer_menu_last_item;
                    }
                    if (current_menu_index > dimmer_menu_last_item)
                    {
                        current_menu_index = 0;
                    }
                    break;
                }
            }
            if (input_seen)
            {
                redraw_needed = true;
            }
            if (button_clicked)
            {
                button_clicked = false;
                switch (current_menu_level)
                {
                    case 1: // Save menu
                    {
                        menu_selected_stack[1] = current_menu_index;
                        Serial.print(F("Selected motor menu index ")); Serial.println(current_menu_index, DEC);
                        switch(current_menu_index)
                        {
                            case 0: // Back
                                Serial.println(F("Back from motor"));
                                current_menu_level = 0;
                                current_menu_index = menu_selected_stack[0];
                                menu_state_stack[0] = ROOT;
                                return;
                            break;
                            case 1: // confirm
                                Serial.println(F("Confirmed"));
                                config_eeprom_write();
                                current_menu_level = 0;
                                current_menu_index = menu_selected_stack[0];
                                menu_state_stack[0] = ROOT;
                                return;
                            break;
                        }
                    }
                }
            }
            if (redraw_needed)
            {
                Serial.print(F("current_menu_level=")); Serial.println(current_menu_level, DEC);
                redraw_needed = false;
                switch (current_menu_level)
                {
                    case 1:
                    {
                        Serial.println(F("Save menu"));
                        Serial.print(F("current_menu_index=")); Serial.println(current_menu_index, DEC);
                        Serial.print(F("current_menu_item=")); Serial.println(FSA(save_menu[current_menu_index]));
                        lcd.clear();
                        lcd.print(FSA(save_menu[current_menu_index]));
                        lcd.setCursor(0, 1); // cols, rows
                        if (current_menu_index < save_menu_last_item)
                        {
                            lcd.print(FSA(save_menu[(current_menu_index+1)]));
                        }
                        else
                        {
                            lcd.print(FSA(save_menu[0]));
                        }
                        lcd.setCursor(0, 0); // cols, rows
                        lcd.blink();
                        break;
                    }
                }
            }
            break;
        }
    }
}



#endif
