#ifndef GROUND_TEST_HEADER_FILE_INCLUDED
  #include "GroundTest.h"
#endif

/* Library: https://learn.adafruit.com/adafruit-max31865-rtd-pt100-amplifier/arduino-code */
#include <Adafruit_MAX31865.h>

#define RTD_BOARD_IO_EXPANDER_ADDRESS 0x22

#define NUM_RTD_AMPLIFIERS 8

/* The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000 */
#define RREF 430.0
/* The 'nominal' 0-degrees-C resistance of the sensor. 100.0 for PT100, 1000.0 for PT1000 */
#define RNOMINAL 100.0

class RTDBoard {
  private:
    int slot;
    TwoWire *I2C_slow;
    PCF8575 *IO_expander;
    Adafruit_MAX31865 *RTD_amplifiers[NUM_RTD_AMPLIFIERS];
  public:
    RTDBoard(int slot_argument, TwoWire *I2C_slow_argument);
    void PrintData(int rtd_number);
};