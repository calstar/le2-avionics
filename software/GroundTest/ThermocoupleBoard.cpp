#include "ThermocoupleBoard.h"

/* Cosntructor for the ThermocoupleBoard class. */
ThermocoupleBoard::ThermocoupleBoard(int slot_argument, TwoWire *I2C_slow_argument) {

  slot = slot_argument;
  I2C_slow = I2C_slow_argument;

  // Initialize the IO Expander
  uint16_t io_expander_initial_value = 0xFFFF;
  IO_expander = new PCF8575(THERMOCOUPLE_BOARD_IO_EXPANDER_ADDRESS, I2C_slow);
  IO_expander->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, io_expander_initial_value);

  // Initialize each thermocouple amplifier, and add each one to the array thermocouple_amplifiers
  for (int thermocouple_number = 0; thermocouple_number < NUM_THERMOCOUPLE_AMPLIFIERS; thermocouple_number++) {

    // Decide which IO Expander pin to use as the thermocouple amplifier CS pin
    int io_expander_pin_number;
    switch (thermocouple_number) {
      case 0 : io_expander_pin_number = THERMOCOUPLE_0_CS_PIN;
      case 1 : io_expander_pin_number = THERMOCOUPLE_1_CS_PIN;
      case 2 : io_expander_pin_number = THERMOCOUPLE_2_CS_PIN;
      case 3 : io_expander_pin_number = THERMOCOUPLE_3_CS_PIN;
      case 4 : io_expander_pin_number = THERMOCOUPLE_4_CS_PIN;
    }

    IO_expander->write(io_expander_pin_number, LOW);

    Adafruit_MAX31856 *thermocouple_amplifier = new Adafruit_MAX31856(PIN_SPI_CS_0, PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK);
    if (!thermocouple_amplifier->begin()) {
      Serial.print("Could not initialize thermocouple #");
      Serial.println(thermocouple_number);
    }
    thermocouple_amplifier->setThermocoupleType(MAX31856_TCTYPE_K);
    thermocouple_amplifiers[thermocouple_number] = thermocouple_amplifier;

    IO_expander->write(io_expander_pin_number, HIGH);

  }

}

/* Prints out temperature data from the Thermocouple.
 * Most of this is copied directly from the Adafruit example
 * located at File->Examples->Adafruit_MAX31856->max31856. 
 * thermocouple_number: the index of the thermocouple that you
 * want to read from, must be one of (0, 1, 2, 3, 4). 
 * returns: void */
void ThermocoupleBoard::PrintData(int thermocouple_number) {

  if (thermocouple_number < 0 || thermocouple_number >= NUM_THERMOCOUPLE_AMPLIFIERS) {
    Serial.println("Error: Thermocouple number out of range");
    return;
  }

  // Decide which IO Expander pin to use as the thermocouple amplifier CS pin
  int io_expander_pin_number;
  switch (thermocouple_number) {
    case 0 : io_expander_pin_number = THERMOCOUPLE_0_CS_PIN;
    case 1 : io_expander_pin_number = THERMOCOUPLE_1_CS_PIN;
    case 2 : io_expander_pin_number = THERMOCOUPLE_2_CS_PIN;
    case 3 : io_expander_pin_number = THERMOCOUPLE_3_CS_PIN;
    case 4 : io_expander_pin_number = THERMOCOUPLE_4_CS_PIN;
  }

  Adafruit_MAX31856 *thermocouple_amplifier = thermocouple_amplifiers[thermocouple_number];

  // Toggle CS pin to low and begin SPI communication
  IO_expander->write(io_expander_pin_number, LOW);

  Serial.print("Data for Thermocouple #");
  Serial.print(thermocouple_number);
  Serial.println(":");

  Serial.print(" - Cold junction temperature = ");
  Serial.println(thermocouple_amplifier->readCJTemperature());
  Serial.print(" - Thermocouple temperature = ");
  Serial.println(thermocouple_amplifier->readThermocoupleTemperature());

  Serial.print(" - Error status: ");
  uint8_t fault = thermocouple_amplifier->readFault();
  if (fault) {
    if (fault & MAX31856_FAULT_CJRANGE) Serial.println("Cold Junction Range Fault");
    if (fault & MAX31856_FAULT_TCRANGE) Serial.println("Thermocouple Range Fault");
    if (fault & MAX31856_FAULT_CJHIGH)  Serial.println("Cold Junction High Fault");
    if (fault & MAX31856_FAULT_CJLOW)   Serial.println("Cold Junction Low Fault");
    if (fault & MAX31856_FAULT_TCHIGH)  Serial.println("Thermocouple High Fault");
    if (fault & MAX31856_FAULT_TCLOW)   Serial.println("Thermocouple Low Fault");
    if (fault & MAX31856_FAULT_OVUV)    Serial.println("Over/Under Voltage Fault");
    if (fault & MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
  } else {
    Serial.println("No fault detected");
  }

  // Toggle CS pin to high and end SPI communication
  IO_expander->write(io_expander_pin_number, HIGH);

}

