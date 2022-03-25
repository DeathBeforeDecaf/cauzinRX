#include "support.h"
#include "syscfg.h"

extern struct SystemConfiguration settings;


void printIdentity( char* filename )
{
   fputs( "The Cauzin Softstrip Receiver -", stdout );

   printVersion( filename );
}


void printVersion( char* filename )
{
   struct _stat status = { 0 };

   uint32_t versionID;

   fputs( "  Version 0.1", stdout );

   if ( ( 0 == _stat( filename, &status ) ) && ( status.st_mode & _S_IFREG ) )
   {         
      fputs( " (", stdout );

      versionID = status.st_mtime;
      versionID /= status.st_size;

      fprintf( stdout, "%04X", versionID );

      fputc( ')', stdout );
   }

   fputc( '\n', stdout );
}


void printUsage()
{
   printf( "\nUSAGE:\n" );
   printf( "   cauzinrx [serial port#]                     (interactively execute commands)\n" );
   printf( "   cauzinrx [serial port#] [immediate command] (immediately execute command)\n\n" );

   printf( "   Immediate Commands include:\n" );
   printf( "      R  :  read a sequence of data strips\n" );
   printf( "      I  :  inspect hardware revision ID of the optical reader\n" );
   printf( "      T  :  read the text component of a strip sequence in ASCII terminal mode\n" );
   printf( "      C  :  capture timing and synchronization data to the host\n" );
   printf( "      B  :  bitwise test mode evaluate strip to determine readability\n" );
   printf( "      H  :  harvest optical scan in slices (requires special cable and setup)\n\n" );
      
   printf( "EXAMPLES:\n" );
   printf( "   cauzinrx 1 R     : read a strip sequence from Softstrip reader on COM1\n" );
   printf( "   cauzinrx 2 I     : verify reader on COM2 returns a hardware revision string\n\n" );
}


enum CMDLineResultType processCommandLineArguments( int16_t argC, char* argV[] )
{
   enum CMDLineResultType result = cmdln_unknown;

   int16_t explicitPort = 0;

   uint16_t argIndex = 1;

   if ( 1 < argC )
   {
      if ( ( '0' <= argV[ argIndex ][ 0 ] ) && ( '9' >= argV[ argIndex ][ 0 ] ) )
      {
         explicitPort = atoi( argV[ argIndex ] );

         if ( ( explicitPort > 0 ) && ( explicitPort < 5 ) )
         {
            settings.readerPort = explicitPort;         

            result = cmdln_setup;
         }
         else
         {
            fprintf( stderr, "\nERROR: Sorry only COM1 to COM4 supported, %d out of range.\n", explicitPort );

            result = cmdln_error;
         }

         if ( ( argIndex + 1 ) < ( uint16_t)argC )
         {
            argIndex++;
         }
         else
         {
            return result;
         }
      }


      if ( 0 == stricmp( "i", argV[ argIndex ] ) )
      {
         // I = inspect hardware revision ID of the optical reader
         inspectHardwareRevision( true );

         result = cmdln_completed;

         if ( ( argIndex + 1 ) < ( uint16_t)argC )
         {
            argIndex++;
         }
         else
         {
            return result;
         }
      }

      if ( 0 == stricmp( "r", argV[ argIndex ] ) )
      {
         readStripSequence( true );

         result = cmdln_completed;

         return result;
      }

      if ( 0 == stricmp( "t", argV[ argIndex ] ) )
      {
         // T ( or t ) = read a strip as ascii text starting at OpSys Type (field#11)
         readTerminal( true );

         result = cmdln_completed;

         return result;
      }

      if ( 0 == stricmp( "b", argV[ argIndex ] ) )
      {
//       bitwiseTestMode( true );

         result = cmdln_completed;

         return result;
      }

      if ( 0 == stricmp( "c", argV[ argIndex ] ) )
      {
         // C = capture timing and synchronization data to the host
//       captureTimingAndSync( true );

         result = cmdln_completed;

         return result;
      }

      if ( 0 == stricmp( "h", argV[ argIndex ] ) )
      {
         // H + $0E$0E$0E = accumulate all optical slices w/special cable
//       harvestOpticalSlices( true );

         result = cmdln_completed;

         return result;
      }
   }
   else
   {
      result = cmdln_setup;
   }

   return result;
}


void openSerialPort( int16_t openPort )
{
   if ( 0 == settings.readerPortOpen )
   {
      settings.readerPortOpen++;

      switch ( openPort )
      {
         case 1:
            Open_Serial( COM_1, SDIV_BAUD_4800, SCFG_PARITY_NONE | SCFG_BITS_8 | SCFG_STOP_1 );
            break;

         case 2:
            Open_Serial( COM_2, SDIV_BAUD_4800, SCFG_PARITY_NONE | SCFG_BITS_8 | SCFG_STOP_1 );
            break;

         case 3:
            Open_Serial( COM_3, SDIV_BAUD_4800, SCFG_PARITY_NONE | SCFG_BITS_8 | SCFG_STOP_1 );
            break;

         case 4:
            Open_Serial( COM_4, SDIV_BAUD_4800, SCFG_PARITY_NONE | SCFG_BITS_8 | SCFG_STOP_1 );
            break;
      }
   }
}

