#ifndef GROUND_TEST_HEADER_FILE_INCLUDED
  #include "GroundTest.h"
#endif

#define IGNITER_BOARD_IO_EXPANDER_ADDRESS 0x21

class IgniterBoard {
  private:
    int slot;
    TwoWire *I2C_slow;
    PCF8575 *IO_expander;
  public:
    IgniterBoard(int slot_argument, TwoWire *I2C_slow_argument);
    void OpenRelay(int relay);
    void CloseRelay(int relay);
};