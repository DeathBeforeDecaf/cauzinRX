#ifndef SUPPORT_H
#define SUPPORT_H

#include <direct.h> // getcwd()
#include <stdio.h>  // fputc(), fputs()
#include <stdlib.h> // _MAX_PATH
#include <string.h> // strlen()
#include <sys\types.h> // struct _stat
#include <sys\stat.h>  // stat()

#include "cznstrip.h"
#include "serlib.h"
#include "stdtypes.h"

enum CMDLineResultType
{
   cmdln_unknown = 0,
   cmdln_error,   
   cmdln_setup,
   cmdln_completed,
};

// support.c
void printIdentity( char* filename );

void printVersion( char* filename );

void printUsage();

enum CMDLineResultType processCommandLineArguments( int16_t argC, char* argV[] );

void openSerialPort( int16_t openPort );

void closeSerialPort( int16_t closePort );

char prompt( char* promptStr, const char option[], uint8_t optionCount, char* optionDefault );

char* ltrim( char* input, const char cutChar[], unsigned char cutCharSize );

void updateSerialPort( int16_t preferredPort );

void restrictToAllowedCharacters( char* input, char substitute );

void normalize( char* input );

void createSubdirectory( char* path );

void saveAs( char* savePrompt, struct StripSequenceType* sequence );

// inspect.c
void inspectHardwareRevision( bool isImmediate );

// rxseqnce.c
void readStripSequence( bool isImmediate );

bool displayProblemHint( enum CZNRXResult hint, uint16_t stripOffset, char* stripID );

// rxtermnl.c
void readTerminal( bool isImmediate );

// rxbittst.c
void bitwiseTestMode( bool isImmediate );

// rxcaptur.c
void captureTimingAndSync( bool isImmediate );

#endif