void closeSerialPort( int16_t closePort )
{
   if ( 0 < settings.readerPortOpen )
   {
      settings.readerPortOpen = 0;

      switch ( closePort )
      {
         case 1:
            Close_Serial( COM_1 );
            break;

         case 2:
            Close_Serial( COM_2 );
            break;

         case 3:
            Close_Serial( COM_3 );
            break;

         case 4:
            Close_Serial( COM_4 );
            break;
      }
   }
}


void updateSerialPort( int16_t preferredPort )
{
   printf( ">>> Update Serial Port\n\n" );

   if ( preferredPort != settings.readerPort )
   {
      if ( 0 < settings.readerPortOpen )
      {
         closeSerialPort( settings.readerPort );

         settings.readerPortOpen = 0;
      }

      settings.readerPort = preferredPort;
   }
}


char prompt( char* promptStr, const char option[], uint8_t optionCount, char* optionDefault )
{
   char result = '\0';

   char selection;
   char extended;

   uint8_t i;

   // absorb pending keypress(es) before prompt
   while ( kbhit() )
   {
      selection = getch();

      if ( 0 == selection )
      {
         extended = getch();
      }
   }

   do
   {
      // display choices
      fputs( promptStr, stdout );

      selection = getch();

      if ( 0 == selection )
      {
         extended = getch();
      }

      if ( 0 < selection )
      {
         // normal keypress
         for ( i = 0; i < optionCount; i++ )
         {
            if ( option[ i ] == selection )
            {
               result = selection;

               break;
            }
         }
      }
      else
      {
         // command keypress
         for ( i = 0; i < optionCount; i++ )
         {
            if ( option[ i ] == extended )
            {
               result = extended;

               break;
            }
         }
      }

      // was the any key pressed?
      if ( ( '\0' == result ) && ( NULL != optionDefault ) )
      {
         result = *optionDefault;
      }

      // should a default selection be displayed? => y
      if ( ( 32 < result ) && ( 127 > result ) )
      {
         fputc( result, stdout );
      }

      fputc( '\n', stdout );      
   }
   while ( '\0' == result );
   
   return result;
}


char* ltrim( char* input, const char cutChar[], unsigned char cutCharSize )
{
   char* result = input;

   unsigned char i;

   char* index;

   if ( ( NULL != input ) && ( '\0' != *input ) )
   {
      index = input;

      do
      {
         for ( i = 0; i < cutCharSize; i++ )
         {
            if ( cutChar[ i ] == *index )
            {
               result = index + 1;

               break;
            }
         }

         ++index;
      }
      while ( ( index == result ) && ( '\0' != *index ) );
   }

   return result;
}


void rtrim( char* input, const char cutChar[], unsigned char cutCharSize )
{
   char* index;

   uint8_t i;

   if ( ( NULL != input ) && ( '\0' != *input ) )
   {
      index = input + 1;

      while ( '\0' != *index ) { ++index; }

      i = 0;

      do
      {
         --index;

         for ( i = 0; i < cutCharSize; i++ )
         {
            if ( cutChar[ i ] == *index )
            {
               *index = '\0';

               break;
            }
         }
      }
      while ( ( '\0' == *index ) && ( index != input ) );
   }
}


void restrictToAllowedCharacters( char* input, char substitute )
{
   char* index;

   if ( NULL != input )
   {
      for ( index = input; '\0' != *index; index++ )
      {
         if ( '~' >= *index )
         {
            if ( '@' <= *index )
            {
               switch ( *index )
               {
                  case  '[' :
                  case '\\' :
                  case  ']' :
                  case  '|' :
                     *index = substitute;
                     break;

                  default:
                     continue;
               }
            }
            else if ( '0' <= *index )
            {
               if ( '9' >= *index )
               {
                  continue;
               }
               else
               {
                  *index = substitute;
               }
            }
            else if ( ( '!' <= *index ) && ( '-' >= *index ) )
            {
               switch ( *index )
               {
                  case '"' :
                  case '*' :
                  case '+' :
                  case ',' :
                     *index = substitute;
                     break;

                  default :
                     continue;
               }
            }
            else
            {
               *index = substitute;
            }
         }
         else
         {
            *index = substitute;
         }
      }      
   }
}

