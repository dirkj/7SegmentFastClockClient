#include <Arduino.h>
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <ArduinoJson.h>
#include <SPI.h>
#include "SevenSegmentClock.h"

#define USE_CONFIG false

static const char *appName = "FastclockClient7Seg";

#define MAX_CLOCK_NAME_LEN 16
#define DEFAULT_CLOCK_CHANNEL 1

SevenSegmentClock sevenSegmentClock;

char static_ip[16] = "10.0.1.56";
char static_gw[16] = "10.0.1.1";
char static_sn[16] = "255.255.255.0";

char clockName[MAX_CLOCK_NAME_LEN+1] = "fastclk";
uint8_t clockChannel = DEFAULT_CLOCK_CHANNEL;

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
  wifiManager.setMinimumSignalQuality();

  Serial.println("Starting autoConnect ...");
  if (!wifiManager.autoConnect("FastclockClient7Seg", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //save the custom parameters to FS
  #if USE_CONFIG
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonDocument jsonBuffer(2048);
    JsonObject json = jsonBuffer.createObject();
    //**json["mqtt_server"] = mqtt_server;
    //**json["mqtt_port"] = mqtt_port;
    //json["blynk_token"] = blynk_token;

    json["ip"] = WiFi.localIP().toString();
    json["gateway"] = WiFi.gatewayIP().toString();
    json["subnet"] = WiFi.subnetMask().toString();

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    serializeJsonPretty(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
    //end save
  }
  #endif

  Serial.print("local ip: "); Serial.println(WiFi.localIP());
  Serial.print("gateway: "); Serial.println(WiFi.gatewayIP());
  Serial.print("subnet: "); Serial.println(WiFi.subnetMask());
}

void setup() {
  // if coming from deep sleep, we just go to sleep again

  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("Starting *** "); Serial.println(appName);
  Serial.println(ESP.getResetReason());

  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
#if USE_CONFIG
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
        DynamicJsonDocument jsonBuffer(2048);
        JsonObject json = jsonBuffer.createObject();
        DeserializationError error = deserializeJson(jsonBuffer, json);
        serializeJson(json, Serial);
        if (!error) {
          Serial.println("\nparsed json");

          //**strcpy(mqtt_server, json["mqtt_server"]);
          //**strcpy(mqtt_port, json["mqtt_port"]);
          //strcpy(blynk_token, json["blynk_token"]);

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
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
#endif
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
  sevenSegmentClock.begin();
}

int hours = 0, minutes = 0;
uint32_t nextUpdate_ms = 0;

void loop() {
  if (millis() > nextUpdate_ms) {
    nextUpdate_ms = millis() + 1000;
    minutes++;
    if (minutes > 99) { minutes = 0; }
    if (minutes % 5 == 0) hours++;
    if (hours > 99) hours = 0;
    sevenSegmentClock.displayTime(hours, minutes);
    if (hours % 4 == 0) sevenSegmentClock.setBlinkMode(SeperatorBlinking); else sevenSegmentClock.setBlinkMode(NoBlinking);
  }
  sevenSegmentClock.displayUpdate();
}
