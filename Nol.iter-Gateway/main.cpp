#include <cox.h>

IPv6Interface *ppp;
Timer ledTimer;
bool booted = false;

SX1272_6Chip *SX1276;
LPPMac *Lpp;
NoliterAPI &Noliter = enableNoliterLite();

static void eventGatewaySetupDone() {
  booted = true;
  printf("Nol.iter setup done.\n");
}

static void ip6_state_changed(IPv6Interface &interface, IPv6Interface::State_t state) {
  printf("IPv6 iface 'ppp0': State changed to %s\n",
         ip6_state_string(state));

  if (state == IPv6Interface::STATE_HOST && !booted) {
    printf("Nol.iter start!\n");
    Noliter.onReady(eventGatewaySetupDone);
    Noliter.setGateway(interface);
  }
}

static void eventLppFrameReceived(IEEE802_15_4Mac &radio,
                                  const IEEE802_15_4Frame *frame) {
  char id[24];

  if (frame->srcAddr.len == 2) {
    sprintf(id, "N%u", frame->srcAddr.id.s16);
  } else if (frame->srcAddr.len == 8) {
    sprintf(id, "N%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x",
            frame->srcAddr.id.s64[0],
            frame->srcAddr.id.s64[1],
            frame->srcAddr.id.s64[2],
            frame->srcAddr.id.s64[3],
            frame->srcAddr.id.s64[4],
            frame->srcAddr.id.s64[5],
            frame->srcAddr.id.s64[6],
            frame->srcAddr.id.s64[7]);
  }

  const uint8_t *payload = (const uint8_t *) frame->getPayloadPointer();
  printf("* LPP RX: %s,RSSI(%d),%02x %02x~ (length:%u)\n",
          id,
          frame->power,
          payload[0],
          payload[1],
          frame->getPayloadLength());

  if (booted) {
    char *data = (char *) dynamicMalloc(frame->getPayloadLength() + 1);
    if (data) {
      memcpy(data, payload, frame->getPayloadLength());
      data[frame->getPayloadLength()] = '\0';
      Noliter.send(id, data);
      dynamicFree(data);
    } else {
      printf("* Not enough memory\n");
    }
  }
}

void setup(void) {
  Serial.begin(115200);
  printf("\n\n*** [ST Nucleo-L152RE] Nol.iter Gateway ***\n");

  /* Single interface, no routing entry. */
  ip6_init(1, 0);

  /* Initialize the PPP interface. */
  Serial2.begin(115200);
  Serial2.listen();

  ppp = enableIPv6PPPoS(Serial2);
  if (ppp) {
    ppp->begin();
    ppp->setStateNotifier(ip6_state_changed);
    ip6_start();
  } else {
    printf("* Error on enable PPPoS.\n");
  }

  SX1276 = System.attachSX1276MB1LASModule();
  SX1276->begin();
  SX1276->setDataRate(Radio::SF7);
  SX1276->setCodingRate(Radio::CR_4_5);
  SX1276->setTxPower(20);
  SX1276->setChannel(917300000);

  Lpp = LPPMac::Create();
  Lpp->begin(*SX1276, 0x1234, 0x0001, NULL);
  Lpp->setProbePeriod(3000);
  Lpp->setListenTimeout(3300);
  Lpp->setTxTimeout(632);
  Lpp->setRxTimeout(465);
  Lpp->setRxWaitTimeout(30);
  Lpp->setRadioAlwaysOn(true);

  Lpp->onReceive(eventLppFrameReceived);
}
