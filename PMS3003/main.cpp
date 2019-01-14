#include <cox.h>

int32_t pm1_0_CF1 = -1;
int32_t pm2_5_CF1 = -1;
int32_t pm10_0_CF1 = -1;
int32_t pm1_0_Atmosphere = -1;
int32_t pm2_5_Atmosphere = -1;
int32_t pm10_0_Atmosphere = -1;

static void sensorDataReceived(SerialPort &) {
  static uint8_t index = 0;
  static uint8_t high;

  while (Serial2.available() > 0) {
    uint8_t c = Serial2.read();

    if ((index == 0 && c != 0x42) || (index == 1 && c != 0x4d)) {
      printf("* Wrong start: reset\n");
      index = 0;
      continue;

    } else if (index == 2 ||
                index == 4 ||
                index == 6 ||
                index == 8 ||
                index == 10 ||
                index == 12 ||
                index == 14) {
      high = c;
    } else if (index == 3) {
      uint16_t frameLength = (high << 8) | c;
      printf("\n* Start of frame (length: %u)\n", frameLength);
      if (frameLength != 20) {
        printf("* Wrong frame length (20 expected): reset\n");
        index = 0;
        continue;
      }
    } else if (index == 5) {
      pm1_0_CF1 = (high << 8) | c;
      printf("* PM1.0 (CF=1): %ld ug/m^3\n", pm1_0_CF1);
    } else if (index == 7) {
      pm2_5_CF1 = (high << 8) | c;
      printf("* PM2.5 (CF=1): %ld ug/m^3\n", pm2_5_CF1);
    } else if (index == 9) {
      pm10_0_CF1 = (high << 8) | c;
      printf("* PM10 (CF=1): %ld ug/m^3\n", pm10_0_CF1);
    } else if (index == 11) {
      pm1_0_Atmosphere = (high << 8) | c;
      printf("* PM1.0 (atmosphere): %ld ug/m^3\n", pm1_0_Atmosphere);
    } else if (index == 13) {
      pm2_5_Atmosphere = (high << 8) | c;
      printf("* PM2.5 (atmosphere): %ld ug/m^3\n", pm2_5_Atmosphere);
    } else if (index == 15) {
      pm10_0_Atmosphere = (high << 8) | c;
      printf("* PM10 (atmosphere): %ld ug/m^3\n", pm10_0_Atmosphere);
    } else if (index == 23) {
      index = 0;
      printf("* End of sensor output\n");
      continue;
    }
    index++;
  }
}

void setup() {
  Serial.begin(115200);
  printf("\n*** [ST Nucleo-L152RE] PMS3003 ***\n");

  pinMode(D6, OUTPUT);
  digitalWrite(D6, LOW);

  pinMode(D7, OUTPUT);
  digitalWrite(D7, LOW);
  delay(1000);
  digitalWrite(D7, HIGH);

  digitalWrite(D6, HIGH);

  Serial2.begin(9600);
  Serial2.onReceive(sensorDataReceived);
  Serial2.listen();
}
