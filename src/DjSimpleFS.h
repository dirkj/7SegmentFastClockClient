//
//    FILE: DjSimpleFS.h
// VERSION: 1.0
// PURPOSE: File system
//
//

#ifndef _djSimpleFSLoaded
#define _djSimpleFSLoaded true

#include <Arduino.h>
#include <FS.h>
#include "DjDebug.h"

class SimpleFS {
  public:
    SimpleFS(Debug& _debug):debug(_debug) { logHeap(); };
    void begin();
    void loop();
    boolean initialized() { return _initialized; };
    static size_t getMaxPathLength();
    void print_filesystem_info();

  private:
    Debug& debug;
    String formatBytes(size_t bytes);
    static boolean _initialized;
    static FSInfo fsInfo;
};

#endif
