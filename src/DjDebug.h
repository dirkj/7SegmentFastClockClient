
#ifndef debug_h
#define debug_h

#include <Arduino.h>
#include <pgmspace.h>

#define logHeap() Debug::heapLogger(__FILE__,__LINE__,__func__)

class Debug {
  public:
    Debug();
    Debug(int baudRate);
  	Debug(const char * outputChannel);
    Debug(int baudRate, const char * outputChannel);
    static void heapLogger(const char *fileName, const int line, const char *functionName);
    static void out(int number);
    static void outln(int number);
    static void out(int number, int level);
    static void outln(int number, int level);
    static void outHex16(uint16_t *data, uint8_t length);
    static void outHex8(uint8_t *data, uint8_t length);
  	static void out(const String& text, const int level);
  	static void out(const String& text);
  	static void outln(const String& text, const int level);
  	static void outln(const String& text);
    static void out(const char * text, const int level);
    static void out(const char * text);
    static void outln(const char * text, const int level);
    static void outln(const char * text);
    static void out_p(PGM_P text, const int level);
    static void out_p(PGM_P text);
    static void outln_p(PGM_P text, const int level);
    static void outln_p(PGM_P text);
  	static void setOutputUptoLevel(int level);
  	static void setDefaultDebugLevel(int level);

  private:
  	static int outputUptoLevel;
  	static int defaultDebugLevel;
};

// Wieviele Informationen sollen über die serielle Schnittstelle ausgegeben werden?
#define DEBUG 3

// Definition der Debuglevel
#define DEBUG_ERROR 1
#define DEBUG_WARNING 2
#define DEBUG_MIN_INFO 3
#define DEBUG_MED_INFO 4
#define DEBUG_MAX_INFO 5

#endif
