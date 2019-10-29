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
int ClockClient::interval{0};
String ClockClient::clock{""};
String ClockClient::weekday{""};
int ClockClient::clockHours{0};
int ClockClient::clockMinutes{0};
int ClockClient::clockSeconds{0};
int ClockClient::numClockChangeCallbacks{0};
ClockChangeCallback ClockClient::clockChangeCallback[];

String ClockClient::checkName{""}, ClockClient::_text{""}, ClockClient::_clocktype{""}, ClockClient::_clock{""}, ClockClient::_weekday{""};
float ClockClient::_speed=0;
boolean ClockClient::_active;
int ClockClient::_clockHours=0, ClockClient::_clockMinutes=0, ClockClient::_clockSeconds=0, ClockClient::_interval=0;

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

void ClockClient::outLine(uint8_t *buff, int len) {
  // print buffer line
  buff[len] = 0;
  debug.out((char *) buff);
  debug.out("  ");
  /*
  for (int j=0; j<len; ++j) {
    debug.out((int) buff[j]);
    debug.out(" ");
  }
  */
  debug.outHex8(buff, len);
  debug.outln("");
}

void ClockClient::dumpReceivedMessage(String msg) {
  debug.outln("Message dump:");
  int len = msg.length();
  uint8_t charBuffer[17];
  int bufferLen=0;

  int i=0;
  while (i < len) {
    charBuffer[bufferLen++] = msg[i];
    ++i;
    if (bufferLen >= 16) {
      outLine(charBuffer, bufferLen);
      bufferLen=0;
    }
  }
  outLine(charBuffer, bufferLen);
}


//void ClockClient::interpretClockMessageLine(const String msg) {
void ClockClient::interpretClockMessageLine(const char * _msg) {
  String msg = String(_msg);
  String value;
  /*
  if (!msg.startsWith("fastclock\r\n")) {
      debug.out(F("ERROR: This is not a fastclock message! Got message="));
      debug.outln(msg.substring(0,30));
      dumpReceivedMessage(msg);
      return;
  }
  */

  //debug.out(">msg>"); debug.out(msg);
  if (msg.startsWith("version=")) {
    value = msg.substring(msg.indexOf('=')+1, msg.indexOf('\r'));
    if (value != "2") {
      debug.out(F("WARNING: Version of fastclock not supported! Got "));
      debug.out(_msg);
      debug.outln(F(". Trying to understand what I can ..."));
    }
  } else if (msg.startsWith("name=")) {
    checkName = String(msg.substring(msg.indexOf('=')+1, msg.indexOf('\r')));
    fastclockScanner.addClock(checkName);
    debug.out(F(">name>")); debug.outln(checkName);
  } else if (msg.startsWith("ip-address=")) {
    // ignore this
  } else if (msg.startsWith("ip-port=")) {
    // ignore this
  } else if (msg.startsWith("text=")) {
    _text = String(msg.substring(msg.indexOf('=')+1, msg.indexOf('\r')));
  } else if (msg.startsWith("clocktype=")) {
    _clocktype = String(msg.substring(msg.indexOf('=')+1, msg.indexOf('\r')));
  } else if (msg.startsWith("active=")) {
    if (msg.startsWith("active=yes\r\n")) {
      _active = true;
    } else {
      _active = false;
    }
  } else if (msg.startsWith("speed=")) {
    _speed = msg.substring(msg.indexOf('=')+1, msg.indexOf('\r')).toFloat();
  } else if (msg.startsWith("clock=")) {
    _clock = String(msg.substring(msg.indexOf('=')+1, msg.indexOf('\r')));
    int firstColonPos = _clock.indexOf(':');
    int secondColonPos = _clock.lastIndexOf(':');
    debug.out("clock="); debug.out(_clock); debug.out("  ");
    debug.out(firstColonPos); debug.out(" / "); debug.outln(secondColonPos);
    _clockHours = _clock.substring(0,firstColonPos).toInt();
    _clockMinutes = _clock.substring(firstColonPos+1, secondColonPos).toInt();
    _clockSeconds = _clock.substring(secondColonPos+1).toInt();
  } else if (msg.startsWith("interval=")) {
    _interval = msg.substring(msg.indexOf('=')+1, msg.indexOf('\r')).toInt();
  } else if (msg.startsWith("weekday=")) {
    _weekday = msg.substring(msg.indexOf('=')+1, msg.indexOf('\r'));
  } else if (msg.startsWith("\r\n") || msg.startsWith("\n")) {
    // ignore empty lines
  } else {
    debug.out(F("ERROR: Invalid clock message format! Ignoring: "));
    debug.outln(msg);
    //dumpReceivedMessage(msg);
  }
}

void ClockClient::interpretClockMessage(const char *_msg) {
  #define MAX_LINE_LEN 255
  static char lineBuffer[MAX_LINE_LEN+1];
  static char *lineBufferPointer;
  char currentChar;

  //debug.out(">>> Msg len="); debug.out(strlen(_msg)); debug.outln(" bytes");
  //debug.out(">>>>"); debug.outln(_msg);
  lineBufferPointer = &lineBuffer[0];
  while (*_msg != 0) {
    currentChar = *_msg++;
    *lineBufferPointer++ = currentChar;
    if (currentChar == '\n') {
      *lineBufferPointer = 0;
      //debug.out(">>"); debug.outln(lineBuffer);
      interpretClockMessageLine(lineBuffer);
      lineBufferPointer = &lineBuffer[0];
      yield();
    }
  }

  if (!checkName.equals(name)) {
    // this is another clock, we are not following this one
    debug.out(F("NOT Ignoring clock with name="), DEBUG_MAX_INFO);
    debug.out(checkName.c_str(), DEBUG_MAX_INFO);
    debug.out(F("; looking for "), DEBUG_MAX_INFO);
    debug.outln(name.c_str(), DEBUG_MAX_INFO);
    return;
  }

  text = _text;
  clocktype = _clocktype;
  clock = _clock;
  weekday = _weekday;
  speed =_speed;
  active = _active;
  clockHours = _clockHours;
  clockMinutes = _clockMinutes;
  clockSeconds = _clockSeconds;
  interval = _interval;

  debug.out(F("Got clock h:m:s  "));
  debug.out(clockHours); debug.out(":");
  debug.out(clockMinutes); debug.out(":");
  debug.out(clockSeconds);
  debug.out(F(" active=")); debug.out(_active ? "y" : "n");
  debug.out(F("  text=")); debug.outln(text);

  for (int i=0; i<numClockChangeCallbacks; ++i) {
    clockChangeCallback[i](clockHours, clockMinutes, clockSeconds);
  }

  // Debug::out(F("Clock Name=")); Debug::out(ClockClient::name.c_str()); Debug::out(F(", ")); Debug::outln(ClockClient::clock.c_str());
}

void ClockClient::loop() {
  int length = 0;

  length = udp.parsePacket();
  debug.out(">UDP>"); debug.out(length); debug.outln(" bytes");
  if (length > 0) {
    // debug.out(F("ClockClient received: ")); debug.out(length); debug.outln(F(" bytes."));
    udp.read(packetBuffer, CLOCK_PACKET_SIZE);
    packetBuffer[length] = '\0';
    debug.outln(">UDP-msg>"); debug.out((char *) packetBuffer);
    interpretClockMessage((char *) packetBuffer);
    // debug.out(F("> ")); debug.outln( (char *) packetBuffer);
  }
}
