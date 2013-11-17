// Note uses the *new* LiquidCrystal library https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_SR3W.h>
// LiquidCrystal_SR3W lcd(DataPin, ClockPin, LatchPin, sr_enable, sr_rw, sr_rs, sr_d4, sr_d5, sr_d6, sr_d7, sr_bl, bl_pol);
LiquidCrystal_SR3W lcd(A2, A1, A0, 6, 7, 0 , 5, 4, 3, 2, 1, POSITIVE); 


// ShiftPWM library, use my fork: https://github.com/rambo/ShiftPWM
#include <SPI.h>
const uint8_t ShiftPWM_latchPin = 8;
const bool ShiftPWM_invertOutputs = false; 
const bool ShiftPWM_balanceLoad = false;
#define SHIFTPWM_USE_TIMER2
#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!
// These are config variables ShiftPWM uses
const uint8_t maxBrightness = 255;
const uint8_t pwmFrequency = 120;
const  uint8_t numRegisters = 2;
//Usable HW-PWM pins: 3,5,6,9 & 10 (11 is taken by SPI  above)


// Use the AVR block functions directly (looping over arrays using the EEPROM Arduino library writing a byte at a time is just stupid), see also http://arduino.cc/playground/Code/EEPROMWriteAnything
#include <avr/eeprom.h>
typedef struct {
    uint16_t motor_stop_wait;
    uint16_t motor_run_wait;
    uint8_t motor_speed;
    uint16_t ui_idle_timeout;
    int8_t global_dimmer_adjust;
    uint8_t led_pwm_values[(numRegisters*8)];
    
    uint16_t magic_number;
} CoasterConfig;
CoasterConfig global_config;

void config_defaults()
{
    global_config.magic_number    = 0xdead;
    global_config.ui_idle_timeout = 10000;
    global_config.motor_stop_wait = 5000;
    global_config.motor_run_wait  = 2000;
    global_config.motor_speed     = 180; // 71% which should be close to the ~8.6V the original 555-box did.

    global_config.global_dimmer_adjust = 0;
    // I'm not 100% that is the correct way to point to a struct member but it does compile...
    memset(global_config.led_pwm_values, 255, sizeof(global_config.led_pwm_values));
}

void config_eeprom_read()
{
    eeprom_read_block((void*)&global_config, (void*)0, sizeof(global_config));
    if (global_config.magic_number != 0xdead)
    {
        // Insane value, restore defaults
        config_defaults();
    }
}

void config_eeprom_write()
{
    cli();
    eeprom_write_block((const void*)&global_config, (void*)0, sizeof(global_config));
    sei();
}



// http://code.google.com/p/adaencoder/ (I have older version thus still using pinchangeint and not the new oopinchangeint)
#include <PinChangeInt.h> // necessary otherwise we get undefined reference errors.
#include <AdaEncoder.h>
// Used by the encoder
int8_t clicks;
char encoder_id;
encoder *thisEncoder;


// From http://playground.arduino.cc/code/bounce
#include <Bounce.h>
Bounce bouncer = Bounce(A4, 10);
byte bouncer_value=HIGH;


// Get this library from http://bleaklow.com/files/2010/Task.tar.gz (and fix WProgram.h -> Arduino.h)
// and read http://bleaklow.com/2010/07/20/a_very_simple_arduino_task_manager.html for background and instructions
#include <Task.h>
#include <TaskScheduler.h>

#include "motortimertask.h"
MotorTimer motortimer(9);

#include "uitask.h"
UITask uitask;

void setup ( )
{
    Serial.begin(57600);
    config_eeprom_read();

    // Init LCD
    lcd.begin ( 16, 2 );
    lcd.clear();
    lcd.print(F("Hello World"));

    // Init encoder (and the pushbutton)
    AdaEncoder::addEncoder('a', A3, A5);
    pinMode(A4, INPUT_PULLUP);

    // Init ShiftPWM
    pinMode(ShiftPWM_latchPin, OUTPUT);  
    SPI.setBitOrder(LSBFIRST);
    // SPI_CLOCK_DIV2 is only a tiny bit faster in sending out the last byte. 
    // SPI transfer and calculations overlap for the other bytes.
    SPI.setClockDivider(SPI_CLOCK_DIV4); 
    SPI.begin(); 
    ShiftPWM.SetAmountOfRegisters(numRegisters);
    ShiftPWM.Start(pwmFrequency,maxBrightness);

    // Set the PWMs as outputs
    pinMode(3, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);

    // Test the LEDs    
    ShiftPWM.SetAll(maxBrightness);
    delay(1000);
    ShiftPWM.SetAll(0);
    ShiftPWM.PrintInterruptLoad();

    
}

void loop ()
{
    // Initialise the task list and scheduler. (uitask must be the last one, otherwise it robs priority from everything else)
    Task *tasks[] = { &motortimer, &uitask };
    TaskScheduler sched(tasks, NUM_TASKS(tasks));
    
    // Run the scheduler - never returns.
    sched.run();
}
