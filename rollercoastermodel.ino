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
uint8_t maxBrightness = 255;
uint8_t pwmFrequency = 120;
uint8_t numRegisters = 2;
//Usable HW-PWM pins: 3,5,6,9 & 10 (11 is taken by SPI  above)



// http://code.google.com/p/adaencoder/ (I have older version thus still using pinchangeint and not the new oopinchangeint)
#include <PinChangeInt.h> // necessary otherwise we get undefined reference errors.
#include <AdaEncoder.h>
// Used by the encoder
int8_t clicks;
char id;
encoder *thisEncoder;


// From http://playground.arduino.cc/code/bounce
#include <Bounce.h>
Bounce bouncer = Bounce(A4, 10);
byte bouncer_value=HIGH;


void setup ( )
{
    Serial.begin(57600);

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

int motor_pwm = 0; // We use the rotary to adjust this, must catch above 255 and below 0 conditions
uint16_t i = 0;
void loop ()
{
    i++;
//    lcd.setBacklight(HIGH);
    lcd.clear();
    lcd.print(F("Hello World "));
    lcd.print(i, DEC);

    thisEncoder = AdaEncoder::genie(&clicks, &id);
    if (thisEncoder != NULL)
    {
        thisEncoder = AdaEncoder::getFirstEncoder();
        // clicks has the movement value
        Serial.print(F("clicks="));
        Serial.println(clicks, DEC);
        motor_pwm += (clicks*5);
        if (motor_pwm < 0)
        {
            motor_pwm = 0;
        }
        if (motor_pwm > 255)
        {
            motor_pwm = 255;
        }
        analogWrite(9, motor_pwm);
        Serial.print(F("motor_pwm="));
        Serial.println(motor_pwm, DEC);
    }

    if (bouncer.update())
    {
        bouncer_value = bouncer.read();
        Serial.print(F("bouncer_value=0x"));
        Serial.println(bouncer_value, HEX);
        // Switch connects to ground, pin is pulled up internally
        if (bouncer_value == LOW)
        {
            // pass
        }
        else
        {
            // pass
        }
    }

    for (uint8_t ii = 0; ii < (numRegisters*8) ; ii++)
    {
        ShiftPWM.SetOne(ii, maxBrightness);
        delay(100);
    }
    ShiftPWM.OneByOneFast();
    ShiftPWM.PrintInterruptLoad();

    delay(100);
}
