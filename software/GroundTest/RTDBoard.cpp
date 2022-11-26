#include "RTDBoard.h"

/* Cosntructor for the RTDBoard class. */
RTDBoard::RTDBoard(int slot_argument, TwoWire *I2C_slow_argument) {

  slot = slot_argument;
  I2C_slow = I2C_slow_argument;

  // Initialize the IO Expander
  uint16_t io_expander_initial_value = 0xFFFF;
  IO_expander = new PCF8575(RTD_BOARD_IO_EXPANDER_ADDRESS, I2C_slow);
  IO_expander->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, io_expander_initial_value);

  // Initalize each RTD amplifier, and add each one to the array RTD_amplifiers.
  for (int rtd_number = 0; rtd_number < NUM_RTD_AMPLIFIERS; rtd_number++) {

    // Decide which IO Expander pin to use as the RTD amplifier CS pin
    int io_expander_pin_number = rtd_number + 8;
    IO_expander->write(io_expander_pin_number, LOW);

    Adafruit_MAX31865 *RTD_amplifier = new Adafruit_MAX31865(PIN_SPI_CS_0, PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK);
    if (!RTD_amplifier->begin(MAX31865_3WIRE)) {
      Serial.print("Could not initialize RTD #");
      Serial.println(rtd_number);
    }
    RTD_amplifiers[rtd_number] = RTD_amplifier;

    IO_expander->write(io_expander_pin_number, HIGH);

  }

}

/* Prints out resistance and temperature data from the RTD.
 * Most of this is copied directly from the Adafruit RTD example
 * located at file->examples->AdafruitMAX31865Library->max31865. 
 * rtd_number: the index of the rtd that you want to read from,
 * must be one of (0, 1, 2, 3, 4, 5, 6, 7). 
 * returns: void */
void RTDBoard::PrintData(int rtd_number) {

  if (rtd_number < 0 || rtd_number >= NUM_RTD_AMPLIFIERS) {
    Serial.println("Error: RTD number out of range");
    return;
  }

  int io_expander_pin_number = rtd_number + 8;
  Adafruit_MAX31865 *RTD_amplifier = RTD_amplifiers[rtd_number];
  
  // Toggle CS pin to low and begin SPI communication
  IO_expander->write(io_expander_pin_number, LOW);

  Serial.print("Data for RTD #");
  Serial.print(rtd_number);
  Serial.println(":");

  uint16_t rtd = RTD_amplifier->readRTD();
  Serial.print(" - RTD value: ");
  Serial.println(rtd);

  float ratio = rtd / 32768;
  Serial.print(" - Ratio = ");
  Serial.println(ratio,8);

  Serial.print(" - Resistance = ");
  Serial.println(RREF*ratio,8);

  Serial.print(" - Temperature = ");
  Serial.println(RTD_amplifier->temperature(RNOMINAL, RREF));

  Serial.print(" - Error Status: ");

  uint8_t fault = RTD_amplifier->readFault();
  if (fault) {
    Serial.print("Fault 0x");
    Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold"); 
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold"); 
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias"); 
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage"); 
    }
    RTD_amplifier->clearFault();
  } else {
    Serial.println("No Error");
  }

  // Toggle CS pin to high and end SPI communication
  IO_expander->write(io_expander_pin_number, HIGH);

}
