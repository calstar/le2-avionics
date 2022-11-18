#include "CoreBoard.h"

/* Constructor for the CoreBoard class. */
CoreBoard::CoreBoard() {

  I2C_fast = new TwoWire(0);
  I2C_slow = new TwoWire(1);

  I2C_fast->begin(PIN_I2C_FAST_SDA, PIN_I2C_FAST_SCL, 400000);
  I2C_slow->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, 400000);

  I2C_mux = new TCA9548A(I2C_MUX_ADDRESS);
	I2C_mux->begin(*I2C_slow);
  I2C_mux->closeAll();

  uint16_t io_expander_initial_value = 0x0000;
  IO_expander = new PCF8575(IO_EXPANDER_ADDRESS, I2C_slow);
  IO_expander->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, io_expander_initial_value);

  spi_settings = &SPISettings(14000000, MSBFIRST, SPI_MODE0);

  pinMode(PIN_BROADCAST, OUTPUT);
  digitalWrite(PIN_BROADCAST, LOW);

	Serial.print("Initialized core board!\n");

}

/* Check if a peripheral board with the provided id is connected
 * to a slot on the core board.
 * id: The numerical ID of the peripheral board which the core
 * board should be connected to.
 * returns: the slot number (0 to 7) that the peripheral board is
 * connected to. If the peripheral board is not found, return -1. */
int CoreBoard::FindPeripheralBoard(int id) {

	int noise_tolerance = 40;

	bool found_board = false;
	int slot_with_correct_board = -1;

	int adc_inputs[8] = {analogRead(PIN_BOARD_ID_0), analogRead(PIN_BOARD_ID_1),
						           analogRead(PIN_BOARD_ID_2), analogRead(PIN_BOARD_ID_3),
						           analogRead(PIN_BOARD_ID_4), analogRead(PIN_BOARD_ID_5),
						           analogRead(PIN_BOARD_ID_6), analogRead(PIN_BOARD_ID_7)};

	for (int slot = 0; slot < 7; slot++) {
		int adc_value = adc_inputs[slot];
		if (abs(id - adc_value) < noise_tolerance) {
			if (found_board) {
				return -1;
			} else {
				found_board = true;
				slot_with_correct_board = slot;
			}
		}
	}

	if (found_board) {
		return slot_with_correct_board;
	} else {
		return -1;
	}

}

/* Write to the I2C bus on a peripheral board.
 * slot: The slot number (0 to 7) of the peripheral board to write to.
 * address: The address of the I2C device to write to.
 * buf_out: The buffer with data to be written to the bus.
 * len: How many bytes to write. 
 * returns: 0 on success, -1 on error. */
int CoreBoard::I2CWrite(int slot, uint8_t address, uint8_t* buf_out, uint8_t length) {
	if (slot < 0 or slot > 7) {
		return -1;
	}
	I2C_mux->openChannel(slot);
	I2C_slow->beginTransmission(address);
  I2C_slow->write(buf_out, length);
  if (I2C_slow->endTransmission() != 0) {
		return -1;
	}
  I2C_mux->closeChannel(slot);
  return 0;
}

/* Read from the I2C bus on a peripheral board.
 * slot: The slot number (0 to 7) of the peripheral board to write to.
 * address: The address of the I2C device to write to.
 * buf_in: The empty buffer that the data from the bus will be written to.
 * len: How many bytes to read. 
 * returns: 0 on success, -1 on error. */
int CoreBoard::I2CRead(int slot, uint8_t address, uint8_t* buf_in, uint8_t length) {
	if (slot < 0 or slot > 7) {
		return -1;
	}
	I2C_mux->openChannel(slot);
	I2C_slow->requestFrom(address, length);
	if (I2C_slow->available() >= length) {
		for (int byte = 0; byte < length; byte++) {
			buf_in[byte] = I2C_slow->read();
		}
	} else {
		return -1;
	}
	I2C_mux->closeChannel(slot);
  return 0;
}

/* Write to the I2C_FAST bus.
 * address: The address of the I2C device to write to.
 * buf_out: The buffer with data to be written to the bus.
 * len: How many bytes to write. 
 * returns: 0 on success, -1 on error. */
int CoreBoard::I2CFastWrite(uint8_t address, uint8_t buf_out[], uint8_t length) {
  I2C_fast->beginTransmission(address);
  I2C_fast->write(buf_out, length);
  if (I2C_fast->endTransmission() != 0) {
		return -1;
	}
  return 0;
}

/* Read from the I2C_FAST bus.
 * address: The address of the I2C device to read from.
 * buf_in: The empty buffer that the data from the bus will be written to.
 * len: How many bytes to read.
 * returns: 0 on success, -1 on error. */
int CoreBoard::I2CFastRead(uint8_t address, uint8_t buf_in[], uint8_t length) {
	I2C_fast->requestFrom(address, length);
	if (I2C_fast->available() >= length) {
		for (int byte = 0; byte < length; byte++) {
			buf_in[byte] = I2C_fast->read();
		}
	} else {
		return -1;
	}
  return 0;
}

/* Blinks a specific LED repeatedly using the IO Expander. 
 * led_name: The identity of the LED to blink (A, B, C, D, E, F, G, H) 
 * num_blinks: Number of times to blink the LED. 
 * returns: void. */
void CoreBoard::BlinkLED(char led_name, int num_blinks) {
  uint8_t led_pin_number;
  switch (led_name) {
    case 'A' : led_pin_number = 8;
    case 'B' : led_pin_number = 9;
    case 'C' : led_pin_number = 10;
    case 'D' : led_pin_number = 11;
    case 'E' : led_pin_number = 12;
    case 'F' : led_pin_number = 13;
    case 'G' : led_pin_number = 14;
    case 'H' : led_pin_number = 15;
  }
  for (int i = 0; i < num_blinks; i++) {
    IO_expander->write(led_pin_number, HIGH);
    delay(500);
    IO_expander->write(led_pin_number, LOW);
    delay(500);
  }
}

/* Checks if one of the buttons is being pressed using the IO Expander.
 * returns: The name of the button currently being pressed (A, B, C, D, E, F, G, H).
 * If multiple buttons are being pressed, or if no buttons are being pressed,
 * then it returns 'X'. */
char CoreBoard::GetButtonPress() {
  uint8_t button_values = (uint8_t) (IO_expander->read16() & 0x00FF);
  switch (button_values) {
    case 0x01 : return 'A';
    case 0x02 : return 'B';
    case 0x04 : return 'C';
    case 0x08 : return 'D';
    case 0x10 : return 'E';
    case 0x20 : return 'F';
    case 0x40 : return 'G';
    case 0x80 : return 'H';
  }
  return 'X';
}

/* Toggle the broadcast pin to either 1 or 0.
 * value: boolean to set the broadcast pin's output to
 * returns: 0 on success, -1 on error. */
void CoreBoard::SetBroadcast(bool value) {
  digitalWrite(PIN_BROADCAST, value);
}


