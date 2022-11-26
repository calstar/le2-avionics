#ifndef GROUND_TEST_HEADER_FILE_INCLUDED
  #include "GroundTest.h"
#endif

#define PT_BOARD_IO_EXPANDER_ADDRESS 0x23

// The pin numbers (0 to 15) on the IO Expander that connect to each PT or LC amplifier.
#define LC_0_CLK_PIN  15
#define LC_0_DATA_PIN 14
#define LC_1_CLK_PIN  0
#define LC_1_DATA_PIN 1
#define PT_0_CLK_PIN  9
#define PT_0_DATA_PIN 8
#define PT_1_CLK_PIN  11
#define PT_1_DATA_PIN 10
#define PT_2_CLK_PIN  13
#define PT_2_DATA_PIN 12
#define PT_3_CLK_PIN  6
#define PT_3_DATA_PIN 7
#define PT_4_CLK_PIN  4
#define PT_4_DATA_PIN 5
#define PT_5_CLK_PIN  2
#define PT_5_DATA_PIN 3

class PTBoard {
  private:
    int slot;
    TwoWire *I2C_slow;
    PCF8575 *IO_expander;
  public:
    PTBoard(int slot_argument, TwoWire *I2C_slow_argument);
    void PrintDataPT(int pt_number, uint8_t gain);
    void PrintDataLC(int lc_number, uint8_t gain);
    long ReadFromAmplifier(int io_expander_data_pin, int io_expander_clk_pin, uint8_t gain);
    uint8_t ShiftIn(int io_expander_data_pin, int io_expander_clk_pin);
};