#include <Arduino.h>
#include <Wire.h>

#include <TCA9548A.h>
/* Library: https://www.arduino.cc/reference/en/libraries/tca9548a/ */

#define PIN_I2C_SLOW_SDA 21
#define PIN_I2C_SLOW_SCL 22
#define PIN_I2C_FAST_SDA 16
#define PIN_I2C_FAST_SCL 17
#define PIN_BOARD_ID_0 36
#define PIN_BOARD_ID_1 39
#define PIN_BOARD_ID_2 34
#define PIN_BOARD_ID_3 35
#define PIN_BOARD_ID_4 32
#define PIN_BOARD_ID_5 33
#define PIN_BOARD_ID_6 0
#define PIN_BOARD_ID_7 2

#define BOARD_ID_IGNITER      0
#define BOARD_ID_SOLENOID     0
#define BOARD_ID_SERVO        0
#define BOARD_ID_THERMOCOUPLE 0
#define BOARD_ID_RTD          0
#define BOARD_ID_PT_A         0
#define BOARD_ID_PT_B         0

#define I2C_MUX_ADDRESS 0x70

class CoreBoard {

private:
  
  TwoWire *I2C_fast;
  TwoWire *I2C_slow;
	TCA9548A *I2C_mux;

public:
	CoreBoard();
	int FindPeripheralBoard(int id);
  int I2CWrite(int slot, uint8_t address, uint8_t* buf_out, uint8_t len);
	int I2CRead(int slot, uint8_t address, uint8_t* buf_in, uint8_t len);
	int I2CFastWrite(uint8_t address, uint8_t* buf_out, uint8_t len);
	int I2CFastRead(uint8_t address, uint8_t* buf_in, uint8_t len);

};


