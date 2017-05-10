#include <cox.h>
#include <LPPMac.hpp>

#define SEND_TO_NOLITER

Timer tSense;
LPPMac *Lpp;
SX127xChip *SX1276;

static void taskSense(void *) {
  uint8_t read;

  read = Wire.requestFrom(0x4F, 1);
  printf("Request 1 byte from 0x4F: %u byte read.\n", read);

  while (Wire.available()) {
    printf("%02X ", Wire.read());
  }
  printf("\n");

  read = Wire.requestFrom(0x4F, 2);
  printf("Request 2 byte from 0x4F: %u byte read.\n", read);

  int16_t temperature = 0;

  while (Wire.available()) {
    uint8_t c = Wire.read();
    printf("%02X ", c);
    temperature = (temperature << 8) | c;
  }
  temperature >>= 7;
  printf("=> %d celcius degree\n", temperature / 2);

#ifdef SEND_TO_NOLITER
  IEEE802_15_4Frame *frame = new IEEE802_15_4Frame(125);
  if (!frame) {
    printf("Not enough memory\n");
    return;
  }

  frame->dstAddr.pan.len = 2;
  frame->dstAddr.pan.id = 0x1234;
  frame->dstAddr.len = 2;
  frame->dstAddr.id.s16 = 1;
  frame->setPayloadLength(snprintf((char *) frame->getPayloadPointer(), 100, "\"temp\":\"%d\"", temperature / 2));
  printf("* Report: %s\n", (const char *) frame->getPayloadPointer());
  Lpp->send(frame);
}

static void eventSendDone(IEEE802_15_4Mac &radio,
                          IEEE802_15_4Frame *frame) {
  printf("* Send done: %s, t: %u\n", (frame->result == RadioPacket::SUCCESS) ? "SUCCESS" : "FAIL", frame->txCount);
  delete frame;
}
#endif //SEND_TO_NOLITER

void setup(void) {
  Serial.begin(115200);
  printf("\n*** [ST Nucleo-L152RE] I2C Test with AT30TSE758 Temperature Sensor ***\n");

  Wire.begin();

#ifdef SEND_TO_NOLITER
  SX1276 = System.attachSX1276MB1LASModule();
  SX1276->begin();
  SX1276->setDataRate(Radio::SF7);
  SX1276->setCodingRate(Radio::CR_4_5);
  SX1276->setTxPower(20);
  SX1276->setChannel(917300000);

  Lpp = new LPPMac();
  Lpp->begin(*SX1276, 0x1234, 0x0004, NULL);
  Lpp->setProbePeriod(3000);
  Lpp->setListenTimeout(3300);
  Lpp->setTxTimeout(632);
  Lpp->setRxWaitTimeout(25);
  Lpp->setRxTimeout(465);
  Lpp->setUseSITFirst(true);

  Lpp->onSendDone(eventSendDone);
#endif //SEND_TO_NOLITER

  tSense.onFired(taskSense, NULL);
  tSense.startPeriodic(10000);
}
