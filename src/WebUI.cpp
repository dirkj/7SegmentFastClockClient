#include "WebUI.h"

static const String appName{"7SegDisplay-XL"};
static const char _HEAD[] PROGMEM              = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
static const char _STYLE[] PROGMEM             = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} input.r{width:20%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
static const char _SCRIPT[] PROGMEM            = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
static const char _HEAD_END[] PROGMEM          = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
static const char _PORTAL_OPTIONS[] PROGMEM    = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
static const char _ITEM[] PROGMEM              = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
static const char _FORM_START[] PROGMEM        = "<form method='get' action='configSave'>";
static const char _FORM_CLOCKNAME[] PROGMEM    = "<label for='n'>Fastclock name</label><input id='n' name='n' length=32 placeholder='clock name' value='{n}'><br/>";
static const char _FORM_CLOCKSFOUND_START[] PROGMEM = "Fastclocks seen:<br/><ul>";
static const char _FORM_CLOCKSFOUND_ITEM[] PROGMEM = "<li>{fc}</li>";
static const char _FORM_CLOCKSFOUND_END[] PROGMEM = "</ul><br/>";
static const char _FORM_CLOCKMODE_HEADLINE[] PROGMEM = "<br/>Clock mode:<br/>";
static const char _FORM_CLOCKMODE_DEMO[] PROGMEM = "<input class='r' id='md' name='m' type='radio' value='demo' {check}><label for='md'>Demo</label><br/>";
static const char _FORM_CLOCKMODE_REAL[] PROGMEM = "<input class='r' id='mr' name='m' type='radio' value='real' {check}><label for='md'>Real Clock</label><br/>";
static const char _FORM_CLOCKMODE_FAST[] PROGMEM = "<input class='r' id='mf' name='m' type='radio' value='fast' {check}><label for='md'>Fast Clock</label><br/>";
static const char _FORM_UTC_OFFSET[] PROGMEM   = "<label for='utc'>UTC offset (minutes)</label><input id='utc' name='utc' length=4 placeholder='120'><br/>";
static const char _FORM_PARAM[] PROGMEM        = "<br/><input id='{i}' name='{n}' maxlength={l} placeholder='{p}' value='{v}' {c}>";
static const char _FORM_COLOR_HEADLINE[] PROGMEM = "Display color:<br/>";
static const char _FORM_COLOR_template[] PROGMEM = "<input class='r' id='{cid}' name='c' type='radio' value='{cname}' {check}><label for='{cid}'>{cname}</label><br/>";
static const char _FORM_BRIGHTNESS[] PROGMEM   = "<label for='b'>Brightness:</label><input id='b' name='b' type='range' min='10' max='255' value='{bright}'><br/>";
static const char _FORM_FASTCLOCK_INFO[] PROGMEM = "<div>Number of fastclocks found: {nfc}</div><br/>";
static const char _FORM_END[] PROGMEM          = "<br/><button type='submit'>apply</button></form><br/>";
static const char _SAVE_PERM_BUTTON[] PROGMEM  = "<br/><form action=\"/configSavePermanent\" method=\"get\"><button>Save permanently</button></form><br/>";
static const char _CONFIG_BUTTON[] PROGMEM     = "<br/><form action=\"/config\" method=\"get\"><button>Configure</button></form><br/>";
static const char _VSPACE[] PROGMEM            = "<br/><br/>";
static const char _SAVED[] PROGMEM             = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
static const char _END[] PROGMEM               = "</div></body></html>";

void WebUI::appConfig() {
  String page = FPSTR(_HEAD);
  String input;
  String value;
  int ivalue;

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
  input.replace("{check}", (config.getString("appMode").equals("Demo")) ? "checked" : "");
  page += input;
  input = FPSTR(_FORM_CLOCKMODE_REAL);
  input.replace("{check}", (config.getString("appMode").equals("Realclock")) ? "checked" : "");
  page += input;
  input = FPSTR(_FORM_CLOCKMODE_FAST);
  input.replace("{check}", (config.getString("appMode").equals("Fastclock")) ? "checked" : "");
  page += input;
  page += FPSTR(_VSPACE);
  page += FPSTR(_FORM_UTC_OFFSET);
  page += FPSTR(_VSPACE);
  input = FPSTR(_FORM_CLOCKNAME);
  value = config.getString("listenToClock");
  input.replace("{n}", value);
  page += input;
  page += FPSTR(_FORM_CLOCKSFOUND_START);
  String *knownClocks = fastclock.getKnownClocks();
  for (int i=0; i<fastclock.getNumberOfKnownClocks(); ++i) {
    input = FPSTR(_FORM_CLOCKSFOUND_ITEM);
    input.replace("{fc}", knownClocks[i]);
    page += input;
  }
  page += FPSTR(_FORM_CLOCKSFOUND_END);

  page += FPSTR(_VSPACE);
  page += FPSTR(_FORM_COLOR_HEADLINE);
  for (int i=0; i<sevenSegmentClock.getNumberSupportedColors(); ++i) {
    input = FPSTR(_FORM_COLOR_template);
    input.replace("{cid}", sevenSegmentClock.getColorName(i));
    input.replace("{cname}", sevenSegmentClock.getColorName(i));
    input.replace("{check}", (config.getString("clockColor").equals(sevenSegmentClock.getColorName(i))) ? "checked" : "");
    page += input;
  }

  page += FPSTR(_VSPACE);
  input = FPSTR(_FORM_BRIGHTNESS);
  value = String(sevenSegmentClock.getBrightness());
  input.replace("{bright}", value);
  page += input;

  page += FPSTR(_VSPACE);
  input = FPSTR(_FORM_FASTCLOCK_INFO);
  ivalue = fastclock.getNumberOfKnownClocks();
  value = String(ivalue);
  //value = String("unknown");
  input.replace("{nfc}", value);
  page += input;

  page += FPSTR(_FORM_END);
  page += FPSTR(_END);

  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);
}

