#include <cox.h>

SerialPort &Serial2 = enableSerialUSART3();
IPv6Interface *ppp;
Timer ledTimer;
bool booted = false;

SX1272_6Chip &SX1276 = attachSX1276MB1LASModule();
LPPMac &Lpp = getLPPInstance();
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

static void eventLppFrameReceived(PacketRadio &radio,
                                  const struct wpan_frame *frame) {
  char id[24];

  if (frame->meta.ieee802_15_4.addr.src.len == 2) {
    sprintf(id, "N%u", frame->meta.ieee802_15_4.addr.src.id.s16);
  } else if (frame->meta.ieee802_15_4.addr.src.len == 8) {
    sprintf(id, "N%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x",
            frame->meta.ieee802_15_4.addr.src.id.s64[0],
            frame->meta.ieee802_15_4.addr.src.id.s64[1],
            frame->meta.ieee802_15_4.addr.src.id.s64[2],
            frame->meta.ieee802_15_4.addr.src.id.s64[3],
            frame->meta.ieee802_15_4.addr.src.id.s64[4],
            frame->meta.ieee802_15_4.addr.src.id.s64[5],
            frame->meta.ieee802_15_4.addr.src.id.s64[6],
            frame->meta.ieee802_15_4.addr.src.id.s64[7]);
  }
  printf("* LPP RX: %s,RSSI(%d),LQI(%d),MHR(%u),%02x %02x~ (length:%u)\n",
          id,
          frame->rssi,
          frame->meta.ieee802_15_4.lqi,
          frame->mhr_len,
          frame->buf[frame->mhr_len + 0],
          frame->buf[frame->mhr_len + 1],
          frame->len - frame->mhr_len);

  if (booted) {
    char *data = (char *) dynamicMalloc(frame->len - frame->mhr_len + 1);
    if (data) {
      memcpy(data, &frame->buf[frame->mhr_len], frame->len - frame->mhr_len);
      data[frame->len - frame->mhr_len] = '\0';
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

  SX1276.begin();
  SX1276.setDataRate(7);
  SX1276.setCodingRate(1);
  SX1276.setTxPower(20);
  SX1276.setChannel(917300000);

  Lpp.begin(SX1276, 0x1234, 0x0001, NULL);
  Lpp.setProbePeriod(3000);
  Lpp.setListenTimeout(3300);
  Lpp.setTxTimeout(632);
  Lpp.setRxWaitTimeout(25);
  Lpp.setRxTimeout(465);
  Lpp.setRadioAlwaysOn(true);

  Lpp.onReceive(eventLppFrameReceived);
}
