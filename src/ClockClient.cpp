//
//    FILE: ClockClient.cpp
// PURPOSE: UDP broadcast listener for fastclock (FREMO clock)
//
//

#include "ClockClient.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

//const char * const PROGMEM clockConfig[] = {"ipMulticast:string:239.50.50.20", "ipInterface:string:192.168.0.100", "listenPort:int:2000", "interpolate:boolean:true"};
//const char * const PROGMEM clockConfig[] = {"ipMulticast:string:239.50.50.20", "ipInterface:string:127.0.0.1", "listenPort:int:2000", "listenToClock:string:MRclock#2"};
const char * const PROGMEM clockConfig[] = {
  "ipMulticast:string:239.50.50.20",
  "ipInterface:string:127.0.0.1",
  "listenPort:int:2000",
  "listenToClock:string:DefaultClock"
};

static WiFiUDP udp;

#define CLOCK_PACKET_SIZE 1024
static byte packetBuffer[CLOCK_PACKET_SIZE+1]; //buffer to hold incoming and outgoing packets

const char * const ClockClient::getLastMessage() { return (const char *) packetBuffer; }

String ClockClient::name{""};
String ClockClient::text{""};
String ClockClient::clocktype{""};
boolean ClockClient::active{false};
float ClockClient::speed{1.0};
String ClockClient::clock{""};
String ClockClient::weekday{""};
int ClockClient::clockHours{0};
int ClockClient::clockMinutes{0};
int ClockClient::clockSeconds{0};
int ClockClient::numClockChangeCallbacks{0};
ClockChangeCallback ClockClient::clockChangeCallback[];

void ClockClient::addClockChangeCallback(ClockChangeCallback _clockChangeCallback) {
  if (numClockChangeCallbacks >= MAX_CLOCK_CHANGE_CALLBACKS) {
    Debug::outln(F("ERROR: Too many clock change callbacks registered!"));
    return;
  }
  clockChangeCallback[numClockChangeCallbacks++] = _clockChangeCallback;
}


static IPAddress interfaceAddr;
static IPAddress multicast;
static uint16_t port = 2000;

IPAddress ClockClient::getMulticastIP() { return multicast; }

int ClockClient::getListenPort() { return port; }

void ClockClient::begin() {
  debug.outln("Beginning fastclock client", DEBUG_MAX_INFO);
  config.loadFile("clockclient.cfg", clockConfig, sizeof(clockConfig)/sizeof(clockConfig[0]));
  name = config.getString("listenToClock");
  // WiFi.mode(WIFI_STA);
  multicast.fromString(config.getString("ipMulticast"));
  port = config.getInt("listenPort");
  logHeap();

  delay(100);
  if (!udp.beginMulticast(WiFi.localIP(), multicast, port)) {
    debug.outln(F("ERROR: failed to begin UDP"));
  } else {
    debug.outln(F("Successfully started multicast receiver"));
  }
  debug.out(F("interfaceAddr=")); debug.out(WiFi.localIP().toString());
  debug.out(F(", multicast IP=")); debug.out(config.getString("ipMulticast"));
  debug.out(F(", Port=")); debug.outln(port);
}

