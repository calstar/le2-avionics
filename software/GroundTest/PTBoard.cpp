#include "PTBoard.h"

/* Cosntructor for the PTBoard class. */
PTBoard::PTBoard(int slot_argument, TwoWire *I2C_slow_argument, TCA9548A *I2C_mux_argument) {

  slot = slot_argument;
  I2C_slow = I2C_slow_argument;
  I2C_mux = I2C_mux_argument;

  I2C_mux->openChannel(slot);

  // Initial value has all CLK pins set to low and all DATA pins set to high.
  // In binary: (15) 0101010110101010 (0)
  uint16_t io_expander_initial_value = 0x55AA;
  IO_expander = new PCF8575(PT_BOARD_IO_EXPANDER_ADDRESS, I2C_slow);
  IO_expander->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, io_expander_initial_value);

  I2C_mux->closeChannel(slot);

}

/* Prints out data for one of the Pressure Transducers.
 * pt_number: the number of the PT to get data from. This number
 * must be one of (0, 1, 2, 3, 4, 5).
 * gain: Gain setting to use for the amplifier. This must be one of (32, 64, 128).
 * returns: void */
void PTBoard::PrintDataPT(int pt_number, uint8_t gain) {

  int io_expander_data_pin;
  int io_expander_clk_pin;
  switch (pt_number) {
    case 0 : io_expander_data_pin = PT_0_DATA_PIN; io_expander_clk_pin = PT_0_CLK_PIN; break;
    case 1 : io_expander_data_pin = PT_1_DATA_PIN; io_expander_clk_pin = PT_1_CLK_PIN; break;
    case 2 : io_expander_data_pin = PT_2_DATA_PIN; io_expander_clk_pin = PT_2_CLK_PIN; break;
    case 3 : io_expander_data_pin = PT_3_DATA_PIN; io_expander_clk_pin = PT_3_CLK_PIN; break;
    case 4 : io_expander_data_pin = PT_4_DATA_PIN; io_expander_clk_pin = PT_4_CLK_PIN; break;
    case 5 : io_expander_data_pin = PT_5_DATA_PIN; io_expander_clk_pin = PT_5_CLK_PIN; break;
  }

  I2C_mux->openChannel(slot);

  long pt_output = ReadFromAmplifier(io_expander_data_pin, io_expander_clk_pin, gain);

  I2C_mux->closeChannel(slot);

  if (!pt_output) { return; }
  Serial.print("Output from Pressure Transducer ");
  Serial.print(pt_number);
  Serial.print(": ");
  Serial.println(pt_output);

}

/* Prints out data for one of the Load Cells.
 * lc_number: the number of the LC to get data from. This number must be one of (0, 1).
 * gain: Gain setting to use for the amplifier. This must be one of (32, 64, 128).
 * returns: void */
void PTBoard::PrintDataLC(int lc_number, uint8_t gain) {

  int io_expander_data_pin;
  int io_expander_clk_pin;
  switch (lc_number) {
    case 0 : io_expander_data_pin = LC_0_DATA_PIN; io_expander_clk_pin = LC_0_CLK_PIN; break;
    case 1 : io_expander_data_pin = LC_1_DATA_PIN; io_expander_clk_pin = LC_1_CLK_PIN; break;
  }

  I2C_mux->openChannel(slot);

  long lc_output = ReadFromAmplifier(io_expander_data_pin, io_expander_clk_pin, gain);

  I2C_mux->closeChannel(slot);

  if (!lc_output) { return; }
  Serial.print("Output from Load Cell ");
  Serial.print(lc_number);
  Serial.print(": ");
  Serial.println(lc_output);

}

/* Reads out data from the HX711 Amplifier and prints it to the serial port.
 * Most of this was copied from the library here: https://github.com/bogde/HX711
 * io_expander_data_pin: The pin number (0 to 15) on the PT Board's
 * PCF8575 IO Expander that should be used for the data line.
 * io_expander_clk_pin: The pin number (0 to 15) on the PT Board's
 * PCF8575 IO Expander that should be used for the clock line.
 * gain: Gain setting to use for the amplifier. This must be one of (32, 64, 128).
 * returns: long value that was read from the HX711. */
long PTBoard::ReadFromAmplifier(int io_expander_data_pin, int io_expander_clk_pin, uint8_t gain) {

  /* Choose the gain setting for the amplifier. The setting must be one of (1, 2, 3). */
  int gain_normalized = 1;
  switch(gain) {
    case 128 : gain_normalized = 1; break;
    case 32  : gain_normalized = 2; break;
    case 64  : gain_normalized = 3; break;
  }

  /* Wait for the chip to become ready until timeout. The chip will set the 
   * data pin to LOW when it is ready. https://github.com/bogde/HX711/pull/96 */
	unsigned long millisStarted = millis();
  unsigned long timeout = 100;
  bool hx711_ready = false;
	while (millis() - millisStarted < timeout) {
		if (IO_expander->read(io_expander_data_pin) == LOW) {
			hx711_ready = true;
		}
		delay(1);
	}
	if (!hx711_ready) {
    Serial.println("Error: HX711 Amplifier is not ready.");
    return 0;
  }

  /* Define structures for reading data into. */
	unsigned long value = 0;
	uint8_t data[3] = { 0 };
	uint8_t filler = 0x00;

  /* Pulse the clock pin 24 times to read the data. */
  data[2] = ShiftIn(io_expander_data_pin, io_expander_clk_pin);
	data[1] = ShiftIn(io_expander_data_pin, io_expander_clk_pin);
	data[0] = ShiftIn(io_expander_data_pin, io_expander_clk_pin);

  /* Set the channel and the gain factor for the next reading using the clock pin. */
  for (unsigned int i = 0; i < gain_normalized; i++) {
		IO_expander->write(io_expander_clk_pin, HIGH);
		delayMicroseconds(1);
		IO_expander->write(io_expander_clk_pin, LOW);
		delayMicroseconds(1);
	}

  /* Replicate the most significant bit to pad out a 32-bit signed integer */
	if (data[2] & 0x80) {
		filler = 0xFF;
	} else {
		filler = 0x00;
	}

	/* Construct a 32-bit signed integer */
	value = ( static_cast<unsigned long>(filler) << 24
			    | static_cast<unsigned long>(data[2]) << 16
			    | static_cast<unsigned long>(data[1]) << 8
			    | static_cast<unsigned long>(data[0]));
	return static_cast<long>(value);

}

/* Helper function for the ReadFromAmplifier() function. Toggle the
 * clock high and low 8 times while reading in one bit at a time from
 * the Amplifier.The bits are shifted in MSB First. Most of this was
 * copied from the library here: https://github.com/bogde/HX711
 * io_expander_data_pin: The pin number (0 to 15) on the PT Board's
 * PCF8575 IO Expander that should be used for the data line.
 * io_expander_clk_pin: The pin number (0 to 15) on the PT Board's
 * PCF8575 IO Expander that should be used for the clock line.
 * returns: 8_bit value that was read in from the amplifier. */
uint8_t PTBoard::ShiftIn(int io_expander_data_pin, int io_expander_clk_pin) {
    uint8_t value = 0;
    uint8_t i;
    for(i = 0; i < 8; i++) {
        IO_expander->write(io_expander_clk_pin, HIGH);
        delayMicroseconds(1);
        value |= IO_expander->read(io_expander_data_pin) << (7 - i);
        IO_expander->write(io_expander_clk_pin, LOW);
        delayMicroseconds(1);
    }
    return value;
}
