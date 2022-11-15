#include "CoreBoard.h"

/* Constructor for the CoreBoard class. */
CoreBoard::CoreBoard() {

  TwoWire bus_fast = TwoWire(0);
  TwoWire bus_slow = TwoWire(1);

  bus_fast.begin(PIN_I2C_FAST_SDA, PIN_I2C_FAST_SCL, 400000);
  bus_slow.begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, 400000);

  TCA9548A mux(I2C_MUX_ADDRESS);
	mux.begin(bus_slow);
  mux.closeAll();

  I2C_fast = &(bus_fast);
  I2C_slow = &(bus_slow);
  I2C_mux = &(mux);

	Serial.print("Initialized core board!");

}

/* Check if a peripheral board with the provided id is connected
 * to a slot on the core board.
 * id: The numerical ID of the peripheral board which the core
 * board should be connected to.
 * returns: the slot number (0 to 7) that the peripheral board is
 * connected to. If the peripheral board is not found, return -1. */
int CoreBoard::FindPeripheralBoard(int id) {

	int noise_tolerance = 50;

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
				Serial.print("Error in FindPeripheralBoard()");
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
		Serial.print("Error in FindPeripheralBoard()");
		return -1;
	}

}

/* Write to the I2C bus on a peripheral board.
 * slot: The slot number (0 to 7) of the peripheral board to write to.
 * address: The address of the I2C device to write to.
 * buf_out: The buffer with data to be written to the bus.
 * len: How many bytes to write. 
 * returns: 0 on success, -1 on error. */
int CoreBoard::I2CWrite(int slot, uint8_t address, uint8_t* buf_out, uint8_t len) {
	if (slot < 0 or slot > 7) {
		Serial.print("Error in I2CWrite()");
		return -1;
	}
	I2C_mux->openChannel(slot);
	I2C_slow->beginTransmission(address);
  	I2C_slow->write(buf_out, len);
  	if (I2C_slow->endTransmission() != 0) {
		Serial.print("Error in I2CWrite()");
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
int CoreBoard::I2CRead(int slot, uint8_t address, uint8_t* buf_in, uint8_t len) {
	if (slot < 0 or slot > 7) {
		Serial.print("Error in I2CRead()");
		return -1;
	}
	I2C_mux->openChannel(slot);
	I2C_slow->requestFrom(address, len);
	if (I2C_slow->available() >= len) {
		while (I2C_slow->available() > 0) {
			*buf_in = I2C_slow->read();
			buf_in += 4;
		}
	} else {
		Serial.print("Error in I2CRead()");
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
int CoreBoard::I2CFastWrite(uint8_t address, uint8_t* buf_out, uint8_t len) {
	I2C_fast->beginTransmission(address);
	I2C_fast->write(buf_out, len);
	if (I2C_fast->endTransmission() != 0) {
		Serial.print("Error in I2CFastWrite()");
		return -1;
	}
	return 0;
}

/* Read from the I2C_FAST bus.
 * address: The address of the I2C device to read from.
 * buf_in: The empty buffer that the data from the bus will be written to.
 * len: How many bytes to read.
 * returns: 0 on success, -1 on error. */
int CoreBoard::I2CFastRead(uint8_t address, uint8_t* buf_in, uint8_t len) {
	I2C_fast->requestFrom(address, len);
	if (I2C_fast->available() >= len) {
		while (I2C_fast->available() > 0) {
			*buf_in = I2C_fast->read();
			buf_in += 4;
		}
	} else {
		Serial.print("Error in I2CFastRead()");
		return -1;
	}
	return 0;
}