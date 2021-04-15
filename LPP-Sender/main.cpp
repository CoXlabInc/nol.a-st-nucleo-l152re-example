#include <cox.h>
#include <LPPMac.hpp>
#include "SX1276Wiring.hpp"

void send(void *args);
static void sendDone(IEEE802_15_4Mac &radio,
                     IEEE802_15_4Frame *frame);
static void sendTask(void *args);
static void receivedProbe(uint16_t panId,
                          const uint8_t *eui64,
                          uint16_t shortId,
                          int16_t rssi,
                          const uint8_t *payload,
                          uint8_t payloadLen,
                          uint32_t channel);

Timer sendTimer;

uint16_t node_id = 3;
uint8_t node_ext_id[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0, 0};

uint32_t sent = 0;
uint32_t success = 0;

SX1276Wiring SX1276(Spi,
		    A0,  //Reset
		    D10, //CS
		    A4,  //RxTx
		    D2,  //DIO0 (PA10)
		    D3,  //DIO1 (PB3)
		    D4,  //DIO2 (PB5)
		    D5,  //DIO3 (PB4)
		    A3); //DIO4 (PB0)

LPPMac Lpp;

void setup(void) {
  Serial.begin(115200);
  printf("\n*** [ST Nucleo-L152RE] LPP Sender ***\n");
  srand(0x1234 + node_id);

  SX1276.begin();
  SX1276.setRadio(Radio::SF7, Radio::BW_125kHz, Radio::CR_4_5);
  SX1276.setTxPower(14);
  SX1276.setChannel(922100000);

  node_ext_id[6] = highByte(node_id);
  node_ext_id[7] = lowByte(node_id);

  Lpp.begin(SX1276, 0x1234, node_id, node_ext_id);
  Lpp.setProbePeriod(3000);
  Lpp.setListenTimeout(3300);
  Lpp.setTxTimeout(632);
  Lpp.setRxTimeout(465);
  Lpp.setRxWaitTimeout(30);
  //Lpp.setUseSITFirst(true);

  Lpp.onSendDone(sendDone);
  Lpp.onReceiveProbe(receivedProbe);
  Lpp.setProbePayload("test2", 5);

  sendTimer.onFired(sendTask, NULL);
  sendTimer.startPeriodic(10000);
}

static void sendDone(IEEE802_15_4Mac &radio,
                     IEEE802_15_4Frame *frame) {
  uint16_t ratio;

  printf("TX (");

  if (frame->result == RadioPacket::SUCCESS) {
    success++;
    printf("S ");
  } else {
    printf("F ");
  }

  if (sent > 0)
    ratio = success * 100 / sent;
  else
    ratio = 0;

  printf("%u %% (%lu/%lu)) (%02X %02X..) t: %u\n",
         ratio, success, sent,
         frame->getPayloadAt(0),
         frame->getPayloadAt(1),
         frame->txCount);
  delete frame;
}

static void sendTask(void *args) {
  IEEE802_15_4Frame *frame;
  uint8_t n;
  uint16_t dst;
  uint8_t dest_ext_id[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0, 0};
  error_t err;

  for (dst = 1; dst <= 1; dst++) {
    frame = new IEEE802_15_4Frame(100);
    if (!frame) {
      printf("Not enough memory\n");
      return;
    }

  #if 0
    IEEE802_15_4Address dstAddr(dst, 0x1234);
  #else
    dest_ext_id[6] = highByte(dst);
    dest_ext_id[7] = lowByte(dst);
    IEEE802_15_4Address dstAddr(dest_ext_id, 0x1234);
  #endif
    frame->setDstAddr(dstAddr);

    for (n = 2; n < frame->getPayloadLength(); n++) {
      frame->setPayloadAt(n, n);
    }

    frame->setPayloadAt(0, sent >> 8);
    frame->setPayloadAt(1, sent & 0xff);

    //Lpp.useForceNoAckRequest = true; // To unicast without Ack request
    err = Lpp.send(frame);
    if (err != ERROR_SUCCESS) {
      printf("Sending fail.(%d)\n", err);
      delete frame;
    } else {
      printf("Trying to send...\n");
      if (sent == 0) {
        success = 0;
        //while (1);
      }
      sent++;
    }

  }
}

static void receivedProbe(uint16_t panId,
                          const uint8_t *eui64,
                          uint16_t shortId,
                          int16_t rssi,
                          const uint8_t *payload,
                          uint8_t payloadLen,
                          uint32_t channel) {
  printf("* Probe received from PAN:0x%04X, "
        "Node EUI64:%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x, "
        "ID:%x, RSSI:%d",
        panId,
        eui64[0], eui64[1], eui64[2], eui64[3],
        eui64[4], eui64[5], eui64[6], eui64[7],
        shortId, rssi);

  if (payloadLen > 0) {
    uint8_t i;

    printf(", length:%u, ", payloadLen);
    for (i = 0; i < payloadLen; i++) {
      printf("%02X ", payload[i]);
    }
  }

  printf("\n");
}
