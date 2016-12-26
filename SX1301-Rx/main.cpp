#include <cox.h>

#define RADIO_A_FREQ  922200000
#define RADIO_B_FREQ  923000000

Timer timerTest;

static void eventTest(void *) {
  SX1301.end();
  printf("!\n");
  SX1301.begin();
}

void setup() {
  Serial.begin(115200);
  printf("\n*** [ST Nucleo-L152RE] SX1301 Rx ***\n");

  SX1301.setSyncword(0x12);

  /*
    Radio A
    - Rx enabled
    - Tx enabled
    - Center frequency: 922.2 MHz
    - RSSI offset: -166.0
  */
  SX1301.configureRf(0, true, true, RADIO_A_FREQ, -166.0);

  /*
    Radio B
    - Rx enabled
    - Tx enabled
    - Center frequency: 923.0 MHz
    - RSSI offset: -166.0
  */
  SX1301.configureRf(1, true, true, RADIO_B_FREQ, -166.0);

  SX1301.configureRxIf(0, 0, true, 921900000 - RADIO_A_FREQ); //Multi LoRa
  SX1301.configureRxIf(1, 0, true, 922100000 - RADIO_A_FREQ); //Multi LoRa
  SX1301.configureRxIf(2, 0, true, 922300000 - RADIO_A_FREQ); //Multi LoRa
  SX1301.configureRxIf(3, 0, true, 922500000 - RADIO_A_FREQ); //Multi LoRa
  SX1301.configureRxIf(4, 1, true, 922700000 - RADIO_B_FREQ); //Multi LoRa
  SX1301.configureRxIf(5, 1, true, 922900000 - RADIO_B_FREQ); //Multi LoRa
  SX1301.configureRxIf(6, 1, true, 923100000 - RADIO_B_FREQ); //Multi LoRa
  SX1301.configureRxIf(7, 1, true, 923300000 - RADIO_B_FREQ); //Multi LoRa
  SX1301.configureRxIf(8, 0, false, 0);                       //LoRa standalone (not used)
  SX1301.configureRxIf(9, 0, false, 0);                       //FSK standalone (not used)
  SX1301.begin();

  for (uint8_t i = 0; i < 10; i++) {
    printf("* Rx interface:#%u, Frequency:%lu Hz\n", i, SX1301.getChannel(i));
  }

  timerTest.onFired(eventTest, NULL);
  timerTest.startPeriodic(5000);
}
