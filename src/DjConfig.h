//
//    FILE: Config.h
// VERSION: 0.1
// PURPOSE: Configuration of controller, wifi and application basics
//
//

#ifndef lightningConfigLoaded
#define lightningConfigLoaded true

#include <Arduino.h>
#include "DjDebug.h"

#define MAX_NUMBER_OF_CONFIG_ITEMS 50

class Config {
public:
  Config(Debug& _debug):debug(_debug) { debug.outln("Config constructor", DEBUG_MAX_INFO); logHeap(); };
  void begin(const char * filename, const char * const *configItemDescriptions, unsigned int numberOfConfigItems);
  void loadFile(const char *filename, const char * const configDescriptions[], int numConfigs);
  int getInt(String parameter);
  String getString(String parameter);
  const char * getCString(String parameter);
  boolean getBoolean(String parameter);
  void setString(String parameter, String value);
  void setInt(String parameter, int value);
  void setBoolean(String parameter, boolean value);
  void writeAllConfigs(void) ;
private:
  String _filename = "";
  Debug& debug;
  void writeConfigFile(String filename);
};

#endif
