#include <cox.h>

Timer timerWave;

static void taskWave(void *) {
  static float in = 4.712;
  float out;

  in += 0.1;
  if (in > 10.995) {
    in = 4.712;
  }
  out = sin(in) * 2047.5 + 2047.5;
  printf("sin():%u\n", (uint16_t) out);
  analogWrite(PA4, out);

  out = cos(in) * 2047.5 + 2047.5;
  printf("cos():%u\n", (uint16_t) out);
  analogWrite(PA5, out); //It generates voltage value from 0 to 1.8V due to the user LED.
}

void setup(void) {
  Serial.begin(115200);
  printf("\n*** [ST Nucleo-L152RE] DAC Test ***\n");

  pinMode(PA4, OUTPUT);
  pinMode(PA5, OUTPUT);

  timerWave.onFired(taskWave, NULL);
  timerWave.startPeriodic(100);
}
