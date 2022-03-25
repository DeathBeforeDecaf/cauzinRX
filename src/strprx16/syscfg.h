#ifndef SYSCFG_H
#define SYSCFG_H

#include <direct.h> // _getcwd()
#include <stdio.h>  // fprintf()
#include <stdlib.h> // _MAX_PATH, getenv()
#include <string.h> // strcpy()
#include <time.h>   // CLOCKS_PER_SEC

#include "stdtypes.h"
#include "support.h"

struct SystemConfiguration
{
   int16_t readerPort; // = 1
   int16_t readerPortOpen; // = 0;

   uint16_t maxStripSizeBytes; // = 8192

   uint16_t readIntervalMS; // 250ms

   uint8_t  latchThreshold; // 4
   uint8_t  timeoutThreshold; // 6 ( * latch Threshold )

   char workingDirectory[ _MAX_PATH ];
   char outputDirectory[ _MAX_PATH ];
};

void SystemConfiguration_Initialize( int argC, char* argV[] );

//// Resolution of System Configuration Properties
// This structure encapsulates all of the global, system or otherwise
// magic variables and exposes them in a single plain old datatype.
//
// System Defaults -> Initial System Settings
//    Resolve Environment Variables
//    Load Application Configuration File(s) values
//    Merge Command Line Options
//    In-Application Property Settings

#endif
