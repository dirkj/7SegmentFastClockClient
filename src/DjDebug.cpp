#include <Arduino.h>
#include "DjDebug.h"

/*****************************************************************/
/* Debug output                                                  */
/*****************************************************************/

int Debug::outputUptoLevel = DEBUG_MAX_INFO;
int Debug::defaultDebugLevel = DEBUG_MIN_INFO;

Debug::Debug() {
  Debug(115200, "Serial");
}

Debug::Debug(int baudRate) {
  Debug(baudRate, "Serial");
}

Debug::Debug(const char * outputChannel) {
  Debug(115200, outputChannel);
}

Debug::Debug(int baudRate, const char * outputChannel)
{
  Serial.begin(baudRate);
  Serial.println(F("\n"));
  Serial.println(F("=== Debug output starts"));
  Serial.print(F("Output to: "));
  Serial.print(outputChannel);
  Serial.print(", speed ");
  Serial.println(baudRate);
  logHeap();
}

void Debug::setOutputUptoLevel(int level)
{
  outputUptoLevel = level;
}

void Debug::setDefaultDebugLevel(int level)
{
  defaultDebugLevel = level;
}

void Debug::heapLogger(const char *fileName, const int line, const char *functionName)
{
  out(F("> Heap: "));
  out(ESP.getFreeHeap());
  out(F(" - "));
  // search filename beginning without path, look for slash character
  const char *slash = fileName + strlen(fileName);
  while (*slash != '/' && slash > fileName)
    --slash;
  out(slash);
  out(F(": "));
  outln(line);
  #if 0
  if (functionName) {
    out(F(" in "));
    outln(functionName);
  }
  #endif
  delay(10);
}

void Debug::out(const String& text, const int level)
{
  if (level <= outputUptoLevel) {
    Serial.print(text);
  }
}

void Debug::out(int number)
{
  if (defaultDebugLevel <= outputUptoLevel) {
    Serial.print(number);
  }
}

void Debug::outln(int number)
{
  if (defaultDebugLevel <= outputUptoLevel) {
    Serial.println(number);
  }
}

void Debug::out(int number, int level)
{
  if (level <= outputUptoLevel) {
    Serial.print(number);
  }
}

void Debug::outln(int number, int level)
{
  if (level <= outputUptoLevel) {
    Serial.println(number);
  }
}

void Debug::out(const char * text)
{
  if (defaultDebugLevel <= outputUptoLevel) {
    Serial.print(text);
  }
}

void Debug::out_p(PGM_P text)
{
  if (defaultDebugLevel <= outputUptoLevel) {
    Serial.print(text);
  }
}

void Debug::out(const char * text, const int level)
{
  if (level <= outputUptoLevel) {
    Serial.print(text);
  }
}

void Debug::out_p(PGM_P text, const int level)
{
  if (level <= outputUptoLevel) {
    Serial.print(text);
  }
}

void Debug::out(const String& text)
{
  if (defaultDebugLevel <= outputUptoLevel) {
    Serial.print(text);
  }
}

void Debug::outln(const String& text, const int level)
{
  if (level <= outputUptoLevel) {
    Serial.println(text);
  }
}

void Debug::outln(const String& text)
{
  if (defaultDebugLevel <= outputUptoLevel) {
    Serial.println(text);
  }
}

void Debug::outln(const char * text, const int level)
{
  if (level <= outputUptoLevel) {
    Serial.println(text);
  }
}

void Debug::outln_p(PGM_P text, const int level)
{
  if (level <= outputUptoLevel) {
    Serial.println(text);
  }
}

void Debug::outln(const char * text)
{
  if (defaultDebugLevel <= outputUptoLevel) {
    Serial.println(text);
  }
}

void Debug::outln_p(PGM_P text)
{
  if (defaultDebugLevel <= outputUptoLevel) {
    Serial.println(text);
  }
}
