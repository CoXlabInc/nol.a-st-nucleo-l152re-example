#include <cox.h>
#include <HX711.hpp>

Hx711 scale(PC0, PC3); /* PC3: SCK, PC0: DT*/
Timer timerMeasure;

static void taskMeasure(void *) {
  printf("[%lu usec] getting value...\n", micros());
  int32_t val = scale.getMilligram();
  printf("[%lu usec] %ld mg\n", micros(), val);
}

void setup() {
  Serial.begin(115200);
  printf("*** [ST Nucleo-L152RE] HX711 based Scale ***\n");

  scale.begin();
  timerMeasure.onFired(taskMeasure, NULL);
  timerMeasure.startPeriodic(1000);
}
