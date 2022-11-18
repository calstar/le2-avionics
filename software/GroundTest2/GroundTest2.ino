#include <Arduino.h>
#include <Wire.h>

#define PIN_I2C_FAST_SDA 16
#define PIN_I2C_FAST_SCL 17

class CoreBoard {
  private:
    TwoWire *I2C_fast;
  public:
	  CoreBoard();
	  int I2CFastWrite(uint8_t address, uint8_t buf_out[], uint8_t len);
};

/* Constructor for the CoreBoard class. */
CoreBoard::CoreBoard() {
  TwoWire bus_fast = TwoWire(0);
  bus_fast.begin(PIN_I2C_FAST_SDA, PIN_I2C_FAST_SCL, 400000);
  I2C_fast = &(bus_fast);
	Serial.println("Initialized core board!");
}

/* Write to the I2C_FAST bus. */
int CoreBoard::I2CFastWrite(uint8_t address, uint8_t buf_out[], uint8_t len) {

  I2C_fast->beginTransmission(address);

  Serial.println("Preparing to do I2C Write");
  I2C_fast->write(0x55);
  Serial.println("I2C Write sucessful");

  I2C_fast->endTransmission();
  return 0;

}

void setup() {
  Serial.begin(115200);
}

void loop() {
  CoreBoard core;
  uint8_t example_address = 0x70;
  uint8_t example_data[] = {0x41, 0x42, 0x43};
  while (true) {
    delay(1000);
    core.I2CFastWrite(example_address, example_data, 3);
  }
}
