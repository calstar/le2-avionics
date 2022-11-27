/* BOARD CONFIGURATION (edit this section to change it) */
/* ---------------------------------------------------- */
#define SLOT_IGNITER      0
#define SLOT_RTD          1
#define SLOT_THERMOCOUPLE 2
#define SLOT_PT_A         3
#define SLOT_PT_B         4
#define SLOT_SOLENOID     5
/* ---------------------------------------------------- */

#ifndef GROUND_TEST_HEADER_FILE_INCLUDED
  #include "GroundTest.h"
#endif

#include "IgniterBoard.h"
#include "RTDBoard.h"
#include "ThermocoupleBoard.h"
#include "PTBoard.h"

TwoWire *I2C_fast;
TwoWire *I2C_slow;
PCF8575 *IO_expander_core;
TCA9548A *I2C_mux;

IgniterBoard *igniter_board;
RTDBoard *rtd_board;
ThermocoupleBoard *thermocouple_board;
PTBoard *pt_board_a;
PTBoard *pt_board_b;

bool button_press_detected = false;

/* ---------------------------------------------------- */

/* Verify that a peripheral board with the provided id is connected
 * to the provided slot on the core board.
 * id: The numerical ID of the peripheral board which is inserted
 * into the core board.
 * slot: The slot number that the peripheral board is inserted
 * into the core board (from 0 to 7).
 * returns: True if the peripheral board is found in that slot, or
 * false if the peripheral board is not found in that slot. */
bool FindPeripheralBoard(int id, int slot) {
	int noise_tolerance = 40;
	int adc_inputs[8] = {analogRead(PIN_BOARD_ID_0), analogRead(PIN_BOARD_ID_1),
						           analogRead(PIN_BOARD_ID_2), analogRead(PIN_BOARD_ID_3),
						           analogRead(PIN_BOARD_ID_4), analogRead(PIN_BOARD_ID_5),
						           analogRead(PIN_BOARD_ID_6), analogRead(PIN_BOARD_ID_7)};
	int adc_value = adc_inputs[slot];
	return (abs(id - adc_value) < noise_tolerance);
}

/* Blinks a specific LED repeatedly using the IO Expander. 
 * led_name: The identity of the LED to blink (A, B, C, D, E, F, G, H) 
 * num_blinks: Number of times to blink the LED. 
 * returns: void. */
void BlinkLED(char led_name, int num_blinks) {
  uint8_t led_pin_number;
  switch (led_name) {
    case 'A' : led_pin_number = 0; break;
    case 'B' : led_pin_number = 1; break;
    case 'C' : led_pin_number = 2; break;
    case 'D' : led_pin_number = 3; break;
    case 'E' : led_pin_number = 4; break;
    case 'F' : led_pin_number = 5; break;
    case 'G' : led_pin_number = 6; break;
    case 'H' : led_pin_number = 7; break;
    default  : return;
  }
  for (int i = 0; i < num_blinks; i++) {
    IO_expander_core->write(led_pin_number, HIGH);
    delay(500);
    IO_expander_core->write(led_pin_number, LOW);
    delay(500);
  }
}

/* Interrupt Service Routine that runs when the IO Expander
 * detects a button press. This function updates the global
 * variable button_press_detected to record that a button was pressed.
 * returns: void. */
void IRAM_ATTR OnButtonPress() {
  button_press_detected = true;
}

/* Checks if the SD card has been connected properly, and
 * prints out information about the SD card.
 * returns: 0 if the SD card is properly connected, and -1 otherwise. */
