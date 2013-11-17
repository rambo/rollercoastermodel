#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>

// Use the AVR block functions directly (looping over arrays using the EEPROM Arduino library writing a byte at a time is just stupid), see also http://arduino.cc/playground/Code/EEPROMWriteAnything
#include <avr/eeprom.h>
typedef struct {
    uint16_t motor_stop_wait;
    uint16_t motor_run_wait;
    uint8_t  motor_speed;
    uint16_t ui_idle_timeout;
    int16_t  global_dimmer_adjust; // We need to catch going below 0 without overflow (ditto for going over 255)
    uint8_t  led_pwm_values[(numRegisters*8)];
    
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

    global_config.global_dimmer_adjust = -200;
    // I'm not 100% that is the correct way to point to a struct member but it does compile...
    memset(global_config.led_pwm_values, 255, sizeof(global_config.led_pwm_values));
}

void dump_config()
{
    Serial.println(F("global_config:"));
    Serial.print(F("  magic_number = 0x")); Serial.println(global_config.magic_number, HEX);
    Serial.print(F("  ui_idle_timeout = ")); Serial.println(global_config.ui_idle_timeout, DEC);
    Serial.print(F("  motor_stop_wait = ")); Serial.println(global_config.motor_stop_wait, DEC);
    Serial.print(F("  motor_run_wait = ")); Serial.println(global_config.motor_run_wait, DEC);
    Serial.print(F("  motor_speed = ")); Serial.println(global_config.motor_speed, DEC);
    Serial.print(F("  global_dimmer_adjust = ")); Serial.println(global_config.global_dimmer_adjust, DEC);
    for (uint8_t i = 0; i < sizeof(global_config.led_pwm_values); i++)
    {
        Serial.print(F("  led_pwm_values["));
        Serial.print(i, DEC);
        Serial.print(F("] = "));
        Serial.println(global_config.led_pwm_values[i], DEC);
    }
}

void config_eeprom_read()
{
    eeprom_read_block((void*)&global_config, (void*)0, sizeof(global_config));
    if (global_config.magic_number != 0xdead)
    {
        // Insane value, restore defaults
        Serial.println(F("EEPROM config magic number was insane, loading defaults"));
        config_defaults();
    }
}

void config_eeprom_write()
{
    cli();
    eeprom_write_block((const void*)&global_config, (void*)0, sizeof(global_config));
    sei();
}


#endif
