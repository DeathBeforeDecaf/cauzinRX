#include <ctype.h> // isalnum(), ispunct(), isspace()
#include <time.h> // clock_t

#ifdef __BORLANDC__
#include "src/strprx16/support.h"
#include "src/strprx16/syscfg.h"
#elif _MSC_VER
#include "support.h"
#include "syscfg.h"
#endif

extern struct SystemConfiguration settings;

void inspectHardwareRevision( bool isImmediate )
{
   uint8_t* buffer;

   uint16_t byteCounter = 0;

   bool isAcknowledged = false;

   clock_t start;
   clock_t now;

   uint16_t index;

   uint16_t i;

   if ( false == isImmediate )
   {
      printf( ">>> Inspect Hardware Revision\n\n" );
   }

   if ( 0 == settings.readerPortOpen )
   {
      openSerialPort( settings.readerPort );
   }

   buffer = ( uint8_t* )malloc( settings.maxStripSizeBytes / 4 );

   if ( NULL == buffer )
   {
      printf( "   Optical Hardware Revision: INSUFFICIENT MEMORY\n" );
      printf( "   0 bytes received\n\n" );

      return;
   }

   memset( buffer, 0, settings.maxStripSizeBytes / 4 );

   Serial_Write( 'I' );

   start = clock();

   // time-constrained loop (serial read byte within ~667ms)
   do
   {
      if ( Ready_Serial() )
      {
         buffer[ byteCounter++ ] = Serial_Read();

         start = clock();
      }

      now = clock();
   }
   while ( ( ( now - start ) < ( ( clock_t )settings.readIntervalMS ) )
           && ( byteCounter < ( settings.maxStripSizeBytes / 4 ) ) );

   if ( 0 == byteCounter )
   {
      // timeout
      printf( "   Optical Hardware Revision: READER COMMUNICATION TIMEOUT\n" );
      printf( "   %d bytes received\n\n", byteCounter );
   }
   else
   {
      // display
      printf( "   Optical Hardware Revision: " );

      index = 0;

      // accept CTF
      while ( ( index < byteCounter ) && ( 0 == buffer[ index ] ) )
      {
         index++;
      }

      // reader acknowleges command request
      if ( ( index < byteCounter ) && ( 'I' == buffer[ index ] ) )
      {
         isAcknowledged = true;

         index++;
      }

      if ( 0 < index )
      {
         for ( i = index; i < byteCounter; i++ )
         {
            if ( isalnum( buffer[ i ] ) || ispunct( buffer[ i ] ) || isspace( buffer[ i ] ) )
            {
               fputc( buffer[ i ], stdout );
            }
            else
            {
               printf( " 0x%02X", buffer[ i ] );
            }
         }
      }

      printf( "\n   %d bytes received\n\n", byteCounter );
   }

   free( buffer );
}