void WebUI::appConfigSave(void) {
  String page = FPSTR(_HEAD);

  page.replace("{v}", "7Seg Config");
  page += FPSTR(_SCRIPT);
  page += FPSTR(_STYLE);
  page += FPSTR(_HEAD_END);
  page += String(F("<h1>"));
  page += appName;
  page += String(F("</h1>"));

  debug.out(server->args(), DEBUG_MED_INFO); debug.outln(" arguments", DEBUG_MED_INFO);
  for (int i=0; i<server->args(); ++i) {
    debug.out(server->argName(i), DEBUG_MAX_INFO);
    debug.out(": ", DEBUG_MAX_INFO);
    debug.outln(server->arg(i), DEBUG_MAX_INFO);
  }
  if (server->hasArg("n")) {
    String clockName = server->arg("n");
    config.setString("listenToClock", clockName);
    fastclock.setListenToClock(clockName);
    page += F("<div>Set fastclock to listen to clock with name ");
    page += clockName;
    page += F(".</div>");
  }
  if (server->hasArg("b")) {
    int brightness = server->arg("b").toInt();
    sevenSegmentClock.setBrightness(brightness);
    config.setInt("brightness", brightness);
    page += F("<div>Set brightness to ");
    page += server->arg("b");
    page += F(".</div>");
  }
  if (server->hasArg("c")) {
    String colorName = server->arg("c");
    SevenSegmentClock::Color colorHandle = sevenSegmentClock.getColorHandleByName(server->arg("c"));
    sevenSegmentClock.setColor(colorHandle);
    config.setString("clockColor", colorName);
    page += F("<div>Set color to ");
    page += server->arg("c");
    page += F(".</div>");
  }
  if (server->hasArg("m")) {
    debug.out("setting clock mode to ", DEBUG_MAX_INFO); debug.outln(server->arg("m"), DEBUG_MAX_INFO);
    page += F("<div>Set clock mode to ");
    page += server->arg("m");
    page += F(".</div>");
    if (server->arg("m").equals("real")) config.setString("appMode", MODE_REALCLOCK);
    else if (server->arg("m").equals("fast")) config.setString("appMode", MODE_FASTCLOCK);
    else if (server->arg("m").equals("demo")) config.setString("appMode", MODE_DEMO);
    else {
      debug.outln("ERROR: Unknown application mode, going into demo mode", DEBUG_ERROR);
      config.setString("appMode", MODE_DEMO);
      page += F("<div>ERROR: Unknown clockmode, using default: demo.</div>");
    }
  }
  if (server->hasArg("utc")) {
    page += F("<div>Set real clock offset to ");
    int timeOffset;
    if (server->arg("utc").equals("")) {
      page += "120";
      timeOffset = 120;
    } else {
      page += server->arg("utc");
      timeOffset = server->arg("utc").toInt();
    }
    config.setInt("utcTimeOffsetMinutes", timeOffset);
    timeClient.setTimeOffset(timeOffset * 60);
    page += F(" minutes.</div>");
  }
  page += String(F("<div>Configuration updated.</div>"));
  page += FPSTR(_CONFIG_BUTTON);
  page += FPSTR(_SAVE_PERM_BUTTON);
  page += FPSTR(_END);
  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);
}

void WebUI::appConfigSavePermanent(void) {
  String page = FPSTR(_HEAD);

  page.replace("{v}", "7Seg Config");
  page += FPSTR(_SCRIPT);
  page += FPSTR(_STYLE);
  page += FPSTR(_HEAD_END);
  page += String(F("<h1>"));
  page += appName;
  page += String(F("</h1>"));

  debug.outln("Writing configs to save them permanently", DEBUG_MAX_INFO);
  config.writeAllConfigs();
  page += String(F("<div>Configuration permanently saved.</div>"));
  page += FPSTR(_CONFIG_BUTTON);
  page += FPSTR(_END);
  server->sendHeader("Content-Length", String(page.length()));
  server->send(200, "text/html", page);
}
