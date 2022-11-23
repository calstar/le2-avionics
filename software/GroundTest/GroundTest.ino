#include "GroundTest.h"

TwoWire *I2C_slow;
PCF8575 *IO_expander;
bool button_press_detected = false;

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

  /* Initialize the I2C_SLOW bus. */
  I2C_slow = new TwoWire(1);
  I2C_slow->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, 400000);

  /* Initialize the PCF8575 I/O Expander. The ESP32 uses the I2C_SLOW
   * bus to communicate with the I/O Expander. */
  uint16_t io_expander_initial_value = 0xFF00;
  IO_expander = new PCF8575(IO_EXPANDER_ADDRESS, I2C_slow);
  IO_expander->begin(PIN_I2C_SLOW_SDA, PIN_I2C_SLOW_SCL, io_expander_initial_value);
  pinMode(PIN_IO_EXPANDER_INTERRUPT, INPUT);
  attachInterrupt(PIN_IO_EXPANDER_INTERRUPT, OnButtonPress, FALLING);

  Serial.println("Initialization sucessful!");

}

void loop() {

  if (button_press_detected) {
    uint16_t all_pins = IO_expander->read16();
    uint8_t button_pins = (uint8_t) (all_pins >> 8);
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
  }
  
}
