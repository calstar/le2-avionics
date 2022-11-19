#include "CoreBoard.h"

/* Constructor for the CoreBoard class. */
CoreBoard::CoreBoard() {

  /* Initialize the I2C_FAST bus. */
  I2C_fast = new TwoWire(0);
  I2C_fast->begin(PIN_I2C_FAST_SDA, PIN_I2C_FAST_SCL, 400000);

  /* Initialize the I2C_SLOW bus. */
  I2C_slow = new TwoWire(1);
  I2C_slow->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, 400000);

  /* Initialize the TCA9548A I2C mux. The ESP32 uses the I2C_SLOW
   * bus to communicate with the I2C mux. */
  I2C_mux = new TCA9548A(I2C_MUX_ADDRESS);
	I2C_mux->begin(*I2C_slow);
  I2C_mux->closeAll();

  /* Initialize the PCF8575 I/O Expander. The ESP32 uses the I2C_SLOW
   * bus to communicate with the I/O Expander. */
  uint16_t io_expander_initial_value = 0x0000;
  IO_expander = new PCF8575(IO_EXPANDER_ADDRESS, I2C_slow);
  IO_expander->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, io_expander_initial_value);
  pinMode(PIN_IO_EXPANDER_INTERRUPT, INPUT);

  /* Initialize the SPI_CORE bus, and also initialize the SD Card. The
   * SPI_CORE bus is used only for communicating with the SD Card. */
  SPI_core = new SPIClass(VSPI);
  SPI_core->begin(SPI_SCK_CORE, SPI_MISO_CORE, SPI_MOSI_CORE, SPI_CS_CORE);
  pinMode(SPI_CS_CORE, OUTPUT);
  SD.begin(SPI_CS_CORE);

  /* Initialize the SPI_PERIPHERAL bus. The SPI_PERIPHERAL bus is used for
   * communicating with all of the SPI devices on the peripheral boards. */
  SPI_peripheral = new SPIClass(HSPI);
  SPI_peripheral->begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS_0);
  pinMode(SPI_CS_0, OUTPUT);
  pinMode(SPI_CS_1, OUTPUT);
  pinMode(SPI_CS_2, OUTPUT);

  /* Initialize the BROADCAST pin. */
  pinMode(PIN_BROADCAST, OUTPUT);
  digitalWrite(PIN_BROADCAST, LOW);

	Serial.print("Initialized core board!\n");

}

/* Check if a peripheral board with the provided id is connected
 * to a slot on the core board.
 * id: The numerical ID of the peripheral board which the core
 * board should be connected to.
 * returns: The slot number (0 to 7) that the peripheral board is
 * connected to. If the peripheral board is not found, or if 
 * it is found multiple times, return -1. */
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
  if (digitalRead(PIN_IO_EXPANDER_INTERRUPT) == LOW) {
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
  }
  return 'X';
}

/* Toggle the broadcast pin to either 1 or 0.
 * value: Boolean to set the broadcast pin's output to
 * returns: void. */
void CoreBoard::SetBroadcast(bool value) {
  digitalWrite(PIN_BROADCAST, value);
}

/* Checks if the SD card has been connected properly, and
 * prints out information about the SD card.
 * returns: 0 if the SD card is properly connected, and -1 otherwise. */
int CoreBoard::GetSDCardStatus() {
  if (SD.cardType() == CARD_NONE) {
    return -1;
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  return 0;
}

/* Opens a file in the SD card for reading, and prints out the
 * file contents over the serial port.
 * filename: The name of the file to read. 
 * returns: 0 on success, -1 on failure. */
int CoreBoard::SDCardRead(char *filename) {
  File file = SD.open(filename, FILE_READ);
  if (file) {
    Serial.print("Contents of file ");
    Serial.print(filename);
    Serial.println(":");
    while (file.available()) {
    	Serial.write(file.read());
    }
    file.close();
    return 0;
  } else {
    Serial.print("Unable to open file: ");
    Serial.println(filename);
    return -1;
  }
}

/* Open a file in the SD card for writing, and write the
 * provided data to the SD card in that file. If the file does
 * not exist, then it will be created.
 * filename: The name of the file to write to.
 * data: Pointer to the data that should be written to the file.
 * length: Number of bytes to write to the file.
 * append: If this is set to true, the data will be appended
 * to the file without overwriting any previous data.
 * returns: 0 on success, -1 on failure. */
int CoreBoard::SDCardWrite(char *filename, uint8_t *data, uint8_t length, bool append) {
  File file;
  if (append) {
    file = SD.open(filename, FILE_APPEND);
  } else {
    file = SD.open(filename, FILE_WRITE);
  }
  if (file) {
    file.write(data, length);
    file.close();
    return 0;
  } else {
    return -1;
  }
}

/* Delete a file from the SD card.
 * filename: The name of the file to delete.
 * returns: 0 on success, -1 on failure. */
int CoreBoard::SDCardDelete(char *filename) {
  if (SD.remove(filename)) {
    return 0;
  } else {
    return -1;
  }
}