#include <cox.h>

Timer tSense;

static void taskSense(void *) {
  uint8_t read;

  ledToggle();

  read = Wire.requestFrom(0x4F, 1);
  printf("Request 1 byte from 0x4F: %u byte read.\n", read);

  while (Wire.available()) {
    printf("%02X ", Wire.read());
  }
  printf("\n");

  read = Wire.requestFrom(0x4F, 2);
  printf("Request 2 byte from 0x4F: %u byte read.\n", read);

  while (Wire.available()) {
    printf("%02X ", Wire.read());
  }
  printf("\n");
}

void setup(void) {
  Serial.begin(115200);
  printf("\n*** [ST Nucleo-L152RE] I2C Test with AT30TSE758 Temperature Sensor ***\n");

  Wire.begin();

  tSense.onFired(taskSense, NULL);
  tSense.startPeriodic(1000);
}
