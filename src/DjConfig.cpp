//
//    FILE: Config.h
// VERSION: 0.1
// PURPOSE: Configuration of controller, wifi and application basics
//
//

#include "DjConfig.h"
#include <FS.h>
#include <ArduinoJson.h>

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#define DEFAULT_CONFIG_JSON "{default:true}"
struct ConfigItem {
  String section;
  String name;
  String type;
  String value;
  boolean changed;
};
static struct ConfigItem configItems[MAX_NUMBER_OF_CONFIG_ITEMS];
static int numberOfConfigItems = 0;


static StaticJsonDocument<2000> jsonDoc;

void Config::loadFile(const char *filename, const char * const sectionConfigItemDescriptions[], int numberOfSectionConfigs)
{
  boolean fileExists = false;
  boolean fileConfigIsComplete = true;

  static const char *defaultJsonString = "{\"empty\"=\"empty\"}";

  logHeap();
  debug.out(F(">>> loadFile ")); debug.outln(filename);
  _filename = String(filename);
  if (!_filename.startsWith("/")) _filename = "/" + _filename;

  boolean jsonLoaded = false;
  if (SPIFFS.exists(_filename)) {
    //file exists, reading and loading
    fileExists = true;
    debug.out(F("reading config file "),DEBUG_MIN_INFO); debug.outln(_filename);
    File configFile = SPIFFS.open(_filename, "r");
    if (configFile) {
      size_t size = configFile.size();
      configFile.seek(0, SeekSet);
      debug.out(F("opened config file "),DEBUG_MIN_INFO); debug.out(_filename,DEBUG_MIN_INFO); debug.out(F(" size=")); debug.outln(size);
      DeserializationError error = deserializeJson(jsonDoc, configFile);
      configFile.close();
      if (error) {
        debug.out(F("Failed to read file, using default configuration"), DEBUG_ERROR);
        deserializeJson(jsonDoc, defaultJsonString);
      } else {
        debug.out(F("readBytes result=")); debug.outln(size);
        jsonLoaded = true;
      }
    } else {
      debug.outln(F("ERROR: config file exists but cannot be opened."),DEBUG_ERROR);
      deserializeJson(jsonDoc, defaultJsonString);
    }
  } else {
    debug.outln(F("config file not found ..."),DEBUG_MED_INFO);
    deserializeJson(jsonDoc, defaultJsonString);
  } // end of if file exists
  // end of mounted file system (we want to have default config, if no FS available)

  // start setting the config variables
  for (int i=0; i<numberOfSectionConfigs; ++i) {
    String configItemDescription = sectionConfigItemDescriptions[i];
    if (configItemDescription != NULL && configItemDescription.length() > 0) {
      configItems[numberOfConfigItems].section = filename;
      int firstColon = configItemDescription.indexOf(":");
      int secondColon = configItemDescription.indexOf(":", firstColon+1);
      configItems[numberOfConfigItems].name = configItemDescription.substring(0, firstColon);
      configItems[numberOfConfigItems].type = configItemDescription.substring(firstColon+1, secondColon);
      debug.out(F("Adding config item: [")); debug.out(configItems[numberOfConfigItems].section); debug.out(F("] "));
      debug.out(configItems[numberOfConfigItems].name);
      if (jsonLoaded && jsonDoc[configItems[numberOfConfigItems].name] != NULL) {
        if (configItems[numberOfConfigItems].type.equals("int")) {
          configItems[numberOfConfigItems].value = String((int) jsonDoc[configItems[numberOfConfigItems].name]);
        } else
        if (configItems[numberOfConfigItems].type.equals("boolean")) {
          configItems[numberOfConfigItems].value = String(jsonDoc[configItems[numberOfConfigItems].name] ? "true" : "false");
        } else
        if (configItems[numberOfConfigItems].type.equals("string")) {
          configItems[numberOfConfigItems].value = String((const char *) jsonDoc[configItems[numberOfConfigItems].name]);
        } else {
          debug.out(F("ERROR: Unknown type in config definition - "), DEBUG_ERROR);
          debug.outln(configItems[numberOfConfigItems].type, DEBUG_ERROR);
        }
        debug.out(F(" (from file): "));
      } else {
        // parameter does not exist in json config file, thus we add the default value
        fileConfigIsComplete = false;
        configItems[numberOfConfigItems].value = configItemDescription.substring(secondColon+1);
        debug.out(F(" (from default): "));
      }
      debug.out(configItems[numberOfConfigItems].value);
      debug.out(F(" (")); debug.out(configItems[numberOfConfigItems].type); debug.out(F(")"));
      configItems[numberOfConfigItems].changed = false;

      numberOfConfigItems++;
      if (numberOfConfigItems >= MAX_NUMBER_OF_CONFIG_ITEMS) {
        debug.outln(F("ERROR: Too many configuration items!"),DEBUG_ERROR);
        break;
      } else {
        debug.out(F(" [")); debug.out(numberOfConfigItems); debug.out(F("] "));
      }
      logHeap();
    }
  } // end for ... numberOfSectionConfigs ...

  logHeap();
  // Finally, if the configuration file does not exist so far or new entries have been added, then create/update it:
  if (!fileExists || !fileConfigIsComplete) {
    debug.outln(F("Writing config file due to "));
    if (!fileExists) debug.outln(F(" - file does not exist "));
    if (!fileConfigIsComplete) debug.outln(F(" - file config is not complete"));
    debug.outln(_filename);
    writeConfigFile(_filename);
  }
}

