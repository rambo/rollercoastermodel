#define __PROG_TYPES_COMPAT__
#include <avr/pgmspace.h>

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


// Config must be included *after* we know how many shift registers we have for ShiftPWM
#include <avr/eeprom.h>
#include "config.h"

// Updates shiftpwm internal state from the configration for all leds
void update_shiftpwm_all()
{
    for (uint8_t i = 0; i < sizeof(global_config.led_pwm_values); i++)
    {
        update_shiftpwm_single(i);
    }
}

// Updates shiftpwm internal state from the configration for given led
void update_shiftpwm_single(uint8_t i)
{
    int16_t tmp;
    tmp = global_config.led_pwm_values[i] + global_config.global_dimmer_adjust;
    if (tmp > 255)
    {
        tmp = 255;
    }
    if (tmp < 0)
    {
        tmp = 0;
    }
    ShiftPWM.SetOne(i, tmp);
    /*
    Serial.print(F("SetOne("));
    Serial.print(i, DEC);
    Serial.print(F(", "));
    Serial.print(tmp, DEC);
    Serial.println(F(")"));
    */
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
    dump_config();

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
    /**
     * Let the tasks take care of it
    pinMode(3, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
     */

    // Test the LEDs    
    ShiftPWM.SetAll(maxBrightness);
    delay(1000);
    ShiftPWM.SetAll(0);
    ShiftPWM.PrintInterruptLoad();
    
    // And set the initial config state
    update_shiftpwm_all();

}

void loop ()
{
    // Initialise the task list and scheduler. (uitask must be the last one, otherwise it robs priority from everything else)
    Task *tasks[] = { &motortimer, &uitask };
    TaskScheduler sched(tasks, NUM_TASKS(tasks));
    
    // Run the scheduler - never returns.
    sched.run();
}
