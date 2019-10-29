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


/*
 PrintHex routines for Arduino: to print byte or word data in hex with
 leading zeroes.
 Copyright (C) 2010 Kairama Inc

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
void PrintHex8(uint8_t *data, uint8_t length) // prints 8-bit data in hex with leading zeroes
{
       Serial.print("0x");
       for (int i=0; i<length; i++) {
         if (data[i]<0x10) {Serial.print("0");}
         Serial.print(data[i],HEX);
         Serial.print(" ");
       }
}

void Debug::outHex16(uint16_t *data, uint8_t length) // prints 16-bit data in hex with leading zeroes
{
       //Serial.print("0x");
       for (int i=0; i<length; i++)
       {
         uint8_t MSB=byte(data[i]>>8);
         uint8_t LSB=byte(data[i]);

         if (MSB<0x10) {Serial.print("0");} Serial.print(MSB,HEX); Serial.print(" ");
         if (LSB<0x10) {Serial.print("0");} Serial.print(LSB,HEX); Serial.print(" ");
       }
}

void Debug::outHex8(uint8_t *data, uint8_t length) // prints 16-bit data in hex with leading zeroes
{
       //Serial.print("0x");
       for (int i=0; i<length; i++)
       {
         //uint8_t MSB=byte(data[i]>>8);
         uint8_t LSB=byte(data[i]);

         //if (MSB<0x10) {Serial.print("0");} Serial.print(MSB,HEX); Serial.print(" ");
         if (LSB<0x10) {Serial.print("0");} Serial.print(LSB,HEX); Serial.print(" ");
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
