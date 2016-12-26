#include <cox.h>

Timer tPrint;

static void printTask(void *) {
  System.ledToggle();
  printf("[%lu usec] Hi!\n", micros());
  SerialUSB.printf("[%lu usec] Hi!\n", micros());
}

static void keyboard(SerialPort&) {
  System.ledToggle();

  while (Serial.available() > 0) {
    printf("[%lu usec] Keyboard input: 0x%02X\n", micros(), Serial.read());
  }
}

static void button() {
  System.ledToggle();
  printf("[%lu usec] User button pressed\n", micros());
}

static void eventVcpInput(SerialPort &) {
  System.ledToggle();

  while (SerialUSB.available() > 0) {
    SerialUSB.printf("[%lu usec] Keyboard input: 0x%02X\n", micros(), SerialUSB.read());
  }
}

void setup() {
  System.ledOn();
  Serial.begin(115200);
  printf("\n*** Serial test for ST Nucleo-L152RE ***\n");

  pinMode(BUTTON_BUILTIN, INPUT);
  attachInterrupt(BUTTON_BUILTIN, button, FALLING);

  tPrint.onFired(printTask, NULL);
  tPrint.startPeriodic(1000);

  Serial.onReceive(keyboard);
  Serial.listen();

  SerialUSB.begin();
  SerialUSB.onReceive(eventVcpInput);
  SerialUSB.listen();
}
