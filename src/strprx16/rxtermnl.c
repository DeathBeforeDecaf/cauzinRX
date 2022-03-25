#include <time.h> // clock_t

#ifdef __BORLANDC__
#include "src/strprx16/cznstrip.h"
#include "src/strprx16/serlib.h"
#include "src/strprx16/stdtypes.h"
#include "src/strprx16/support.h"
#include "src/strprx16/syscfg.h"
#elif _MSC_VER
#include "cznstrip.h"
#include "serlib.h"
#include "stdtypes.h"
#include "support.h"
#include "syscfg.h"
#endif

extern struct SystemConfiguration settings;

enum CZNRXResult readTerminalStrip( struct StripSequenceType* sequence )
{
   enum CZNRXResult result = unknown_result;

   uint16_t byteCounter;

   clock_t first;
   clock_t start;
   clock_t now;

   uint16_t next;
   uint16_t last;

   uint16_t latch;

   uint16_t index;

   uint16_t i;

   if ( 0 == settings.readerPortOpen )
   {
      openSerialPort( settings.readerPort );
   }

   Attach_Buffer( &( sequence->scan ) );

   byteCounter = 0;

   Serial_Write( 'T' );

   first = clock();

   next = 0;

   latch = 0;

   index = 0;

   // timeout after ~6 seconds of waiting without the buffer increasing
   for ( i = 0; i < ( ( uint16_t )settings.latchThreshold ) * settings.timeoutThreshold; i++ )
   {
      start = clock();

      // time-constrained loop (serial read byte within ~250ms)
      do
      {
         last = next;

         next = Ready_Serial();

         if ( last != next )
         {
            if ( latch <= settings.latchThreshold )
            {
               ++latch;
            }

            start = clock();
         }

         if ( settings.latchThreshold == latch )
         {
            // latch after ~1 sec reads (5 intervals)
            i = settings.latchThreshold * settings.timeoutThreshold;  
         }

         now = clock();
      }
      while ( ( ( now - start ) < ( ( clock_t )settings.readIntervalMS ) )
              && ( byteCounter < settings.maxStripSizeBytes ) );
   }

   Detach_Buffer( &( sequence->scan ) );

   if ( ( 1U + settings.latchThreshold ) == latch )
   {
      printf( "   Read %u bytes in %ld ms\n", sequence->scan.bytesReceived, ( now - first ) );
   }

   index = 0;

   // accept CTF
   while ( ( index < sequence->scan.bytesReceived ) && ( 0 == sequence->scan.buffer[ index ] ) )
   {
      index++;
   }

   // optical reader never acknowleges 'T'erminal read request

   sequence->scan.startIndex = index;

   // no read status from terminal read request (assume successful read)
   result = success_result;

   return result;
}




void readTerminal( bool isImmediate )
{
   struct StripSequenceType sequence = { 0 };

   enum CZNRXResult result = unknown_result;

   uint16_t index;

   if ( false == isImmediate )
   {
      printf( ">>> Read Terminal Strip\n" );
   }

   sequence.scan.buffer = ( char* )calloc( 1, settings.maxStripSizeBytes );

   if ( NULL != sequence.scan.buffer )
   {
      sequence.scan.size = settings.maxStripSizeBytes;

      if ( 0 == settings.readerPortOpen )
      {
         openSerialPort( settings.readerPort );
      }

      result = readTerminalStrip( &sequence );
   }

   if ( success_result == result )
   {
      index = sequence.scan.startIndex;

      if ( sequence.scan.bytesReceived > 3 )
      {
         saveAs( "Please enter the name of the output file to store the terminal data\n> ", &sequence );
      }
      else
      {
         printf( "Sorry, nothing to save\n" );
      }
   }

   if ( NULL != sequence.scan.buffer )
   {
      free( sequence.scan.buffer );
   }
}