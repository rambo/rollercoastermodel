// Note uses the *new* LiquidCrystal library https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_SR3W.h>

// ShiftPWM library, use my fork: https://github.com/rambo/ShiftPWM
#include <SPI.h>
const uint8_t ShiftPWM_latchPin=10;
const bool ShiftPWM_invertOutputs = false; 
const bool ShiftPWM_balanceLoad = false;
#define SHIFTPWM_USE_TIMER2
#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!

uint8_t maxBrightness = 255;
uint8_t pwmFrequency = 120;
uint8_t numRegisters = 1;


// constructor prototype parameter:
//  LiquidCrystal_SR3W lcd(DataPin, ClockPin, LatchPin, sr_enable, sr_rw, sr_rs, sr_d4, sr_d5, sr_d6, sr_d7, sr_bl, bl_pol);
LiquidCrystal_SR3W lcd(A2, A1, A0, 6, 7, 0 , 5, 4, 3, 2, 1, POSITIVE); 


void setup ( )
{
    Serial.begin(57600);
    lcd.begin ( 16, 2 );

    lcd.clear();
    lcd.print(F("Hello World"));
    

    pinMode(ShiftPWM_latchPin, OUTPUT);  
    SPI.setBitOrder(LSBFIRST);
    // SPI_CLOCK_DIV2 is only a tiny bit faster in sending out the last byte. 
    // SPI transfer and calculations overlap for the other bytes.
    SPI.setClockDivider(SPI_CLOCK_DIV4); 
    SPI.begin(); 
    ShiftPWM.SetAmountOfRegisters(numRegisters);
    ShiftPWM.Start(pwmFrequency,maxBrightness);
    
    ShiftPWM.SetAll(maxBrightness);
    ShiftPWM.PrintInterruptLoad();
    delay(1000);
    ShiftPWM.SetAll(0);

}

int i = 0;
void loop ()
{
    i++;
    lcd.setBacklight(HIGH);
    lcd.clear();
    lcd.print(F("Hello World "));
    lcd.print(i, DEC);

    /*
    for (uint8_t ii = 0; ii < (numRegisters*8) ; ii++)
    {
        ShiftPWM.SetOne(ii, maxBrightness);
        delay(100);
    }
    ShiftPWM.PrintInterruptLoad();
    ShiftPWM.OneByOneSlow();
    */

    //lcd.setBacklight(LOW);
    delay(100);
}