// convert a filename-like string into 8 valid filename characters or
// 8.3 characters (filename with extension)
// https://en.wikipedia.org/wiki/8.3_filename
void normalize( char* input )
{
   char* trimmedInput;

   char truncateChars[] =
   {
      ' ',
      '\t',
      '.'
   };

   uint16_t index;

   char next;

   char* lhs;
   char* rhs;

   char* lastDot;

   uint8_t i;

   char result[ 13 ] = { 0 };

   if ( ( NULL != input ) && ( '\0' != *input ) )
   {
      trimmedInput = strdup( input );

      rtrim( trimmedInput, truncateChars, 3 );

      index = 0;

      next = trimmedInput[ index ];

      lhs = NULL;
      rhs = NULL;

      lastDot = NULL;

      i = 0;

      while ( '\0' != next )
      {
         if ( NULL == lhs ) 
         {
            if ( ( ( 'A' <= next ) && ( 'Z' >= next ) )
                 || ( ( 'a' <= next ) && ( 'z' >= next ) ) ) 
            {
               lhs = &( trimmedInput[ index ] );
            }
         }
         else if ( ( '\\' == next ) || ( '/' == next ) )
         {
            lhs = NULL;
            rhs = NULL;

            lastDot = NULL;
         }
         else if ( '.' == next )
         {
            lastDot = &( trimmedInput[ index ] );

            rhs = NULL;
         }
         else if ( ( NULL != lastDot ) && ( NULL == rhs ) )
         {
            rhs = &( trimmedInput[ index ] );
         }         

         ++index;

         next = trimmedInput[ index ];
      }

      if ( NULL != lhs )
      {
         index = 0;

         while ( index < 8 )
         {
            result[ index ] = *lhs;

            ++lhs;
            ++index;

            if ( ( '\0' == *lhs ) || ( lhs == lastDot ) )
            {
               break;
            }
         }

         restrictToAllowedCharacters( result, '_' );

         if ( NULL != rhs )
         {
            result[ index++ ] = '.';

            i = 0;

            while ( i < 3 )
            {
               result[ index + i ] = *rhs;

               ++rhs;
               ++i;

               if ( '\0' == *rhs )
               {
                  break;
               }
            }

            restrictToAllowedCharacters( &( result[ index ] ), '_' );
         }

         strcpy( input, result );
      }
      else
      {
         *input = '\0';
      }

      free( trimmedInput );
   }
}


void createSubdirectory( char* path )
{
   struct _stat info = { 0 };

   char* index;
   char  separator;

   if ( 0 != _stat( path, &info ) )
   {
      errno = 0;

      index = path;

      while ( *index )
      {
         if ( ( '\\' == *index ) || ( '/' == *index ) )
         {
            separator = *index;

            *index = '\0';

            if ( 0 != _stat( path, &info ) )
            {
               errno = 0;

               _mkdir( path );
            }

            *index = separator;
         }

         ++index;
      }
   }
}


void saveAs( char* savePrompt, struct StripSequenceType* sequence )
{
   char* index;

   char* filename = NULL;

   char outputFilename[ 255 ] = { 0 };

   char outputPath[ _MAX_PATH ] = { 0 };

   struct _stat fileinfo = { 0 };

   char selection = 'N';

   FILE* output = NULL;

   bool fileWrittenOrAborted = false;

   char option[] =
   {
      'Y', 'y', 'N', 'n', 'X', 'x'
   };

   char truncateChars[] =
   {
      ' ',
      '\t'
   };

   size_t bytesWritten;

   do
   {
      printf( savePrompt );

      if ( NULL != fgets( outputFilename, sizeof( outputFilename ) / sizeof( outputFilename[ 0 ] ) - 1, stdin ) )
      {
         filename = ltrim( outputFilename, truncateChars, sizeof( truncateChars ) / sizeof( truncateChars[ 0 ] ) );

         normalize( filename );

         if ( '\0' != settings.outputDirectory[ 0 ] )
         {
            strcpy( outputPath, settings.outputDirectory );

            index = outputPath + strlen( outputPath );
         }
         else
         {
            getcwd( outputPath, _MAX_PATH - 1 );

            index = outputPath + strlen( outputPath );

            if ( index > outputPath )
            {
               --index;

               if ( ( '/' != *index ) && ( '\\' != *index ) )
               {
                  ++index;

                  *index = '\\';

                  ++index;
               }
            }
         }

         strcpy( index, filename );

         if ( 0 == _stat( outputPath, &fileinfo ) )
         {
            selection =
               prompt( "Overwrite existing file (Y|N|X)? ", option, sizeof( option ) / sizeof( option[ 0 ] ),
                       &( option[ sizeof( option ) / sizeof( option[ 0 ] ) - 1 ] ) );
         }
         else
         {
            selection = 'Y';
         }

         if ( ( 'Y' == selection ) || ( 'y' == selection ) )
         {
            output = fopen( outputPath, "wb" );

            bytesWritten =
               fwrite( &( sequence->scan.buffer[ sequence->scan.startIndex ] ), 1,
                       sequence->scan.bytesReceived - sequence->scan.startIndex, output );

            fclose( output ); 

            printf( "Wrote %u bytes of terminal file at\n   %s\n\n", bytesWritten, outputPath );

            fileWrittenOrAborted = true;
         }
         else if ( ( 'X' == selection ) || ( 'x' == selection ) )
         {
            printf( "Write terminal file aborted\n\n" );

            fileWrittenOrAborted = true;
         }
      }
   }
   while ( false == fileWrittenOrAborted );   
}
