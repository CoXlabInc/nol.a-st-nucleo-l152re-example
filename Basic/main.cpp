#include <cox.h>

Timer tPrint;

static void printTask(void *) {
  System.ledToggle();
  printf("[%lu usec] Hi!\n", micros());
}

static void keyboard(SerialPort&) {
  System.ledToggle();
  printf("[%lu usec] Keyboard input\n", micros());
}

static void button() {
  System.ledToggle();
  printf("[%lu usec] User button pressed\n", micros());
}

void setup() {
  System.ledOn();
  Serial.begin(115200);
  printf("\n*** Serial test for ST Nucleo-L152RE ***\n");

  pinMode(BUTTON_BUILTIN, INPUT);
  attachInterrupt(BUTTON_BUILTIN, button, FALLING);

  tPrint.onFired(printTask, NULL);
  tPrint.startPeriodic(10000);

  Serial.listen();
  Serial.onReceive(keyboard);
}
