#include <cox.h>

SX1272_6Chip *SX1276;
LPPMac *Lpp;
Timer timerReport;

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

static void taskReport(void *) {
  IEEE802_15_4Frame *frame = new IEEE802_15_4Frame(125);
  if (!frame) {
    printf("Not enough memory\n");
    return;
  }

  uint8_t *payload = (uint8_t *) frame->getPayloadPointer();
  frame->dstAddr.len = 2;
  frame->dstAddr.id.s16 = 1;
  frame->setPayloadLength(snprintf((char *) payload, 125, "\"PM1_0\":\"%ld\",\"PM2_5\":\"%ld\",\"PM10_0\":\"%ld\"",
                        pm1_0_Atmosphere, pm2_5_Atmosphere, pm10_0_Atmosphere));
  printf("* Report: %s\n", (const char *) payload);
  Lpp->send(frame);
}

static void eventSendDone(IEEE802_15_4Mac &radio, IEEE802_15_4Frame *frame, error_t result) {
  printf("* Send done: ");

  if (result == ERROR_SUCCESS) {
    printf("SUCCESS");
  } else {
    printf("FAIL");
  }

  const uint8_t *payload = (const uint8_t *) frame->getPayloadPointer();
  printf(" (%02X %02X..) t: %u\n",
         payload[0],
         payload[1],
         frame->txCount);
  delete frame;
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

  timerReport.onFired(taskReport, NULL);
  timerReport.startPeriodic(10000);

  SX1276 = System.attachSX1276MB1LASModule();
  SX1276->begin();
  SX1276->setDataRate(7);
  SX1276->setCodingRate(1);
  SX1276->setTxPower(20);
  SX1276->setChannel(917300000);

  Lpp = LPPMac::Create();
  Lpp->begin(*SX1276, 0x1234, 0x0002, NULL);
  Lpp->setProbePeriod(3000);
  Lpp->setListenTimeout(3300);
  Lpp->setTxTimeout(632);
  Lpp->setRxTimeout(465);
  Lpp->setRxWaitTimeout(30);
  Lpp->setUseSITFirst(true);

  Lpp->onSendDone(eventSendDone);
}
