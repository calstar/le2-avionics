#include "CoreBoard.h"
#include <string>

/* Prints out the ADC voltage reading at all 8 BOARD_ID pins. */
void PrintADCVoltageReadings() {
  int num_readings = 1000;
  int sums [8] = {0, 0, 0, 0, 0, 0, 0, 0};
  for (int reading = 0; reading < num_readings; reading++) {
    delay(1);
    sums[0] += analogRead(PIN_BOARD_ID_0);
    sums[1] += analogRead(PIN_BOARD_ID_1);
    sums[2] += analogRead(PIN_BOARD_ID_2);
    sums[3] += analogRead(PIN_BOARD_ID_3);
    sums[4] += analogRead(PIN_BOARD_ID_4);
    sums[5] += analogRead(PIN_BOARD_ID_5);
    sums[6] += analogRead(PIN_BOARD_ID_6);
    sums[7] += analogRead(PIN_BOARD_ID_7);
  }
  Serial.println("ADC Voltage Readings: ");
  for (int index = 0; index < 8; index++) {
    int average = sums[index] / num_readings;
    Serial.print("Pin ");
    Serial.print(index);
    Serial.print(": ");
    Serial.println(average);
  }
}

void setup() {
  Serial.begin(115200);
  CoreBoard core;
}

void loop() { }