int GetSDCardStatus() {
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
int SDCardRead(char *filename) {
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
int SDCardWrite(char *filename, uint8_t *data, uint8_t length, bool append) {
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
int SDCardDelete(char *filename) {
  if (SD.remove(filename)) {
    return 0;
  } else {
    return -1;
  }
}

/* ---------------------------------------------------- */

void setup() {

  Serial.begin(115200);
  Serial.println("Initializing core board...");

  /* Initialize the I2C_SLOW bus. */
  I2C_slow = new TwoWire(1);
  I2C_slow->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, 400000);

  /* Initialize the PCF8575 I/O Expander. The ESP32 uses the I2C_SLOW
   * bus to communicate with the I/O Expander. 
   * NOTE: Don't initalize the I2C_FAST bus before initializing the PCF8575. */
  uint16_t io_expander_initial_value = 0xFF00;
  IO_expander_core = new PCF8575(CORE_IO_EXPANDER_ADDRESS, I2C_slow);
  if (IO_expander_core->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, io_expander_initial_value)) {
    Serial.println(" - Connected to PCF8575 IO Expander");
  } else {
    Serial.println(" - Unable to connect to PCF8575 IO Expander");
  }
  // IO Expander will set the Interrupt pin low if it detects a button press
  pinMode(PIN_IO_EXPANDER_INTERRUPT, INPUT);
  attachInterrupt(PIN_IO_EXPANDER_INTERRUPT, OnButtonPress, FALLING);

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

  /* Initialize the SPI_CORE bus, and also initialize the SD Card. The
   * SPI_CORE bus is used only for communicating with the SD Card. */
  SPIClass SPI_core = SPIClass(VSPI);
  SPI_core.begin(PIN_SPI_SCK_CORE, PIN_SPI_MISO_CORE, PIN_SPI_MOSI_CORE, PIN_SPI_CS_CORE);
  pinMode(PIN_SPI_CS_CORE, OUTPUT);
  SD.begin(PIN_SPI_CS_CORE);

  /* Initialize the I2C_FAST bus. */
  I2C_fast = new TwoWire(0);
  I2C_fast->begin(PIN_I2C_FAST_SDA, PIN_I2C_FAST_SCL, 400000);

  /* Initialize the BROADCAST pin so we can toggle it to high or low. */
  pinMode(PIN_BROADCAST, INPUT);
  digitalWrite(PIN_BROADCAST, LOW);

  /* Check if the peripheral boards are connected. */
  if (FindPeripheralBoard(BOARD_ID_IGNITER, SLOT_IGNITER)) {
    Serial.println(" - Found peripheral board: Igniter");
  } else {
    Serial.println(" - Unable to find peripheral board: Igniter");
  }
  if (FindPeripheralBoard(BOARD_ID_RTD, SLOT_RTD)) {
    Serial.println(" - Found peripheral board: RTD");
  } else {
    Serial.println(" - Unable to find peripheral board: RTD");
  }
  if (FindPeripheralBoard(BOARD_ID_THERMOCOUPLE, SLOT_THERMOCOUPLE)) {
    Serial.println(" - Found peripheral board: Thermocouple");
  } else {
    Serial.println(" - Unable to find peripheral board: Thermocouple");
  }
  if (FindPeripheralBoard(BOARD_ID_PT, SLOT_PT_A)) {
    Serial.println(" - Found peripheral board: Pressure Transducer A");
  } else {
    Serial.println(" - Unable to find peripheral board: Pressure Transducer A");
  }
  if (FindPeripheralBoard(BOARD_ID_PT, SLOT_PT_B)) {
    Serial.println(" - Found peripheral board: Pressure Transducer B");
  } else {
    Serial.println(" - Unable to find peripheral board: Pressure Transducer B");
  }

  /* Initialize the peripheral boards. */
  igniter_board = new IgniterBoard(SLOT_IGNITER, I2C_slow);
  rtd_board = new RTDBoard(SLOT_RTD, I2C_slow);
  thermocouple_board = new ThermocoupleBoard(SLOT_RTD, I2C_slow);
  pt_board_a = new PTBoard(SLOT_PT_A, I2C_slow);
  pt_board_b = new PTBoard(SLOT_PT_B, I2C_slow);

  Serial.println("Succesfully initialized core board.");

}

/* ---------------------------------------------------- */

void loop() {

  /* If a button press is detected, run a debugging test.
   * The test to run depends on which button was pressed. */
  if (button_press_detected) {

    Serial.println("Button press detected!");
    uint16_t all_pins = IO_expander_core->read16();
    uint8_t button_pins = (uint8_t) (all_pins >> 8);

    // Check which button was pressed and run the corresponding test.
    // button_pins is a byte with 8 bits, and 7 of them are set to 1.
    // The bit that is set to 0 corresponds to the button that was pressed.
    char test_name = 'X';
    int error_code = 0;
    switch (button_pins) {
      case 0xFE : test_name = 'A'; error_code = RunTestA(); break;
      case 0xFD : test_name = 'B'; error_code = RunTestB(); break;
      case 0xFB : test_name = 'C'; error_code = RunTestC(); break;
      case 0xF7 : test_name = 'D'; error_code = RunTestD(); break;
      case 0xEF : test_name = 'E'; error_code = RunTestE(); break;
      case 0xDF : test_name = 'F'; error_code = RunTestF(); break;
      case 0xBF : test_name = 'G'; error_code = RunTestG(); break;
      case 0x7F : test_name = 'H'; error_code = RunTestH(); break;
    }

    // Report the results of the test. Print out the error code, and blink
    // the corresponding led the same number of times as error code value.
    // Error Code = 0: No test was run.
    // Error Code = 1: Test passed.
    // Error Code > 1: Test failed.
    Serial.print("Test ");
    Serial.print(test_name);
    if (error_code == 1) {
      Serial.print(" PASSED ");
    } else {
      Serial.print(" FAILED ");
    }
    Serial.print("(Error Code = ");
    Serial.print(error_code);
    Serial.println(")");
    BlinkLED(test_name, error_code);

    // Reset all the button pins to HIGH. Fixes an issue where you
    // press one button and then the IO Expander stops being able to
    // detect any other buttons.
    IO_expander_core->write16(0xFF00);

    // Delay so that you don't detect multiple button presses
    // at once due to the switch bouncing.
    delay(1000);
    button_press_detected = false;
    
  }

  /*
  igniter_board->OpenRelay(0);
  delay(100);
  igniter_board->CloseRelay(0);
  delay(100);

  rtd_board->PrintData(0);

  thermocouple_board->PrintData(0);

  I2C_mux->closeAll();
  I2C_mux->openChannel(SLOT_PT_A);
  pt_board_a->PrintDataLC(0, 128);
  pt_board_a->PrintDataPT(0, 128);

  I2C_mux->closeAll();
  I2C_mux->openChannel(SLOT_PT_B);
  pt_board_b->PrintDataLC(0, 128);
  pt_board_b->PrintDataPT(0, 128);
  */
  
}

/* ---------------------------------------------------- */

int RunTestA() {
  return 1;
}

int RunTestB() {
  return 1;
}

int RunTestC() {
  return 1;
}

int RunTestD() {
  return 1;
}

int RunTestE() {
  return 1;
}

int RunTestF() {
  return 1;
}

int RunTestG() {
  return 1;
}

int RunTestH() {
  return 1;
}

/* ---------------------------------------------------- */