void Config::writeAllConfigs(void) {
  boolean allChangesWritten = false;
  logHeap();

  while (!allChangesWritten) {
    for (int i=0; i<numberOfConfigItems; ++i) {
      debug.out(configItems[i].section);
      debug.out(".");
      debug.out(configItems[i].name);
      debug.out("(");
      debug.out(configItems[i].changed ? "changed" : "-");
      debug.out(")=");
      debug.outln(configItems[i].value);
      if (configItems[i].changed) {
        writeConfigFile(configItems[i].section);
        break; // leave for-loop and restart search for changes
      }
    }
    // no further changes found, we are done
    allChangesWritten = true;
  }
}

void Config::writeConfigFile(String filename) {
  logHeap();
  if (!filename.startsWith("/")) {
    filename = "/" + filename;
  }
  File configFile = SPIFFS.open(filename, "w");

  if (configFile) {
    StaticJsonDocument<2000> json;
    for (int i=0; i<numberOfConfigItems; ++i) {
      if (configItems[i].section.equals(filename.substring(1))) {
        debug.out("writeConfig "); debug.outln(configItems[i].section + "." + configItems[i].name + "=" + configItems[i].type + ":" + configItems[i].value);
        if (configItems[i].type.equals("int")) {
          json[configItems[i].name] = configItems[i].value.toInt();;
        } else if (configItems[i].type.equals("boolean")) {
          if (configItems[i].value.equals("true") || configItems[i].value.equals("TRUE") || configItems[i].value.toInt() != 0) {
            json[configItems[i].name] = true;
          } else {
            json[configItems[i].name] = false;
          }
        } else {
          json[configItems[i].name] = configItems[i].value;
        }
      }
    }
    if (serializeJson(json, configFile) == 0) {
      debug.outln(F("Failed to write configuration to file"));
    }
    configFile.close();
  } else {
    debug.out(F("ERROR: Cannot write config file ")); debug.outln(filename);
  }
}

void Config::begin(const char * filename, const char * const *configItemDescriptions, unsigned int numberOfConfigItems)
{
  debug.out(F("Config::Config called with configuration details for "));
  debug.out(filename); debug.out(" having "); debug.out(numberOfConfigItems); debug.outln(" configuration items.");
  loadFile(filename, configItemDescriptions, numberOfConfigItems);
}

static int _getConfigItemIndex(const String configSection, const String name)
{
  for (unsigned int i=0; i<sizeof(configItems); ++i) {
    if (/*configItems[i].section.equals(configSection) &&*/ configItems[i].name.equals(name)) {
      return i;
    }
  }
  return -1;
}

String Config::getString(String key)
{
  int index = _getConfigItemIndex(/*"system"*/ _filename, key);

  //logHeap();
  // debug.out("Config.getString("); debug.out(key); debug.outln(")");
  if (index < 0) {
    debug.out(F("ERROR: cannot find config item ")); debug.outln(key);
    return String("");
  } else {
    // debug.outln(configItems[index].value);
    return String(configItems[index].value);
  }
}

const char * Config::getCString(String key)
{
  return getString(key).c_str();
}

void Config::setString(String key, String value) {
  int index = _getConfigItemIndex(/*"system"*/ _filename, key);
  logHeap();
  // debug.out("Config.getString("); debug.out(key); debug.outln(")");
  if (index < 0) {
    debug.out(F("ERROR: Tried to set new value, but cannot find config item ")); debug.outln(key);
  } else {
    // debug.outln(configItems[index].value);
    configItems[index].value = String(value);
    configItems[index].changed = true;
  }
}


int Config::getInt(String key)
{
  return this->getString(key).toInt();
}

void Config::setInt(String parameter, int value) {
  this->setString(parameter, String(value));
}

boolean Config::getBoolean(String key)
{
  String sval = getString(key);
  boolean bval = false;

  if (sval.length() > 0) {
    if (sval.equals("true") || sval.equals("TRUE")) {
      bval = true;
    } else if (sval.equals("false") || sval.equals("FALSE")) {
      bval = false;
    } else {
      int ival = sval.toInt();
      if (ival) {
        bval = true;
      }
    }

    return bval;
  } else {
    return false;
  }
}

void Config::setBoolean(String parameter, boolean value) {
  this->setString(parameter, value ? "true" : "false");
}
