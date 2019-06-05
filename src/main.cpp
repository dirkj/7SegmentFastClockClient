#include <Arduino.h>
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <NTPClient.h>        //https://github.com/esp8266/Arduino
#include <WiFiUdp.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <ArduinoJson.h>
#include <SPI.h>
#include "SevenSegmentClock.h"

// NTP
WiFiUDP ntpUDP;
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200, 60000);

#define MODE_DEMO 1
#define MODE_REALCLOCK 2
#define MODE_FASTCLOCK 3
static int appMode = MODE_REALCLOCK;



static const char *appName = "FastclockClient7Seg";

#define MAX_CLOCK_NAME_LEN 16
#define MAX_CLOCK_CHANNEL_STRING_LEN 3
#define MAX_CLOCK_COLOR_LEN 16
#define DEFAULT_CLOCK_NAME "fastclk"
#define DEFAULT_CLOCK_CHANNEL_STRING "1"
#define DEFAULT_CLOCK_CHANNEL 1
#define DEFAULT_CLOCK_COLOR "green"

SevenSegmentClock sevenSegmentClock;
ESP8266WebServer *server;

char static_ip[16] = "10.0.1.56";
char static_gw[16] = "10.0.1.1";
char static_sn[16] = "255.255.255.0";

static struct ColorSelection {
  uint8_t id;
  SevenSegmentClock::Color colorHandle;
  String colorName;
} colorSelection[] = {
  { 1, SevenSegmentClock::Black, "black" },
  { 2, SevenSegmentClock::Blue, "blue" },
  { 3, SevenSegmentClock::Red, "red" },
  { 4, SevenSegmentClock::Green, "green" },
  { 5, SevenSegmentClock::White, "white" },
  { 6, SevenSegmentClock::Yellow, "yellow" }
};

static const String getColorName(uint8_t color) {
  for (unsigned int i=0; i<sizeof(colorSelection); ++i) {
    if (color == colorSelection[i].id) {
      return colorSelection[i].colorName;
    }
  }
  return "**INVALID**";
}

static const uint8_t getColorId(SevenSegmentClock::Color color) {
  for (unsigned int i=0; i<sizeof(colorSelection); ++i) {
    if (color == colorSelection[i].colorHandle) {
      return colorSelection[i].id;
    }
  }
  return -1;
}

static const SevenSegmentClock::Color getColorHandle(uint8_t id) {
  for (unsigned int i=0; i<sizeof(colorSelection); ++i) {
    if (id == colorSelection[i].id) {
      return colorSelection[i].colorHandle;
    }
  }
  return SevenSegmentClock::Green; // default
}

static const SevenSegmentClock::Color getColorHandleByName(String name) {
  for (unsigned int i=0; i<sizeof(colorSelection); ++i) {
    if (name.equals(colorSelection[i].colorName)) {
      return colorSelection[i].colorHandle;
    }
  }
  return SevenSegmentClock::Green; // default
}

#define DEFAULT_COLOR SevenSegmentClock::Green
#define DEFAULT_BRIGHTNESS 31

char clockName[MAX_CLOCK_NAME_LEN+1] = DEFAULT_CLOCK_NAME;
char clockChannelString[MAX_CLOCK_CHANNEL_STRING_LEN+1] = DEFAULT_CLOCK_CHANNEL_STRING;
uint8_t clockChannel = DEFAULT_CLOCK_CHANNEL;
SevenSegmentClock::Color clockColor = DEFAULT_COLOR;
//uint8_t brightness = DEFAULT_BRIGHTNESS;
int utcTimeOffsetMinutes = 120; // default UTC+2h

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void setupWifiConnection() {
  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(300);

  //set static ip
  IPAddress _ip,_gw,_sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);

  //wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);

  //add all your parameters here
  //**wifiManager.addParameter(&custom_mqtt_server);
  //**wifiManager.addParameter(&custom_mqtt_port);
  //wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality(15);

  Serial.println("Starting autoConnect ...");
  //if (!wifiManager.autoConnect("FastclockClient7Seg", "password")) {
  //if (!wifiManager.autoConnect("fc7seg", "password")) {
  if (!wifiManager.autoConnect("fc7seg")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonDocument config(2048);
    //JsonObject json = jsonBuffer.createObject();
    config["clock_name"] = clockName;
    config["clock_channel"] = clockChannelString;
    config["clock_color"] = clockColor;

/*
    json["ip"] = WiFi.localIP().toString();
    json["gateway"] = WiFi.gatewayIP().toString();
    json["subnet"] = WiFi.subnetMask().toString();
*/

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    serializeJsonPretty(config, Serial);
    serializeJson(config, configFile);
    configFile.close();
    //end save
  }

  Serial.print("local ip: "); Serial.println(WiFi.localIP());
  Serial.print("gateway: "); Serial.println(WiFi.gatewayIP());
  Serial.print("subnet: "); Serial.println(WiFi.subnetMask());
}

