//
//    FILE: DjSimpleFS.cpp
// VERSION: 1.0
// PURPOSE: File system
//
//

#include <DjSimpleFS.h>

boolean SimpleFS::_initialized = false;
FSInfo SimpleFS::fsInfo;

//format bytes
String SimpleFS::formatBytes(size_t bytes) {
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}


void SimpleFS::print_filesystem_info()
{
  debug.outln(F("---------------"));
  debug.outln(F("Filesystem Info"));
  debug.outln(F("---------------"));
  debug.out(F("Filesystem capacity: ")); debug.outln(formatBytes(fsInfo.totalBytes));
  debug.out(F("Used capacity: ")); debug.outln(formatBytes(fsInfo.usedBytes));
  debug.out(F("maxOpenFiles: ")); debug.outln(fsInfo.maxOpenFiles);
  debug.outln(F("File system directory:"));
  debug.outln(F("----------------------"));
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    debug.out(F("FS File: ")); debug.out(fileName.c_str());
    debug.out(F(", size: ")); debug.outln(formatBytes(fileSize).c_str());
  }
  debug.outln(F("__________________"));
  delay(10);
  logHeap();
}

void SimpleFS::begin() {
  if (_initialized) return;

  if (SPIFFS.begin() == false) {
    debug.outln(F("Formatting file system ..."));
    SPIFFS.format();
  }
  SPIFFS.info(fsInfo);
  print_filesystem_info();
  _initialized = true;
}

static size_t minFreeSpace = 1024 * 1024 * 1024;

void SimpleFS::loop() {
  SPIFFS.info(fsInfo);
  size_t freeSpace = fsInfo.totalBytes - fsInfo.usedBytes;

  if (freeSpace < minFreeSpace) {
    minFreeSpace = freeSpace;
    debug.out(F("New minFreeSpace=")); debug.outln(formatBytes(minFreeSpace));
  }
}

size_t SimpleFS::getMaxPathLength() {
  if (!_initialized) {
    return -1; // ERROR case
  }
  return fsInfo.maxPathLength;
}

