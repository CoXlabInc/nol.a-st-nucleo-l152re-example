#include <cox.h>

Timer tPrint;

static void printTask(void *) {
  System.ledToggle();
  int32_t a0, a1, a2, a3, a4, a5;
  a0 = analogRead(A0);
  a1 = analogRead(A1);
  a2 = analogRead(A2);
  a3 = analogRead(A3);
  a4 = analogRead(A4);
  a5 = analogRead(A5);
  printf("[%lu usec] A0:%ld, A1:%ld, A2:%ld, A3:%ld, A4:%ld, A5:%ld\n", micros(), a0, a1, a2, a3, a4, a5);
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
  tPrint.startPeriodic(500);
}
