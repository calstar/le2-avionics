#include "GroundTest.h"

// Maybe edit this
#define IGNITER_BOARD_IO_EXPANDER_ADDRESS 0x21

class IgniterBoard {
  private:
    int slot;
    TwoWire *I2C_slow;
    PCF8575 *IO_expander;
    TCA9548A *I2C_mux;
  public:
    IgniterBoard(int slot_argument, TwoWire *I2C_slow_argument, TCA9548A *I2C_mux_argument);
    void OpenRelay(int relay);
    void CloseRelay(int relay);
};