const char _HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char _STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} input.r{width:20%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char _SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char _HEAD_END[] PROGMEM        = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char _PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
const char _ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char _FORM_START[] PROGMEM      = "<form method='get' action='configsave'>";
const char _FORM_CLOCKNAME[] PROGMEM  = "<label for='n'>Fastclock name</label><input id='n' name='n' length=32 placeholder='clock name'><br/>";
const char _FORM_CLOCKMODE_HEADLINE[] PROGMEM = "<br/>Clock mode:<br/>";
const char _FORM_CLOCKMODE_DEMO[] PROGMEM = "<input class='r' id='md' name='m' type='radio' value='demo' {check}><label for='md'>Demo</label><br/>";
const char _FORM_CLOCKMODE_REAL[] PROGMEM = "<input class='r' id='mr' name='m' type='radio' value='real' {check}><label for='md'>Real Clock</label><br/>";
const char _FORM_CLOCKMODE_FAST[] PROGMEM = "<input class='r' id='mf' name='m' type='radio' value='fast' {check}><label for='md'>Fast Clock</label><br/>";
const char _FORM_UTC_OFFSET[] PROGMEM = "<label for='utc'>UTC offset (minutes)</label><input id='utc' name='utc' length=4 placeholder='120'><br/>";
const char _FORM_PARAM[] PROGMEM      = "<br/><input id='{i}' name='{n}' maxlength={l} placeholder='{p}' value='{v}' {c}>";
const char _FORM_COLOR_HEADLINE[] PROGMEM = "<br/>Display color:<br/>";
const char _FORM_COLOR_BLUE[] PROGMEM = "<input class='r' id='cb' name='c' type='radio' value='blue' {check}><label for='cb'>Blue</label><br/>";
const char _FORM_COLOR_RED[] PROGMEM  = "<input class='r' id='cr' name='c' type='radio' value='red' {check}><label for='cr'>Red</label><br/>";
const char _FORM_COLOR_GREEN[] PROGMEM = "<input class='r' id='cg' name='c' type='radio' value='green' {check}><label for='cg'>Green</label><br/>";
const char _FORM_COLOR_WHITE[] PROGMEM = "<input class='r' id='cw' name='c' type='radio' value='white' {check}><label for='cw'>White</label><br/>";
const char _FORM_COLOR_YELLOW[] PROGMEM = "<input class='r' id='cy' name='c' type='radio' value='yellow' {check}><label for='cy'>Yellow</label><br/>";
const char _FORM_BRIGHTNESS[] PROGMEM = "<br/><label for='b'>Brightness:</label><input id='b' name='b' type='range' min='10' max='255' value='{bright}'><br/>";
const char _FORM_END[] PROGMEM        = "<br/><button type='submit'>save</button></form>";
const char _SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char _SAVED[] PROGMEM           = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
const char _END[] PROGMEM             = "</div></body></html>";

void appConfig() {
  String page = FPSTR(_HEAD);
  String input;
  String value;

  page.replace("{v}", "7Seg Config");
  page += FPSTR(_SCRIPT);
  page += FPSTR(_STYLE);
  //page += _customHeadElement;
  page += FPSTR(_HEAD_END);
  page += String(F("<h1>"));
  page += appName;
  page += String(F("</h1>"));
  page += String(F("<h3>Clock Options</h3>"));
  //page += FPSTR(_PORTAL_OPTIONS);
  page += FPSTR(_FORM_START);
  page += FPSTR(_FORM_CLOCKMODE_HEADLINE);
  input = FPSTR(_FORM_CLOCKMODE_DEMO);
  input.replace("{check}", (appMode == MODE_DEMO) ? "checked" : "");
  page += input;
  input = FPSTR(_FORM_CLOCKMODE_REAL);
  input.replace("{check}", (appMode == MODE_REALCLOCK) ? "checked" : "");
  page += input;
  input = FPSTR(_FORM_CLOCKMODE_FAST);
  input.replace("{check}", (appMode == MODE_FASTCLOCK) ? "checked" : "");
  page += input;
  page += FPSTR(_FORM_UTC_OFFSET);
  page += FPSTR(_FORM_CLOCKNAME);
  page += FPSTR(_FORM_COLOR_HEADLINE);
  input = FPSTR(_FORM_COLOR_BLUE);
  input.replace("{check}", (clockColor == SevenSegmentClock::Blue) ? "checked" : "");
  page += input;
  input = FPSTR(_FORM_COLOR_RED);
  input.replace("{check}", (clockColor == SevenSegmentClock::Red) ? "checked" : "");
  page += input;
  input = FPSTR(_FORM_COLOR_GREEN);
  input.replace("{check}", (clockColor == SevenSegmentClock::Green) ? "checked" : "");
  page += input;
  input = FPSTR(_FORM_COLOR_YELLOW);
  input.replace("{check}", (clockColor == SevenSegmentClock::Yellow) ? "checked" : "");
  page += input;
  input = FPSTR(_FORM_COLOR_WHITE);
  input.replace("{check}", (clockColor == SevenSegmentClock::White) ? "checked" : "");
  page += input;
  input = FPSTR(_FORM_BRIGHTNESS);
  value = String(sevenSegmentClock.getBrightness());
  input.replace("{bright}", value);
  page += input;

  page += FPSTR(_FORM_END);
  page += FPSTR(_END);

  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);
}

