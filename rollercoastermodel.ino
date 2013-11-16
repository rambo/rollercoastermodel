// Note uses the *new* LiquidCrystal library https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_SR.h>

// constructor prototype parameter:
//  LiquidCrystal_SR lcd(DataPin, ClockPin, EnablePin);
LiquidCrystal_SR lcd(2, 3, 4); 
void setup ( )
{
    lcd.begin ( 16, 2 );

    lcd.clear();
    lcd.print(F("Hello World"));
    
   // your code here...
}

int i = 0;
void loop ()
{
    i++;
    lcd.clear();
    lcd.print(F("Hello World "));
    lcd.print(i, DEC);
    delay(100);
}
