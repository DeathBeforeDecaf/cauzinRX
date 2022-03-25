#include "syscfg.h"

/*
struct SystemConfiguration
{
   int16_t readerPort; // = 1
   int16_t readerPortOpen; // = 0;

   uint16_t MAX_STRIP_SIZE; // = 8192

   uint16_t readIntervalMS; // 250ms

   uint8_t  latchThreshold; // 4
   uint8_t  timeoutThreshold; // 6 ( * latch Threshold )

   char workingDirectory[ _MAX_PATH ];
   char outputDirectory[ _MAX_PATH ];
};
*/

struct SystemConfiguration settings = { 0 };

void SystemConfiguration_Initialize( int argC, char* argV[] )
{
   char* lastBreak = NULL;
   char* index;

   settings.readerPort = 1;

   settings.maxStripSizeBytes = 8192;

   settings.readIntervalMS = CLOCKS_PER_SEC / 4;

   settings.latchThreshold = 4;
   settings.timeoutThreshold = 6; // ( timeoutThreshold * latchThreshold )



   strcpy( settings.workingDirectory, argV[ 0 ] );

   index = settings.workingDirectory;

   while ( *index )
   {
      if ( ( '/' == *index ) || ( '\\' == *index ) )
      {
         lastBreak = index;
      }

      ++index;
   }

   if ( NULL != lastBreak )
   {
      lastBreak++;
   }
   else
   {
      lastBreak = settings.workingDirectory;
   }

   strcpy( lastBreak, "_CAUZIN_.RX\\" );


   // explicit target directory for inbound file data
   index = getenv( "CZNFILES" );

   if ( NULL != index )
   {
      strcpy( settings.outputDirectory, index );

      index = settings.outputDirectory;

      while ( *index )
      {
         if ( ( '/' == *index ) || ( '\\' == *index ) )
         {
            lastBreak = index;
         }

         ++index;
      }

      if ( lastBreak + 1 != index )
      {
         *index = '\\';
      }      

      createSubdirectory( settings.outputDirectory );
   }


/*
   getcwd( settings.outputDirectory, _MAX_PATH - 1 );

   index = settings.outputDirectory + strlen( settings.outputDirectory );

   if ( index > settings.outputDirectory )
   {
      --index;
   }

   if ( ( '/' != *index ) && ( '\\' != *index ) )
   {
      index++;

      *index = '\\';
   }
*/
}