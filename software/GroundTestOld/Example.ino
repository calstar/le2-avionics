#include "CoreBoard.h"

void setup() {

  // Initialize the CoreBoard object
  Serial.begin(115200);
  CoreBoard core;

  /* -- Get the board slot for a peripheral board, in this case the RTD board */
  int rtd_board_slot = 3;
  if (!core.FindPeripheralBoard(BOARD_ID_RTD, rtd_board_slot)){
  	Serial.println("Could not find RTD board!");
  }

  /* -- Communicate with a device on the I2C_FAST bus */
  // LibraryFunction(i2c_bus = core.I2C_fast);

  /* -- Communicate with a device on a peripheral I2C bus */
  core.I2C_mux->openChannel(rtd_board_slot);
  // LibraryFunction(i2c_bus = core.I2C_slow);
  core.I2C_mux->closeChannel(rtd_board_slot);

  /* -- Blink an LED on the core board */
  core.BlinkLED('A', 5);

  /* -- Check if a button was pressed on the core board */
  char button_pressed = core.GetButtonPress();
  if (button_pressed == 'A') {
    // FunctionA();
  } else if (button_pressed = 'B') {
    // FunctionB();
  }

  /* -- Set the BROADCAST pin high or low */
  core.SetBroadcast(HIGH);
  core.SetBroadcast(LOW);

  /* -- Transfer data over the SPI bus to a peripheral board */
  digitalWrite(SPI_CS_0, LOW);
  // LibraryFunction(spi_bus = core.SPI_peripheral);
  digitalWrite(SPI_CS_0, HIGH);

  /* -- Transfer data to and from the SD card. */
  core.GetSDCardStatus();
  core.SDCardWrite("MyFile.txt", (uint8_t*) "hello", 5, false);
  core.SDCardRead("MyFile.txt");
  core.SDCardDelete("MyFile.txt");
  
}

void loop() { }