void appConfigSave() {
  String page = FPSTR(_HEAD);

  page.replace("{v}", "7Seg Config");
  page += FPSTR(_SCRIPT);
  page += FPSTR(_STYLE);
  page += FPSTR(_HEAD_END);
  page += String(F("<h1>"));
  page += appName;
  page += String(F("</h1>"));

  Serial.print("appConfigSave "); Serial.print(server->args()); Serial.println(" arguments");
  for (int i=0; i<server->args(); ++i) {
    Serial.print(server->argName(i));
    Serial.print(": ");
    Serial.println(server->arg(i));
  }
  if (server->hasArg("b")) {
    sevenSegmentClock.setBrightness(server->arg("b").toInt());
    page += F("<div>Set brightness to ");
    page += server->arg("b");
    page += F(".</div>");
  }
  if (server->hasArg("c")) {
    String colorName = server->arg("c");
    SevenSegmentClock::Color colorHandle = getColorHandleByName(server->arg("c"));
    sevenSegmentClock.setColor(colorHandle);
    page += F("<div>Set color to ");
    page += server->arg("c");
    page += F(".</div>");
  }
  if (server->hasArg("m")) {
    Serial.print("setting clock mode to "); Serial.println(server->arg("m"));
    page += F("<div>Set clock mode to ");
    page += server->arg("m");
    page += F(".</div>");
    if (server->arg("m").equals("real")) appMode = MODE_REALCLOCK;
    else if (server->arg("m").equals("fast")) appMode = MODE_FASTCLOCK;
    else if (server->arg("m").equals("demo")) appMode = MODE_DEMO;
    else {
      Serial.println("ERROR: Unknown application mode, going into demo mode");
      appMode = MODE_DEMO;
      page += F("<div>ERROR: Unknown clockmode, using default: demo.</div>");
    }
  }
  if (server->hasArg("utc")) {
    page += F("<div>Set real clock offset to ");
    if (server->arg("utc").equals("")) {
      page += "120";
      utcTimeOffsetMinutes = 120;
    } else {
      page += server->arg("utc");
      utcTimeOffsetMinutes = server->arg("utc").toInt();
    }
    timeClient.setTimeOffset(utcTimeOffsetMinutes);
    page += F(" minutes.</div>");
  }
  page += String(F("<div>Configuration updated.</div>"));
  page += FPSTR(_END);
  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);
}

