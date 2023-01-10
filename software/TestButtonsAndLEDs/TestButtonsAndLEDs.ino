#include <Arduino.h>
#include <Wire.h>
/* Library: https://www.arduino.cc/reference/en/libraries/pcf8575/ */
#include <PCF8575.h>

#define PIN_I2C_SLOW_SDA 21
#define PIN_I2C_SLOW_SCL 22
#define PIN_IO_EXPANDER_INTERRUPT 4

#define CORE_IO_EXPANDER_ADDRESS 0x20

TwoWire *I2C_slow;
PCF8575 *IO_expander_core;

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

  Serial.println("Succesfully initialized core board.");

}

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
  
}

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

