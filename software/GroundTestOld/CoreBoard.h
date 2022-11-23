#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>

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

#define PIN_IO_EXPANDER_INTERRUPT 4

#define BOARD_ID_IGNITER      493
#define BOARD_ID_SOLENOID     1127
#define BOARD_ID_SERVO        1253
#define BOARD_ID_THERMOCOUPLE 1761
#define BOARD_ID_RTD          2385
#define BOARD_ID_PT_A         3024
#define BOARD_ID_PT_B         3806

#define I2C_MUX_ADDRESS 0x70
#define IO_EXPANDER_ADDRESS 0x20

class CoreBoard {
public:
  TwoWire *I2C_fast;
  TwoWire *I2C_slow;
	TCA9548A *I2C_mux;
  PCF8575 *IO_expander;
  SPIClass *SPI_core;
  SPIClass *SPI_peripheral;
	CoreBoard();
	bool FindPeripheralBoard(int id, int slot);
  void BlinkLED(char led_name, int num_blinks);
  char GetButtonPress();
  void SetBroadcast(bool value);
  int GetSDCardStatus();
  int SDCardRead(char *filename);
  int SDCardWrite(char *filename, uint8_t *data, uint8_t length, bool append);
  int SDCardDelete(char *filename);
};