void setup() {
  Serial.begin(115200);
  Serial.println("---");
  Serial.print("Starting *** "); Serial.println(appName);
  Serial.print("Reset reason: ");
  Serial.println(ESP.getResetReason());

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument config(2048);
        //JsonObject json = jsonBuffer.createObject();
        DeserializationError error = deserializeJson(config, configFile);
        serializeJson(config, Serial);
        if (!error) {
          Serial.println("\nparsed json");

          //**strcpy(mqtt_server, json["mqtt_server"]);
          //**strcpy(mqtt_port, json["mqtt_port"]);
          //strcpy(blynk_token, json["blynk_token"]);

          if (config["clock_name"]) {
            strncpy(clockName, config["clock_name"], MAX_CLOCK_NAME_LEN);
          } else {
            Serial.println("no clock name in config");
          }
          if (config["clock_channel"]) {
            strncpy(clockChannelString, config["clock_channel"], MAX_CLOCK_CHANNEL_STRING_LEN);
          } else {
            Serial.println("no clock channel in config");
          }
          if (config["clock_color"]) {
            //strncpy(clockColor, config["clock_color"], MAX_CLOCK_COLOR_LEN);
            clockColor = getColorHandle(config["clock_color"]);
          } else {
            Serial.println("no clock color in config");
          }
#if 0
          if (json["ip"]) {
            Serial.print("setting custom ip from config: ");
            //**strcpy(static_ip, json["ip"]);
            //**strcpy(static_gw, json["gateway"]);
            //**strcpy(static_sn, json["subnet"]);
            Serial.println(static_ip);
/*            Serial.println("converting ip");
            IPAddress ip = ipFromCharArray(static_ip);
            Serial.println(ip);*/
          } else {
            Serial.println("no custom ip in config");
          }
#endif
        } else {
          Serial.println("failed to load json config, using defaults");
          strncpy(clockName, DEFAULT_CLOCK_NAME, MAX_CLOCK_NAME_LEN);
          strncpy(clockChannelString, DEFAULT_CLOCK_CHANNEL_STRING, MAX_CLOCK_CHANNEL_STRING_LEN);
          //strncpy(clockColor, DEFAULT_CLOCK_COLOR, MAX_CLOCK_COLOR_LEN);
          clockColor = getColorHandleByName(DEFAULT_CLOCK_COLOR);
        }
      }
    } else {
      Serial.println("no config file found");
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
  Serial.print("static ip: "); Serial.println(static_ip);
  // setupWifiConnection();

  /*
  radio.setClockChannel(clockChannel);
  radio.setClockName(clockName);
  radio.begin();
  fastclock.begin();
  pinMode(POWER_OFF_PIN, INPUT);
  */
  setupWifiConnection();
  Serial.println("Starting NTP Client");
  timeClient.begin();

  Serial.println("Have following configuration:");
  Serial.print("   Clock name: "); Serial.println(clockName);
  Serial.print("   Clock channel: "); Serial.println(clockChannelString);
  Serial.print("   Clock color: "); Serial.println(getColorName(clockColor));

  Serial.println("Starting 7-segment clock display ...");
  sevenSegmentClock.begin();

  // setting up web server for clock configuration
  server = new ESP8266WebServer(80);
  server->on("/config", HTTP_GET, appConfig);
  server->on("/configsave", HTTP_GET, appConfigSave);
  server->begin();
}

int hours = 0, minutes = 0;
uint32_t nextUpdate_ms = 0;

void loop() {
  timeClient.update();

  //Serial.println(timeClient.getFormattedTime());

  switch (appMode) {
    case MODE_DEMO:
      if (millis() > nextUpdate_ms) {
        nextUpdate_ms = millis() + 1000;
        minutes++;
        if (minutes > 99) { minutes = 0; }
        if (minutes % 5 == 0) hours++;
        if (hours > 99) hours = 0;
        sevenSegmentClock.displayTime(hours, minutes);
        if (hours % 4 == 0) sevenSegmentClock.setBlinkMode(SevenSegmentClock::SeperatorBlinking); else sevenSegmentClock.setBlinkMode(SevenSegmentClock::NoBlinking);
      }
      break;
    case MODE_REALCLOCK:
      sevenSegmentClock.displayTime(timeClient.getHours(), timeClient.getMinutes());
      break;
    case MODE_FASTCLOCK:
      break;
  }

  sevenSegmentClock.displayUpdate();
  server->handleClient();
}
