#include <stdio.h>  // fprintf()
#include <conio.h>  // kbhit(), getch()
#include <stdlib.h> // atoi()
#include <string.h> // strcmp()

#ifdef __BORLANDC__
#include "src/strprx16/stdtypes.h"
#include "src/strprx16/support.h"
#include "src/strprx16/syscfg.h"
#elif _MSC_VER
#include "strprx16/stdtypes.h"
#include "strprx16/support.h"
#include "strprx16/syscfg.h"
#endif

extern struct SystemConfiguration settings;

/*
   "     C    : capture timing and synchronization data to the host\n" \
   "     B    : bitwise test mode evaluate strip to determine readability\n" \
   "     H    : harvest optical scan in slices (requires special cable and setup)\n" \
*/

const char* promptStr = 
   "\n  1,2,3,4 : set the serial port for the optical reader\n" \
   "     I    : inspect hardware revision ID of the optical reader\n" \
   "     R    : read a sequence of data strips\n" \
   "     T    : read the text component of a strip sequence in ASCII terminal mode\n" \
   "     ?    : display this help message\n" \
   "     Q    : quit the program\n\n" \
   "Select> ";

int main( int argC, char** argV )
{
   int16_t inChar = 0;

   char option[] =
   {
      'Q', 'q', 0x1B, // ESC
      '1', '2', '3', '4',
      'r', 'R',
      'i', 'I',
      't', 'T',
      'c', 'C',
      'b', 'B',
      'h', 'H',
      '?'
   };

   SystemConfiguration_Initialize( argC, argV );

   printIdentity( argV[ 0 ] );

   if ( 2 == argC )
   {
      if ( ( '/' == argV[ 1 ][ 0 ] ) && ( '?' == argV[ 1 ][ 1 ] ) )
      {
         printUsage();

         return 0;
      }
   }

   // handle single immediate command?
   if ( cmdln_setup != processCommandLineArguments( argC, argV ) )
   {
      if ( 0 < settings.readerPortOpen )
      {
         closeSerialPort( settings.readerPort );
      }

      return 0;
   }

   // interactive commands
   do
   {
      if ( 0 != settings.readerPortOpen )
      {
         printf( "Optical reader is connected to serial port COM_%d\n", settings.readerPort );
      }

      inChar =
         prompt( ( char* )promptStr, option, sizeof( option ) / sizeof( option[ 0 ] ),
                 &( option[ sizeof( option ) / sizeof( option[ 0 ] ) - 1 ] ) );

      switch( inChar )
      {
         case '1' :
         case '2' :
         case '3' :
         case '4' :
            updateSerialPort( inChar - '0' );
            break;

         case 'i' :
         case 'I' :
            inspectHardwareRevision( false );
            break;

         case 'r' :
         case 'R' :
            readStripSequence( false );
            break;

         case 't' :
         case 'T' :
            readTerminal( false );
            break;

         case 'b' :
         case 'B' :
            bitwiseTestMode( false );
            break;

         case 'c' :
         case 'C' :
            captureTimingAndSync( false );
            break;

         case 'h' :
         case 'H' :
//            harvestOpticalSlices( false );
//            break;

         case '?' :
            break;
      }
   }
   while ( ( 0x1B != inChar ) && ( 'q' != inChar ) && ( 'Q' != inChar ) );

   if ( 0 < settings.readerPortOpen )
   {
      closeSerialPort( settings.readerPort );
   }

   return 0;
}