void ClockClient::interpretClockMessage(const char *_msg) {
  String msg = String(_msg);
  if (!msg.startsWith("fastclock\r\n")) {
      debug.out(F("ERROR: This is not a fastclock message! Got message=")); debug.outln(msg.substring(0,30));
  }
  msg = msg.substring(11);
  if (!msg.startsWith("version=2\r\n")) {
    debug.out(F("WARNING: Version of fastclock not supported! Got ")); debug.outln(msg.substring(0,10));
  }
  msg = msg.substring(msg.indexOf('\n')+1);

  String checkName{""};
  if (msg.startsWith("name=")) {
    checkName = msg.substring(msg.indexOf('=')+1, msg.indexOf('\r'));
    fastclockScanner.addClock(checkName);
    msg = msg.substring(msg.indexOf('\n')+1);
  } else { debug.outln(F("ERROR: Clock Message Format invalid! Expected name field.")); return; }
  if (!checkName.equals(name)) {
    // this is another clock, we are not following this one
    debug.out(F("Ignoring clock with name="), DEBUG_MAX_INFO); debug.out(checkName.c_str(), DEBUG_MAX_INFO); debug.out(F("; looking for "), DEBUG_MAX_INFO); debug.outln(name.c_str(), DEBUG_MAX_INFO);
    return;
  }
  if (msg.startsWith("ip-address=")) {
    msg = msg.substring(msg.indexOf('\n')+1);
  } else { debug.outln(F("ERROR: Clock Message Format invalid! Expected ip-address field.")); return; }
  if (msg.startsWith("ip-port=")) {
    msg = msg.substring(msg.indexOf('\n')+1);
  } else { debug.outln(F("ERROR: Clock Message Format invalid! Expected ip-port field.")); return; }
  if (msg.startsWith("text=")) {
    text = msg.substring(msg.indexOf('=')+1, msg.indexOf('\r'));
    msg = msg.substring(msg.indexOf('\n')+1);
  } else { debug.outln(F("ERROR: Clock Message Format invalid! Expected text field.")); return; }
  if (msg.startsWith("clocktype=")) {
    clocktype = msg.substring(msg.indexOf('=')+1, msg.indexOf('\r'));
    msg = msg.substring(msg.indexOf('\n')+1);
  } else { debug.outln(F("ERROR: Clock Message Format invalid! Expected clocktype field.")); return; }
  if (msg.startsWith("active=")) {
    if (msg.startsWith("active=yes\r\n")) {
      active = true;
    } else {
      active = false;
    }
    msg = msg.substring(msg.indexOf('\n')+1);
  } else { debug.outln(F("ERROR: Clock Message Format invalid! Expected active field.")); return; }
  if (msg.startsWith("speed=")) {
    speed = msg.substring(msg.indexOf('=')+1, msg.indexOf('\r')).toFloat();
    msg = msg.substring(msg.indexOf('\n')+1);
  } else { debug.outln(F("ERROR: Clock Message Format invalid! Expected speed field.")); return; }
  if (msg.startsWith("clock=")) {
    clock = msg.substring(msg.indexOf('=')+1, msg.indexOf('\r'));
    int firstColonPos = clock.indexOf(':');
    int secondColonPos = clock.lastIndexOf(':');
    clockHours = clock.substring(0,firstColonPos).toInt();
    clockMinutes = clock.substring(firstColonPos+1, secondColonPos).toInt();
    clockSeconds = clock.substring(secondColonPos+1).toInt();
    msg = msg.substring(msg.indexOf('\n')+1);
  } else { debug.outln(F("ERROR: Clock Message Format invalid! Expected clock field.")); return; }
  if (msg.startsWith("weekday=")) {
    weekday = msg.substring(msg.indexOf('=')+1, msg.indexOf('\r'));
    msg = msg.substring(msg.indexOf('\n')+1);
  } else { debug.outln(F("ERROR: Clock Message Format invalid! Expected weekday field.")); return; }

  for (int i=0; i<numClockChangeCallbacks; ++i) {
    clockChangeCallback[i](clockHours, clockMinutes, clockSeconds);
  }

  // Debug::out(F("Clock Name=")); Debug::out(ClockClient::name.c_str()); Debug::out(F(", ")); Debug::outln(ClockClient::clock.c_str());
}

void ClockClient::loop() {
  int length = 0;

  length = udp.parsePacket();
  if (length > 0) {
    // debug.out(F("ClockClient received: ")); debug.out(length); debug.outln(F(" bytes."));
    udp.read(packetBuffer, CLOCK_PACKET_SIZE);
    packetBuffer[length] = '\0';
    interpretClockMessage((char *) packetBuffer);
    // debug.out(F("> ")); debug.outln( (char *) packetBuffer);
  }
}
