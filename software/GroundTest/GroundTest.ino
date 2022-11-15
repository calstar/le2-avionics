#include "CoreBoard.h"

void setup() {
  Serial.begin(115200);
}

void loop() {

  CoreBoard core;

  uint8_t example_address = 0x99;
  uint8_t example_data[3] = {0xAA, 0xBB, 0xCC};

  // Write to I2C_FAST bus
  core.I2CFastWrite(example_address, example_data, 3);

  // Find RTD Board
  int rtd_board_slot = core.FindPeripheralBoard(BOARD_ID_RTD);

  Serial.print("RTD Board slot: ");
  Serial.print(rtd_board_slot);

  // Write to I2C bus on RTD board
  core.I2CWrite(rtd_board_slot, example_address, example_data, 3);

  return;
  
}