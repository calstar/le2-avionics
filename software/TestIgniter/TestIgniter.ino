#include <Arduino.h>
#include <Wire.h>

/* Library: https://www.arduino.cc/reference/en/libraries/tca9548a/ */
#include <TCA9548A.h>

/* Library: https://www.arduino.cc/reference/en/libraries/pcf8575/ */
#include <PCF8575.h>

#define PIN_I2C_SLOW_SDA 21
#define PIN_I2C_SLOW_SCL 22

#define I2C_MUX_ADDRESS 0x70
#define IGNITER_BOARD_IO_EXPANDER_ADDRESS 0x20

#define SLOT_IGNITER 0
#define NUM_RELAYS 6

class IgniterBoard {
  private:
    int slot;
    TwoWire *I2C_slow;
    PCF8575 *IO_expander;
  public:
    IgniterBoard(int slot_argument, TwoWire *I2C_slow_argument);
    int OpenRelay(int relay);
    int CloseRelay(int relay);
};

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
 * returns: 0 on success, -1 on error. */
int IgniterBoard::OpenRelay(int relay) {
  if (relay < 0 || relay >= NUM_RELAYS) {
    Serial.print("Invalid relay number selected: ");
    Serial.println(relay);
    return -1;
  } else {
    IO_expander->write(relay, HIGH);
    return 0;
  }
}

/* Sets one of the 6 relays on the igniter board to closed.
 * relay: the number of the relay to close. This number 
 * must be one of (0, 1, 2, 3, 4, 5).
 * returns: 0 on success, -1 on error. */
int IgniterBoard::CloseRelay(int relay) {
  if (relay < 0 || relay >= NUM_RELAYS) {
    Serial.print("Invalid relay number selected: ");
    Serial.println(relay);
    return -1;
  } else {
    IO_expander->write(relay, LOW);
    return 0;
  }
}

TwoWire *I2C_slow;
TCA9548A *I2C_mux;

IgniterBoard *igniter_board;

void setup() {
  
  Serial.begin(115200);
  Serial.println("Initializing core board...");

  /* Initialize the I2C_SLOW bus. */
  I2C_slow = new TwoWire(1);
  I2C_slow->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, 400000);

  /* Initialize the TCA9548A I2C mux. The ESP32 uses the I2C_SLOW
   * bus to communicate with the I2C mux. */
  I2C_mux = new TCA9548A(I2C_MUX_ADDRESS);
	I2C_mux->begin(*I2C_slow);
  I2C_mux->openAll();
  // BeginTransmission and then EndTransmission to check if connection works
  I2C_slow->beginTransmission(I2C_MUX_ADDRESS);
  if (I2C_slow->endTransmission() == 0) {
    Serial.println(" - Connected to TCA9548A I2C Mux");
  } else {
    Serial.println(" - Unable to connect to TCA9548A I2C Mux");
  }

  /* Initialize the peripheral boards. */
  igniter_board = new IgniterBoard(SLOT_IGNITER, I2C_slow);

}

void loop() {
  
  igniter_board->OpenRelay(0);
  delay(5000);
  igniter_board->CloseRelay(0);
  delay(5000);

}
