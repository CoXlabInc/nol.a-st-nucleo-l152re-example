#include <cox.h>

Timer tPrint;

static void printTask(void *) {
  ledToggle();
  printf("[%lu usec] Hi!\n", micros());

}

static void keyboard(SerialPort&) {
  ledToggle();
  printf("[%lu usec] Keyboard input\n", micros());
}

static void button() {
  ledToggle();
  printf("[%lu usec] User button pressed\n", micros());
}

extern uint32_t SystemCoreClock;

void setup() {
  ledOn();
  Serial.begin(115200);
  printf("\n*** Serial test for ST Nucleo-L152RE *** (%lu Hz)\n", SystemCoreClock);

  pinMode(BUTTON_BUILTIN, INPUT);
  attachInterrupt(BUTTON_BUILTIN, button, FALLING);

  tPrint.onFired(printTask, NULL);
  tPrint.startPeriodic(1000);

  Serial.listen();
  Serial.onReceive(keyboard);
}
