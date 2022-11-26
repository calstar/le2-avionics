#ifndef GROUND_TEST_HEADER_FILE_INCLUDED
  #include "GroundTest.h"
#endif

/* Library: https://learn.adafruit.com/adafruit-max31856-thermocouple-amplifier */
#include <Adafruit_MAX31856.h>

#define THERMOCOUPLE_BOARD_IO_EXPANDER_ADDRESS 0x24

#define NUM_THERMOCOUPLE_AMPLIFIERS 5

// The pin numbers (0 to 15) on the IO Expander that connect to each thermocouple amplifier.
#define THERMOCOUPLE_0_CS_PIN 8
#define THERMOCOUPLE_1_CS_PIN 10
#define THERMOCOUPLE_2_CS_PIN 12
#define THERMOCOUPLE_3_CS_PIN 14
#define THERMOCOUPLE_4_CS_PIN 7

class ThermocoupleBoard {
  private:
    int slot;
    TwoWire *I2C_slow;
    PCF8575 *IO_expander;
    Adafruit_MAX31856 *thermocouple_amplifiers[NUM_THERMOCOUPLE_AMPLIFIERS];
  public:
    ThermocoupleBoard(int slot_argument, TwoWire *I2C_slow_argument);
    void PrintData(int thermocouple_number);
};