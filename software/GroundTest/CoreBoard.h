#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

/* Library: https://www.arduino.cc/reference/en/libraries/tca9548a/ */
#include <TCA9548A.h>

/* Library: https://www.arduino.cc/reference/en/libraries/pcf8575/ */
#include <PCF8575.h>

#define PIN_I2C_SLOW_SDA 21
#define PIN_I2C_SLOW_SCL 22

#define PIN_I2C_FAST_SDA 16
#define PIN_I2C_FAST_SCL 17

#define SPI_MISO 12
#define SPI_MOSI 13
#define SPI_SCK 14
#define SPI_CS_0 15
#define SPI_CS_1 26
#define SPI_CS_2 27

#define SPI_MISO_CORE 19
#define SPI_MOSI_CORE 23
#define SPI_SCK_CORE 18
#define SPI_CS_CORE 5

#define PIN_BOARD_ID_0 36
#define PIN_BOARD_ID_1 39
#define PIN_BOARD_ID_2 34
#define PIN_BOARD_ID_3 35
#define PIN_BOARD_ID_4 32
#define PIN_BOARD_ID_5 33
#define PIN_BOARD_ID_6 0
#define PIN_BOARD_ID_7 2

#define PIN_BROADCAST 25

#define BOARD_ID_IGNITER      430
#define BOARD_ID_SOLENOID     1018
#define BOARD_ID_SERVO        1139
#define BOARD_ID_THERMOCOUPLE 1615
#define BOARD_ID_RTD          2199
#define BOARD_ID_PT_A         2812
#define BOARD_ID_PT_B         3726

#define I2C_MUX_ADDRESS 0x70
#define IO_EXPANDER_ADDRESS 0x20

class CoreBoard {

private:
  TwoWire *I2C_fast;
  TwoWire *I2C_slow;
	TCA9548A *I2C_mux;
  PCF8575 *IO_expander;
  SPISettings *spi_settings;

public:
	CoreBoard();
	int FindPeripheralBoard(int id);
  int I2CWrite(int slot, uint8_t address, uint8_t* buf_out, uint8_t length);
	int I2CRead(int slot, uint8_t address, uint8_t* buf_in, uint8_t length);
	int I2CFastWrite(uint8_t address, uint8_t buf_out[], uint8_t length);
	int I2CFastRead(uint8_t address, uint8_t buf_in[], uint8_t length);
  void BlinkLED(char led_name, int num_blinks);
  char GetButtonPress();
  void SetBroadcast(bool value);
  

};


