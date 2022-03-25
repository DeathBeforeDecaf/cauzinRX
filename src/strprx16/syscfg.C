#ifdef __BORLANDC__
#include "src/strprx16/syscfg.h"
#elif _MSC_VER
#include "syscfg.h"
#endif

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