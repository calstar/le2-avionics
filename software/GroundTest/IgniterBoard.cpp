#include "IgniterBoard.h"

/* Cosntructor for the IgniterBoard class. */
IgniterBoard::IgniterBoard(int slot_argument, TwoWire *I2C_slow_argument) {

  slot = slot_argument;
  I2C_slow = I2C_slow_argument;

  // Initialize the IO Expander
  uint16_t io_expander_initial_value = 0x0000;
  IO_expander = new PCF8575(IGNITER_BOARD_IO_EXPANDER_ADDRESS, I2C_slow);
  IO_expander->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, io_expander_initial_value);

}

/* Sets one of the 6 relays on the igniter board to open.
 * relay: the number of the relay to open. This number 
 * must be one of (0, 1, 2, 3, 4, 5).
 * returns: void */
void IgniterBoard::OpenRelay(int relay) {
  IO_expander->write(relay, HIGH);
}

/* Sets one of the 6 relays on the igniter board to closed.
 * relay: the number of the relay to close. This number 
 * must be one of (0, 1, 2, 3, 4, 5).
 * returns: void */
void IgniterBoard::CloseRelay(int relay) {
  IO_expander->write(relay, LOW);
}



