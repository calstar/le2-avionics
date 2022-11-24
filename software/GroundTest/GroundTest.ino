#ifndef GROUND_TEST_HEADER_FILE_INCLUDED
  #include "GroundTest.h"
#endif
#include "IgniterBoard.h"
#include "RTDBoard.h"

/* BOARD CONFIGURATION (edit this section to change it) */
/* ---------------------------------------------------- */
#define SLOT_IGNITER      0
#define SLOT_RTD          1
#define SLOT_THERMOCOUPLE 2
#define SLOT_PT_A         3
#define SLOT_PT_B         4
#define SLOT_SOLENOID     5
/* ---------------------------------------------------- */

TwoWire *I2C_fast;
TwoWire *I2C_slow;
PCF8575 *IO_expander;
TCA9548A *I2C_mux;

IgniterBoard *igniter_board;
RTDBoard *rtd_board;

bool button_press_detected = false;

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
  }
  for (int i = 0; i < num_blinks; i++) {
    IO_expander->write(led_pin_number, HIGH);
    delay(500);
    IO_expander->write(led_pin_number, LOW);
    delay(500);
  }
}

/* Interrupt Service Routine that runs when the IO Expander
 * detects a button press. This function updates the global
 * variable button_press_detected to record that a button was pressed.
 * returns: void. */
void IRAM_ATTR OnButtonPress() {
  if (!button_press_detected) {
    button_press_detected = true;
  }
}

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
  IO_expander = new PCF8575(IO_EXPANDER_ADDRESS, I2C_slow);
  if (IO_expander->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, io_expander_initial_value)) {
    Serial.println(" - Connected to PCF8575 IO Expander");
  } else {
    Serial.println(" - Unable to connect to PCF8575 IO Expander");
  }

  /* Setup the interrupt pin so the IO Expander can toggle the interrupt
   * pin if it detects a button press. */
  pinMode(PIN_IO_EXPANDER_INTERRUPT, INPUT);
  attachInterrupt(PIN_IO_EXPANDER_INTERRUPT, OnButtonPress, FALLING);

  /* Initialize the TCA9548A I2C mux. The ESP32 uses the I2C_SLOW
   * bus to communicate with the I2C mux. */
  I2C_mux = new TCA9548A(I2C_MUX_ADDRESS);
	I2C_mux->begin(*I2C_slow);
  I2C_mux->closeAll();
  // BeginTransmission and then EndTransmission to check if connection works
  I2C_slow->beginTransmission(I2C_MUX_ADDRESS);
  if (I2C_slow->endTransmission() == 0) {
    Serial.println(" - Connected to TCA9548A I2C Mux");
  } else {
    Serial.println(" - Unable to connect to TCA9548A I2C Mux");
  }

  /* Initialize the I2C_FAST bus. */
  I2C_fast = new TwoWire(0);
  I2C_fast->begin(PIN_I2C_FAST_SDA, PIN_I2C_FAST_SCL, 400000);

  /* Initialize the BROADCAST pin so we can toggle it to high or low. */
  pinMode(PIN_BROADCAST, INPUT);
  digitalWrite(PIN_BROADCAST, LOW);

  /* Check if the peripheral boards are properly connected. */
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
  if (FindPeripheralBoard(BOARD_ID_PT_A, SLOT_PT_A)) {
    Serial.println(" - Found peripheral board: Pressure Transducer A");
  } else {
    Serial.println(" - Unable to find peripheral board: Pressure Transducer A");
  }
  if (FindPeripheralBoard(BOARD_ID_PT_B, SLOT_PT_B)) {
    Serial.println(" - Found peripheral board: Pressure Transducer B");
  } else {
    Serial.println(" - Unable to find peripheral board: Pressure Transducer B");
  }
  if (FindPeripheralBoard(BOARD_ID_SOLENOID, SLOT_SOLENOID)) {
    Serial.println(" - Found peripheral board: Solenoid");
  } else {
    Serial.println(" - Unable to find peripheral board: Solenoid");
  }

  Serial.println("Succesfully initialized core board.");

  igniter_board = new IgniterBoard(SLOT_IGNITER, I2C_slow, I2C_mux);
  rtd_board = new RTDBoard(SLOT_RTD, I2C_slow, I2C_mux);

}

void loop() {

  /* Respond to button press on core board. */
  if (button_press_detected) {
    Serial.println("button press detected.");
    uint16_t all_pins = IO_expander->read16();
    uint8_t button_pins = (uint8_t) (all_pins >> 8);
    // Delay so that you don't detect multiple button presses
    // due to the switch bouncing, instead button presses are 
    // only valid if they're 1 second apart.
    delay(1000);
    switch (button_pins) {
      case 0xFE : Serial.println("Button press: A"); break;
      case 0xFD : Serial.println("Button press: B"); break;
      case 0xFB : Serial.println("Button press: C"); break;
      case 0xF7 : Serial.println("Button press: D"); break;
      case 0xEF : Serial.println("Button press: E"); break;
      case 0xDF : Serial.println("Button press: F"); break;
      case 0xBF : Serial.println("Button press: G"); break;
      case 0x7F : Serial.println("Button press: H"); break;
    }
    button_press_detected = false;
    // Reset all the button pins to HIGH. Fixes an issue where you
    // press one button and then the IO Expander stops being able to
    // detect any other buttons.
    IO_expander->write16(0xFF00);
  }

  /* Test igniter board */
  igniter_board->OpenRelay(0);
  delay(5000);
  igniter_board->CloseRelay(0);
  delay(5000);

  /* Test RTD Board */
  for (int i=0; i<8; i++) { rtd_board->PrintData(i); }
  for (int i=0; i<8; i++) { rtd_board->PrintErrorStatus(i); }
  
}
