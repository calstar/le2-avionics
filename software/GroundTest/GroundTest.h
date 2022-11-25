#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>

/* Library: https://www.arduino.cc/reference/en/libraries/tca9548a/ */
#include <TCA9548A.h>

/* Library: https://www.arduino.cc/reference/en/libraries/pcf8575/ */
#include <PCF8575.h>

/* Value that can be checked in other files so that this file isn't included twice. */
#define GROUND_TEST_HEADER_FILE_INCLUDED true

#define PIN_I2C_SLOW_SDA 21
#define PIN_I2C_SLOW_SCL 22

#define PIN_I2C_FAST_SDA 16
#define PIN_I2C_FAST_SCL 17

#define PIN_SPI_MISO 12
#define PIN_SPI_MOSI 13
#define PIN_SPI_SCK 14
#define PIN_SPI_CS_0 15
#define PIN_SPI_CS_1 26
#define PIN_SPI_CS_2 27

#define PIN_SPI_MISO_CORE 19
#define PIN_SPI_MOSI_CORE 23
#define PIN_SPI_SCK_CORE 18
#define PIN_SPI_CS_CORE 5

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
#define CORE_IO_EXPANDER_ADDRESS 0x20

