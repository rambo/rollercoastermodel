// Note uses the *new* LiquidCrystal library https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_SR.h>

// ShiftPWM library, use my fork: https://github.com/rambo/ShiftPWM
#include <SPI.h>
const uint8_t ShiftPWM_latchPin=10;
const bool ShiftPWM_invertOutputs = false; 
const bool ShiftPWM_balanceLoad = false;
#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!

uint8_t maxBrightness = 255;
uint8_t pwmFrequency = 120;
uint8_t numRegisters = 1;


// constructor prototype parameter:
//  LiquidCrystal_SR lcd(DataPin, ClockPin, EnablePin);
LiquidCrystal_SR lcd(2, 3, 4); 


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

}

int i = 0;
void loop ()
{
    i++;
    lcd.clear();
    lcd.print(F("Hello World "));
    lcd.print(i, DEC);

    for (uint8_t ii = 0; ii < (numRegisters*8) ; ii++)
    {
        if (ii > 0)
        {
            ShiftPWM.SetOne(ii-1, maxBrightness);
        }
        ShiftPWM.SetOne(ii, maxBrightness);
        delay(100);
    }
    ShiftPWM.PrintInterruptLoad();

    delay(100);
}
