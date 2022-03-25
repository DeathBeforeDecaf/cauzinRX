#include <errno.h> // errno
#include <fcntl.h>  // _O_RDONLY
#include <string.h> // memset(), strlen(), strcpy(), strncpy()
#include <time.h> // clock_t, clock()

#include "crc16.h"
#include "cznstrip.h"
#include "serlib.h"
#include "support.h"
#include "syscfg.h"

extern struct SystemConfiguration settings;

// According to the StripWare Stripper manual:
//    High density puts 1065 bytes on each strip
//    Medium density puts 844 bytes on each strip

void displayFileInformation( struct StripSequenceType* sequence )
{
   uint8_t j;

   printf( ">>> Strip Sequence Content\n" );

   for ( j = 0; j < sequence->lStrip.prefix.file_count; j++ )
   {
      printf( "   %s (%u bytes)", sequence->lStrip.file[ j ].entry.name,  sequence->lStrip.file[ j ].sizeBytes );

      switch ( sequence->lStrip.file[ j ].entry.category )
      {
         case 0x00:
            if ( 0x00 != sequence->lStrip.prefix.host_os )
            {
               printf( " t: unk" );
            }
            else
            {
               printf( " t: txt" );
            }
            break;

         case 0x01:
            {
               printf( " t: txt" );
            }
            break;

         case 0x02:
            {
               printf( " t: bin" );
            }
            break;

         case 0x04:
            {
               printf( " t: bas" );
            }
            break;

         case 0x10:
            {
               printf( " t: zip" );
            }
            break;
      }

      if ( 0x10 == sequence->lStrip.prefix.host_os ) // Apple DOS 3.3
      {
         switch ( sequence->lStrip.file[ j ].entry.os_type )
         {            
            case 0x00: printf( " Apple text file\n" );
               break;

            case 0x01:
            case 0x02: printf( " Apple basic file\n" );
               break;

            case 0x04: printf( " Apple binary file\n" );
               break;

            case 0x10: printf( " Apple object file\n" );
               break;

            case 0x20: printf( " Apple A type file\n" );
               break;

            case 0x40: printf( " Apple B type file\n" );
               break;
                                 
            default:
               printf( "\n" );
         }
      }
      else if ( 0x11 == sequence->lStrip.prefix.host_os ) // Apple ProDOS
      {
         switch ( sequence->lStrip.file[ j ].entry.os_type )
         {            
            case 0x04: printf( " ProDOS text file\n" );
               break;

            case 0x06: printf( " ProDOS binary file\n" );
               break;

            case 0xFA:
            case 0xFC: printf( " ProDOS text file\n" );
               break;

            case 0xFE: printf( " ProDOS object file\n" );
               break;

            case 0xFF: printf( " ProDOS system file\n" );
               break;

            default:
               printf( "\n" );
         }
      }
      else if ( ( 0x14 == sequence->lStrip.prefix.host_os )
                || ( 0x20 == sequence->lStrip.prefix.host_os ) ) // PC/MS-DOS
      {
         switch ( sequence->lStrip.file[ j ].entry.os_type )
         {            
            case 0x00: printf( " MS-DOS executable file\n" );
               break;

            case 0x01: printf( " MS-DOS text file\n" );
               break;

            default:
               printf( "\n" );
         }
      }
      else if ( 0x15 == sequence->lStrip.prefix.host_os ) // Macintosh
      {
         switch ( sequence->lStrip.file[ j ].entry.os_type )
         {            
            case 0x00: printf( " Macintosh binary file\n" );
               break;

            case 0x01: printf( " Macintosh text file\n" );
               break;

            default:
               printf( "\n" );
         }
      }
      else
      {
         printf( "\n" );
      }
   }
}

enum CZNRXResult parseStripContent( bool isImmediate, uint16_t expectedStripNumber, struct StripSequenceType* sequence )
{
   enum CZNRXResult result = unknown_result;

   uint8_t previous;
   uint8_t latest = 0;
   char* address;

   uint16_t checksumAccumulator;

   uint16_t fieldWidth = 0;

   uint8_t  fileCounter = 0;

   bool hasCRCSuffix = false;

   uint16_t startIndex = sequence->scan.startIndex;
   uint16_t bytesReceived = sequence->scan.bytesReceived;

   uint16_t contentThreshold = 0;

   enum SegmentType expectedSegment = strip_length;

   uint16_t stripSizeBytes = 0;
   uint8_t  stripChecksum = 0;
   uint8_t  stripNumber = 0;
   uint8_t  stripAdjunctSize = 0;
   char     stripID[ 7 ] = { 0 };

   struct DataStripType* dStrip = NULL;
   struct DataStripType* nextStrip = NULL;

   uint16_t i;
   uint8_t j;

   for ( i = sequence->scan.startIndex; i < bytesReceived; i++ )
   {
      previous = latest;
      address = &( sequence->scan.buffer[ i ] );
      latest = *address;

      // aggregate checksum
      switch ( expectedSegment )
      {
         case strip_id:
         case sequence_number:
         case strip_type:
         case attributes:

         case host_os:
         case file_count:
         case category: 
         case os_type:
         case file_length:
         case file_name:
         case adjunct:
         case trailer_crc:
            checksumAccumulator =
               0x01FF & ( ( checksumAccumulator >> 8 ) + ( 0xFF & checksumAccumulator ) + latest );
            break;

         case body:
            {
               if ( 0 == fieldWidth )
               {
                  sequence->stripContent = i;

                  ++fieldWidth;
               }
            
               if ( ( i < contentThreshold ) && ( 0 < sequence->remainingTotalBytes ) )
               {
                  checksumAccumulator =
                     0x01FF & ( ( checksumAccumulator >> 8 ) + ( 0xFF & checksumAccumulator ) + latest );
               }
               else
               {
                  if ( false == hasCRCSuffix )
                  {
                     expectedSegment = end_of_strip;

                     fieldWidth = 0;
                  }
                  else
                  {
                     expectedSegment = trailer_crc;

                     checksumAccumulator =
                        0x01FF & ( ( checksumAccumulator >> 8 ) + ( 0xFF & checksumAccumulator ) + latest );

                     fieldWidth = 0;                  
                  }
               }
            }
            break;
      }


      switch ( expectedSegment )
      {
         // field 3.4.5 stripSizeBytes uint16_t
         case strip_length:
         {
            if ( 0 != fieldWidth++ )
            {
               stripSizeBytes = ( ( latest << 8 ) | previous );

               expectedSegment = checksum;

               fieldWidth = 0;
            }
         }
            break;

         // field 3.4.6 Checksum uint8_t
         case checksum:
         {
            sequence->stripStart = i;

            stripChecksum = latest;

            checksumAccumulator = 0;

            expectedSegment = strip_id;
         }
            break;

         // field 3.4.7 StripID char[ 6 ]
         case strip_id:
         {
            stripID[ fieldWidth++ ] = ( int8_t )latest;

            if ( fieldWidth > 5 )
            {
               expectedSegment = sequence_number;

               fieldWidth = 0;
            }
         }
            break;

         // field 3.4.8 StripNumber uint8_t
         case sequence_number:
         {
            stripNumber = ( uint8_t )( 0x7F & latest );

            if ( ( 1 < stripNumber ) && ( 0 != strncmp( stripID, sequence->stripID, 6 ) ) )
            {
fprintf( stderr, "DEBUG: Expected StripID: %s, Actual StripID: %s\n", sequence->stripID, stripID );

               return err_strip_id_mismatch;   
            }

            if ( stripNumber != expectedStripNumber )
            {
fprintf( stderr, "DEBUG: Expected StripNumber: %u, Actual StripNumber: %u\n", expectedStripNumber, stripNumber );

               return err_out_of_sequence;
            }

            if ( bytesReceived < stripSizeBytes )
            {
               return err_truncated_strip;
            }

            if ( 1 < stripNumber )
            {
               nextStrip = ( struct DataStripType* )calloc( 1, sizeof( struct DataStripType ) );

               if ( NULL != nextStrip )
               {
                  if ( 2 < stripNumber )
                  {
                     dStrip = sequence->lStrip.nextStrip;
                     
                     for ( j = 2; j < ( stripNumber - 1 ); j++ )
                     {
                        dStrip = dStrip->nextStrip;   
                     }

                     dStrip->nextStrip = nextStrip;
                  }
                  else // stripNumber 2
                  {
                     sequence->lStrip.nextStrip = nextStrip;
                  }

                  dStrip = nextStrip;

                  dStrip->prefix.header.number = stripNumber;

                  memcpy( dStrip->prefix.header.id, stripID, 6 );

                  dStrip->prefix.header.checksum = stripChecksum;

                  dStrip->prefix.length = stripSizeBytes;

                  sequence->stripEnd = sequence->stripStart + stripSizeBytes;
               }
               else
               {
                  return err_memory_insufficient;
               }               
            }
            else
            {
               sequence->lStrip.prefix.header.number = stripNumber;

               memcpy( sequence->lStrip.prefix.header.id, stripID, 6 );

               strncpy( sequence->stripID, stripID, 6 );

               sequence->lStrip.prefix.header.checksum = stripChecksum;

               sequence->lStrip.prefix.length = stripSizeBytes;

               sequence->stripEnd = sequence->stripStart + stripSizeBytes;

               strcpy( sequence->workingPath, settings.workingDirectory );
            }

            expectedSegment = strip_type;
         }
            break;

         // field 3.4.9 StripType uint8_t: 0x00 = standard, 0x01 = key_strip (proprietary hardware) 
         case strip_type:
         {
            if ( 1 < stripNumber )
            {
               dStrip->prefix.header.type = latest;
            }
            else
            {
               sequence->lStrip.prefix.header.type = latest;
            }

            expectedSegment = attributes;
         }
            break;

         // field 3.4.10 SoftwareExpansion uint16_t
         case attributes:
         {
            if ( 0 != fieldWidth++ )
            {
               if ( 0x8000 & ( ( latest << 8 ) | previous ) )
               {
                  hasCRCSuffix = true;

                  crc16_clear();

                  contentThreshold = sequence->stripEnd - 2;
               }
               else
               {
                  hasCRCSuffix = false;

                  contentThreshold = sequence->stripEnd;
               }

               if ( 1 < stripNumber )
               {
                  dStrip->prefix.header.attributes = ( ( latest << 8 ) | previous );

                  expectedSegment = body;
               }
               else
               {
                  sequence->lStrip.prefix.header.attributes = ( ( latest << 8 ) | previous );

                  expectedSegment = host_os;
               }

               fieldWidth = 0;
            }
         }
            break;

         // field 3.4.11 OperatingSystem uint8_t
         case host_os:
         {
            sequence->lStrip.prefix.host_os = latest;

            expectedSegment = file_count;
         }
            break;

         // field 3.4.12 NumberFiles
         case file_count:
         {
            sequence->lStrip.prefix.file_count = latest;

            sequence->lStrip.file = calloc( ( uint8_t )latest, sizeof( struct FileType ) );

            if ( NULL == sequence->lStrip.file )
            {
               return err_memory_insufficient;
            }

            expectedSegment = category;
         }
            break;

         // field 3.4.13 CauzinType uint8_t
         case category:
         {
            sequence->lStrip.file[ fileCounter ].entry.category = latest;

            expectedSegment = os_type;
         }
            break;

         // field 3.4.14 OS FileType uint8_t
         case os_type:
         {
            sequence->lStrip.file[ fileCounter ].entry.os_type = latest;

            expectedSegment = file_length;
         }
            break;

         // field 3.4.15 FileLength uint24_t
         case file_length:
         {
            ++fieldWidth;

            if ( 2 == fieldWidth )
            {
               sequence->lStrip.file[ fileCounter ].sizeBytes = ( ( ( ( uint16_t )latest ) << 8 ) | previous );
            }
            else if ( 3 == fieldWidth ) 
            {
               sequence->lStrip.file[ fileCounter ].sizeBytes |= ( ( ( uint32_t )latest ) << 16 );

               sequence->lStrip.file[ fileCounter ].remainingBytes = sequence->lStrip.file[ fileCounter ].sizeBytes;

               expectedSegment = file_name;

               fieldWidth = 0;
            }
         }
            break;

         // field 3.4.16 FileName char[] ('\0' termination not guaranteed)
         // field 3.4.17 Terminator
         case file_name:
         {
            if ( 0 == fieldWidth )
            {
               sequence->lStrip.file[ fileCounter ].name = ( char* )address;
            }

            ++fieldWidth;

            if ( ( 0 == latest ) || ( 0xFF == latest ) )
            {
               sequence->lStrip.file[ fileCounter ].entry.terminator = ( uint8_t )latest;

               if ( 0 != latest ) // normalize file_name string terminator
               {
                  sequence->scan.buffer[ i ] = '\0';
               }

               sequence->lStrip.file[ fileCounter ].entry.name = strdup( sequence->lStrip.file[ fileCounter ].name );

               sequence->lStrip.file[ fileCounter ].name = sequence->lStrip.file[ fileCounter ].entry.name;

               if ( NULL == sequence->lStrip.file[ fileCounter ].entry.name )
               {
                  return err_memory_insufficient;
               }

               expectedSegment = adjunct;

               fieldWidth = 0;
            }
         }
            break;

         // field 3.4.18 BlockExpandSizeBytes uint8_t
         //              BlockExpand optional uint8_t[] (0-255 bytes)
         case adjunct:
         {
            if ( 0 == fieldWidth )
            {
               stripAdjunctSize = latest;

               sequence->lStrip.file[ fileCounter ].entry.adjunct_size = stripAdjunctSize;

               if ( 0 != stripAdjunctSize )
               {
                  sequence->lStrip.file[ fileCounter ].entry.adjunct = ( uint8_t *)calloc( 1, stripAdjunctSize );

                  if ( NULL == sequence->lStrip.file[ fileCounter ].entry.adjunct )
                  {
                     return err_memory_insufficient;
                  }

                  ++fieldWidth;
               }
               else  // sans adjunct
               {
                  ++fileCounter;

                  if ( fileCounter < sequence->lStrip.prefix.file_count )
                  {
                     expectedSegment = category;
                  }
                  else // last file block consumed
                  {
                     for ( j = 0; j < fileCounter; j++ )
                     {
                        sequence->expectedTotalBytes += sequence->lStrip.file[ j ].sizeBytes;
                     }

                     sequence->remainingTotalBytes = sequence->expectedTotalBytes;

                     if ( false == isImmediate )
                     {
                        displayFileInformation( sequence );
                     }

                     expectedSegment = body;
                  }
               }
            }
            else
            {
               sequence->lStrip.file[ fileCounter ].entry.adjunct[ fieldWidth - 1 ] = latest;

               if ( fieldWidth <= ( uint16_t )stripAdjunctSize )
               {
                  ++fieldWidth;
               }
               else
               {
                  if ( fileCounter < sequence->lStrip.prefix.file_count )
                  {
                     expectedSegment = category;

                     fieldWidth = 0;
                  }
                  else // last file block consumed
                  {
                     for ( j = 0; j < fileCounter; j++ )
                     {
                        sequence->expectedTotalBytes += sequence->lStrip.file[ j ].sizeBytes;
                     }

                     sequence->remainingTotalBytes = sequence->expectedTotalBytes;

                     expectedSegment = body;

                     fieldWidth = 0;
                     fileCounter = 0;

                     if ( false == isImmediate )
                     {
                        displayFileInformation( sequence );
                     }
                  }
               }
            }
         }
            break;

         case body:
            if ( hasCRCSuffix )
            {
               crc16_put( latest );
            }
            break;


         case trailer_crc:
         {
            if ( 0 != fieldWidth++ )
            {
               if ( 1 < stripNumber )
               {
                  dStrip->optionalCRC = ( ( ( ( uint16_t )latest ) << 8 ) | previous );
               }
               else
               {
                  sequence->lStrip.optionalCRC = ( ( ( ( uint16_t )latest ) << 8 ) | previous );
               }

               expectedSegment = end_of_strip;

               fieldWidth = 0;
            }
         }
            break;


         case end_of_strip:
         {
            ++fieldWidth;
            
            //  CFT: 0x00
            // STAT: 0x0A or 0x08

            if ( fieldWidth < 2 )
            {
               if ( 0 != latest ) // CTF (0x00)
               {
                  fprintf( stderr, "\nERROR: CTF after body not found at end of strip.\n" );
               }
            }
            else
            {
               if ( 2 == fieldWidth )
               {
                  if ( 0x0A == latest ) // Strip Read Success (0x0A)
                  {
                     // finalize calculated checksum (twos complement) and verify w/strip value
                     checksumAccumulator =
                        ( ~( ( checksumAccumulator >> 8 )
                           + ( 0xFF & checksumAccumulator ) ) + 1 ) & 0xFF;

                     result = success_result;

                     if ( 1 < stripNumber )
                     {
                        if ( checksumAccumulator != dStrip->prefix.header.checksum )
                        {
                           result = err_checksum_mismatch;
                        }

                        if ( hasCRCSuffix && ( crc16_get() != dStrip->optionalCRC ) )
                        {
                           result = err_crc_mismatch;
                        }
                     }
                     else
                     {
                        if ( checksumAccumulator != sequence->lStrip.prefix.header.checksum )
                        {
                           result = err_checksum_mismatch;
                        }

                        if ( hasCRCSuffix && ( crc16_get() != sequence->lStrip.optionalCRC ) )
                        {
                           result = err_crc_mismatch;
                        }
                     }

                     if ( success_result == result )
                     {
                        if ( ( sequence->stripEnd - sequence->stripContent ) <= sequence->remainingTotalBytes )
                        {
                           sequence->remainingTotalBytes -= ( sequence->stripEnd - sequence->stripContent );
                        }
                        else // more content in strip(s) than in files aggregate
                        {
                           sequence->remainingTotalBytes = 0;
                        }
                     }

                     expectedSegment = trailer_extraneous;

                     fieldWidth = 0;
                  }
               }

               if ( 3 == fieldWidth )
               {
                  expectedSegment = trailer_extraneous;

                  fieldWidth = 0;

                  if ( 0x08 == previous )
                  {
                     switch ( latest )
                     {
                        case 0x00:
                           result = err_line_unreadable;
                           break;

                        case 0x01:
                           result = err_horz_sync;
                           break;

                        case 0x02:
                           result = err_top_too_close;
                           break;

                        case 0x03:
                           result = err_invalid_expansion;
                           break;

                        case 0x04:
                           result = err_course_tilt;
                           break;

                        case 0x05:
                           result = err_vert_sync;
                           break;

                        case 0x06:
                           result = err_data_sync;
                           break;

                        case 0x07:
                           result = err_fine_tilt;
                           break;

                        case 0x08:
                           result = err_tilt_measurement;
                           break;

                        case 0x09:
                           result = err_timer_exceeded;
                           break;
                     }
                  }
               }
            }
         }
            break;


         case trailer_extraneous:
         {
            ++fieldWidth;
////
//          if ( 0 != fieldWidth % 8 )
//          {
//             printf( "0x%02X ", latest );
//          }
//          else
//          {
//             printf( "0x%02X\n", latest );
//          }
////
         }
            break;
      }
   }

   return result;
}


// consume serial output from reader and set the startIndex to point at the byte after command acknowledgement
enum CZNRXResult readSoftstrip( bool isImmediate, uint16_t stripNumber, struct StripSequenceType* sequence )
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

   char option[] =
   {
      ' ',
      'Q', 'q', 0x1B // ESC
   };

   char selection;

   // prompt for the data strip reads
   if ( ( 1 < stripNumber ) && ( NULL != sequence->stripID ) )
   {
      printf( "\n   After reader rewinds, press a key to scan strip #%d of %s\n", stripNumber, sequence->stripID );

      selection = prompt( "   (ESC/q/Q to quit):> ", option, sizeof( option ) / sizeof( option[ 0 ] ), option );

      switch( selection )
      {
         case 0x1B:
         case 'q' :
         case 'Q' :
         {
            printf( "\n   Read Strip Sequence: USER CANCELED\n" );
            printf( "   user canceled optical strip read and processing\n\n" );

            result = usr_canceled;

            return result;
         }
            break;
      } 
   }

   Attach_Buffer( &( sequence->scan ) );

   byteCounter = 0;

   Serial_Write( 'R' );

   first = clock();

   next = 0;

   latch = 0;

   index = 0;


   // timeout after ~6 seconds of waiting without the buffer increasing
   for ( i = 0; i < ( uint16_t)( settings.latchThreshold * settings.timeoutThreshold ); i++ )
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
/*
      // no initial reader response within 2/3 timeout
      if ( ( latch < settings.latchThreshold )
           && ( ( ( ( ( uint16_t )settings.latchThreshold ) * settings.timeoutThreshold ) * 2 ) / 3 == i ) )
      {
         Detach_Buffer( NULL );

         result = err_bad_transmission;

         return result;
      }
*/
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

   // optical reader acknowleges command request
   if ( ( index < sequence->scan.bytesReceived ) && ( 'R' == sequence->scan.buffer[ index ] ) )
   {
      sequence->scan.isAcknowledged = true;
                                      
      index++;
   }

   sequence->scan.startIndex = index;

   // check scan buffer for strip read status
   if ( 0 < index )
   {
      if ( 0x0A == sequence->scan.buffer[ sequence->scan.bytesReceived - 1 ] )
      {
         result = success_result;
      }
      else if ( ( 1 < index ) && ( 0x08 == sequence->scan.buffer[ sequence->scan.bytesReceived - 2 ] )
                && ( 0 <= sequence->scan.buffer[ sequence->scan.bytesReceived - 1 ] )
                && ( 10 > sequence->scan.buffer[ sequence->scan.bytesReceived - 1 ] ) )
      {
         // error code / error status
         switch( sequence->scan.buffer[ sequence->scan.bytesReceived - 1 ] )
         {
            case 0:
               result = err_line_unreadable;    // 0: corridor closed (recoverable)
               break;

            case 1:
               result = err_horz_sync;          // 1: horizontal synchronization (recoverable)
               break;

            case 2:
               result = err_top_too_close;      // 2: strip too close to top (recoverable)
               break;

            case 3:
               result = err_invalid_expansion;  // 3: invalid expansion bytes (non-recoverable)
               break;

            case 4:
               result = err_course_tilt;        // 4: T1 too small (recoverable)
               break;

            case 5:
               result = err_vert_sync;          // 5: vertical synchronization not established (recoverable?)
               break;

            case 6:
               result = err_data_sync;          // 6: data synchronization not established (recoverable?)
               break;

            case 7:
               result = err_fine_tilt;          // 7: tilt > 2.0 scans (recoverable)
               break;

            case 8:
               result = err_tilt_measurement;   // 8: tilt could not be measured - very course tilt (recoverable)
               break;

            case 9:
               result = err_timer_exceeded;     // 9: watchdog timer runout (recoverable - check power)
               break;
         }
      }
      else
      {
         // error (bad transmission - communication breakdown)
         result = err_bad_transmission;
      }
   }

   return result;
}


void saveStripSequenceData( bool isImmediate, uint16_t stripNumber, struct StripSequenceType* sequence )
{
   bool pathChanged = false;

   char normalizedID[ 7 ] = { 0 };

   uint8_t fileIndex = 0;
   uint8_t i;

   char* index;

   char startingDirectory[ _MAX_PATH + 1 ] = { 0 };

   struct DataStripType* dStrip;

   FILE* output = NULL;

   uint16_t stripRemainingBytes =
      ( sequence->stripEnd > sequence->stripContent )
      ? ( sequence->stripEnd - sequence->stripContent ) : 0;

   _getcwd( startingDirectory, _MAX_PATH );

   if ( 1 < stripNumber )
   {
      dStrip = sequence->lStrip.nextStrip;

      for ( i = 2; i < stripNumber; i++ )
      {
         dStrip = dStrip->nextStrip;
      }

      // ?subtract 2 bytes for trailing CRC16
      if ( ( 0x8000 & dStrip->prefix.header.attributes ) && ( 1 < stripRemainingBytes ) )
      {
         stripRemainingBytes -= 2;
      }

      index = sequence->workingPath + strlen( sequence->workingPath );

      if ( sequence->workingPath < index )
      {
         --index;
      }

      *index = '\0';

      if ( 0 == _chdir( sequence->workingPath ) )
      {
         pathChanged = true;
      }

      *index = '\\';
   }
   else
   {
      // ?subtract 2 bytes for trailing CRC16
      if ( ( 0x8000 & sequence->lStrip.prefix.header.attributes ) && ( 1 < stripRemainingBytes ) )
      {
         stripRemainingBytes -= 2;
      }

      strncpy( normalizedID, sequence->stripID, 6 );

      normalize( normalizedID );

      if ( 0 < strlen( normalizedID ) )
      {
         // make a subdirectory with stripID
         strcpy( sequence->workingPath + strlen( sequence->workingPath ), normalizedID );

         index = sequence->workingPath + strlen( sequence->workingPath );

         *index = '\\';

         createSubdirectory( sequence->workingPath );

         errno = 0;

         *index = '\0';

         if ( 0 == _chdir( sequence->workingPath ) )
         {
            pathChanged = true;
         }

         *index = '\\';
      }
   }

   if ( true == pathChanged )
   {
      do
      {
         for ( i = fileIndex; i < sequence->lStrip.prefix.file_count; i++ )
         {
            if ( 0 != sequence->lStrip.file[ i ].remainingBytes )
            {
               fileIndex = i;

               break;
            }
         }

         if ( i < sequence->lStrip.prefix.file_count )
         {
            createSubdirectory( sequence->lStrip.file[ i ].entry.name );

            if ( sequence->lStrip.file[ i ].sizeBytes > sequence->lStrip.file[ i ].remainingBytes )
            {
               // append existing file segment
               output = fopen( sequence->lStrip.file[ i ].entry.name, "ab" );
            }
            else
            {
               output = fopen( sequence->lStrip.file[ i ].entry.name, "wb" );
            }

            if ( NULL != output )
            {
               // most strips are smaller than most files these days...
               if ( stripRemainingBytes < sequence->lStrip.file[ i ].remainingBytes )
               {
                  fwrite( sequence->scan.buffer + sequence->stripContent, 1, stripRemainingBytes, output );

                  sequence->lStrip.file[ i ].remainingBytes -= stripRemainingBytes;

                  stripRemainingBytes = 0;
               }
               else
               {
                  fwrite( sequence->scan.buffer + sequence->stripContent,
                          1, ( size_t )sequence->lStrip.file[ i ].remainingBytes, output );

                  sequence->stripContent += ( uint16_t )sequence->lStrip.file[ i ].remainingBytes;

                  stripRemainingBytes -= ( uint16_t )sequence->lStrip.file[ i ].remainingBytes;

                  sequence->lStrip.file[ i ].remainingBytes = 0;
               }

               fclose( output );
            }
         }
      }
      while ( 0 < stripRemainingBytes );

      _chdir( startingDirectory );
   }
}

/*
// Example: COPY2.C

#include <dos.h>
#include <fcntl.h>
#include <conio.h>
#include <stdio.h>

int copyFile( char *source, char *destination );     /* Prototype */

// Copies one file to another (both specified by path). Dynamically
// allocates memory for the file buffer. Prompts before overwriting
// existing file. Returns 0 if successful, or an error number if
// unsuccessful. This function uses _dos_ functions only; standard
// C functions are not used.

#define EXIST 80

enum ATTRIB { NORMAL, RDONLY, HIDDEN, SYSTEM = 4 };

/*
#define _A_NORMAL   0x00    // Normal file - No read/write restrictions
#define _A_RDONLY   0x01    // Read only file
#define _A_HIDDEN   0x02    // Hidden file
#define _A_SYSTEM   0x04    // System file
#define _A_VOLID    0x08    // Volume ID file
#define _A_SUBDIR   0x10    // Subdirectory
#define _A_ARCH     0x20    // Archive file
*/

int copyFile( char* source, char* target )
{
   char __far* raw = NULL;

   char prompt[] = "Target exists. Overwrite? ";
   char newline[] = "\n\r";

   int hsource;
   int htarget;

   int ch;

   uint16_t count;
   uint16_t result;
   uint16_t segmentBuffer;

   // Attempt to dynamically allocate all of memory which is expected to fail,
   // but will return the amount actually available in segmentBuffer.

   _dos_allocmem( 0xFFFF, &segmentBuffer );

   count = segmentBuffer;

   if ( result = _dos_allocmem( count, &segmentBuffer ) )
   {
      return result;
   }

   _FP_SEG( raw ) = segmentBuffer;

   // Open source file and create target, overwriting if necessary.
   if ( result = _dos_open( source, _O_RDONLY, &hsource ) )
   {
      return result;
   }

   result = _dos_creatnew( target, _A_NORMAL, &htarget );

   if ( result == EXIST )
   {
      // Use _dos_write to display prompts. Use _bdos to call
      // function 1 to get and echo keystroke.

      _dos_write( 1, prompt, sizeof( prompt ) - 1, &ch );

      ch = _bdos( 1, 0, 0 ) & 0x00FF;

      if ( ( 'y' == ch ) || ( 'Y' == ch ) )
      {
         result = _dos_creat( target, _A_NORMAL, &htarget );
      }

      _dos_write( 1, newline, sizeof( newline ) - 1, &ch );
   }

   if ( result )
   {
      return result;
   }

   // Read and write until there is nothing left.
   while ( count )
   {
      // Read and write input.
      if ( ( result = _dos_read( hsource, raw, count, &count ) ) )
      {
         return result;
      }

      if ( ( result = _dos_write( htarget, raw, count, &count ) ) )
      {
         return result;
      }
   }

   // Close files and free memory.
   _dos_close( hsource );
   _dos_close( htarget );

   _dos_freemem( segmentBuffer );

   return 0;
}



void mergeStripFileData( bool isImmediate, struct StripSequenceType* sequence )
{
   bool pathChanged = false;

   char normalizedID[ 7 ] = { 0 };

   uint8_t fileIndex = 0;
   uint8_t i;

   char* index;

   char sequenceDirectory[ _MAX_PATH + 1 ] = { 0 };

   char startingDirectory[ _MAX_PATH + 1 ] = { 0 };

   char* source;
   char* destination;

   _getcwd( startingDirectory, _MAX_PATH );

   if ( '\0' != settings.outputDirectory )
   {
      strcpy( sequenceDirectory, settings.outputDirectory );
   }
   else
   {
      strcpy( sequenceDirectory, startingDirectory );

      index = sequenceDirectory + strlen( sequenceDirectory );

      *index = '\\';
   }

   if ( sequence->lStrip.prefix.file_count > 1 )
   {
      strncpy( normalizedID, sequence->stripID, 6 );

      normalize( normalizedID );

      if ( 0 < strlen( normalizedID ) )
      {
         // make a subdirectory with stripID
         strcpy( sequenceDirectory + strlen( sequenceDirectory ), normalizedID );

         index = sequenceDirectory + strlen( sequenceDirectory );

         *index = '\\';

         createSubdirectory( sequenceDirectory );

         errno = 0;

         *index = '\0';

         if ( 0 == _chdir( sequenceDirectory ) )
         {
            pathChanged = true;
         }

         *index = '\\';
      }
   }

   for ( i = fileIndex; i < sequence->lStrip.prefix.file_count; i++ )
   {
      source = malloc( 1 + strlen( sequence->workingPath ) + strlen( sequence->lStrip.file[ i ].entry.name ) );
      destination = malloc( 1 + strlen( sequenceDirectory ) + strlen( sequence->lStrip.file[ i ].entry.name ) );

      if ( NULL != source )
      {
         strcpy( source, sequence->workingPath );
         strcat( source, sequence->lStrip.file[ i ].entry.name );
      }

      if ( NULL != destination )
      {
         strcpy( destination, sequenceDirectory );
         strcat( destination, sequence->lStrip.file[ i ].entry.name );
      }

      if ( ( NULL != source ) && ( NULL != destination ) )
      {
         createSubdirectory( destination );

         fprintf( stderr, "   Writing: %s\n", destination );

         copyFile( source, destination );
      }

      free( source );
      free( destination );
   }

   if ( true == pathChanged )
   {
      _chdir( startingDirectory );
   }
}

// Allocate space for serial buffer, serial envelope and first (logical) strip
// Read the first strip in the sequence and parse it's directory content
//    Write file content to disk
// Read and parse additional data strip information as necessary
//    Write file content to disk
// When strip sequence read is complete and successful
//    Conditionally merge file content
//    Deallocate all heap allocations in reverse chronological order
//    Remove intermediate files

void relinquishStripData( uint16_t stripNumber, struct StripSequenceType* sequence )
{
   char* index;
   char* source;
   char* destination;

   struct DataStripType* dStrip;
   struct DataStripType* nextStrip;
   struct DataStripType* prevStrip = NULL;

   uint8_t fileIndex;

   if ( 1 < stripNumber )
   {
      dStrip = sequence->lStrip.nextStrip;

      while ( ( 2 < stripNumber ) && ( NULL != dStrip ) )
      {
         --stripNumber;

         prevStrip = dStrip;

         dStrip = dStrip->nextStrip; 
      }

      if ( NULL != prevStrip )
      {
         prevStrip->nextStrip = NULL;
      }
      else
      {
         sequence->lStrip.nextStrip = NULL;
      }

      while ( NULL != dStrip )
      {
         nextStrip = dStrip->nextStrip;

         free( dStrip );

         dStrip = nextStrip;
      }
   }
   else
   {
      nextStrip = sequence->lStrip.nextStrip;

      while ( NULL != nextStrip )
      {
         dStrip = nextStrip;

         nextStrip = dStrip->nextStrip;

         free( dStrip );
      }

      if ( NULL != sequence->lStrip.file )
      {
         for ( fileIndex = 0; fileIndex < sequence->lStrip.prefix.file_count; fileIndex++ )
         {
            if ( NULL != sequence->lStrip.file[ fileIndex ].entry.name )
            {
               free( sequence->lStrip.file[ fileIndex ].entry.name );
            }

            if ( NULL != sequence->lStrip.file[ fileIndex ].entry.adjunct )
            {
               free( sequence->lStrip.file[ fileIndex ].entry.adjunct );
            }
         }

         free( sequence->lStrip.file );
      }

      memset( &( sequence->lStrip ), 0, sizeof( struct MetaStripType ) );

      memset( &( sequence->stripID ), 0, 7 );

      sequence->expectedTotalBytes = 0;
      sequence->remainingTotalBytes = 0;

      sequence->stripStart = 0;
      sequence->stripContent = 0;
      sequence->stripEnd = 0;

      // deltree sequence->workingPath to clobber any intermediate results
      if ( '\0' != sequence->workingPath[ 0 ] )
      {
         // DELTREE_/Y_  :11
         // rshift 11
         index = sequence->workingPath;

         source = sequence->workingPath + strlen( sequence->workingPath );
         destination = source + 10;

         if ( ( destination - index ) < _MAX_PATH )
         {
            *destination = *source;

            --source;

            do
            {
               --destination;
               --source;

               *destination = *source;
            }
            while ( index != source );

            --destination;
            *destination = ' ';            
            --destination;
            *destination = 'Y';            
            --destination;
            *destination = '/';            

            --destination;
            *destination = ' ';            
            --destination;
            *destination = 'E';            
            --destination;
            *destination = 'E';            
            --destination;
            *destination = 'R';            
            --destination;
            *destination = 'T';            
            --destination;
            *destination = 'L';            
            --destination;
            *destination = 'E';            
            --destination;
            *destination = 'D';            
         }
         
         system( sequence->workingPath );
      }

      memset( sequence->workingPath, 0, _MAX_PATH + 14 );
   } 
}


bool displayProblemHint( enum CZNRXResult hint, uint16_t stripNumber, char* stripID )
{
   bool readRepeat = true;

   switch ( hint )
   {
      case err_line_unreadable:     // 0: corridor closed (recoverable)
      {
         printf( "   Read Strip Sequence: Strip #%u Recoverable ERROR\n", stripNumber );
         printf( "   Scan line unreadable (corridor closed)\n" );
      }
         break;

      case err_horz_sync:  // 1: horizontal synchronization (recoverable)
      {
         printf( "   Read Strip Sequence: Strip #%u Recoverable ERROR\n", stripNumber );
         printf( "   Horizontal synchronization read failed\n" );
      }
         break;

      case err_top_too_close:       // 2: strip too close to top (recoverable)
      {
         printf( "   Read Strip Sequence: Strip #%u Recoverable ERROR\n", stripNumber );
         printf( "   Optical reader too close to top of strip\n" );
      }
         break;

      case err_invalid_expansion:   // 3: invalid expansion bytes (non-recoverable)
      {
         printf( "   Read Strip Sequence: Strip #%u INVALID STRIP TYPE\n", stripNumber );
         printf( "   Optical reader hardware is incompatible with the input strip\n" );

         readRepeat = false;
      }
         break;

      case err_course_tilt:// 4: T1 too small - strip margin tilt exceeded (recoverable)
      {
         printf( "   Read Strip Sequence: Strip #%u Recoverable ERROR\n", stripNumber );
         printf( "   Strip margin tilt exceeded (check alignment)\n" );
      }
         break;

      case err_vert_sync:  // 5: vertical synchronization not established (recoverable?)
      {
         printf( "   Read Strip Sequence: Strip #%u Recoverable ERROR\n", stripNumber );
         printf( "   Vertical synchronization not established\n" );
      }
         break;

      case err_data_sync:  // 6: data synchronization not established (recoverable?)
      {
         printf( "   Read Strip Sequence: Strip #%u Recoverable ERROR\n", stripNumber );
         printf( "   Data synchronization not established\n" );
      }
         break;

      case err_fine_tilt:  // 7: tilt > 2.0 scans - fine tilt in vert sync (recoverable)
      {
         printf( "   Read Strip Sequence: Strip #%u Recoverable ERROR\n", stripNumber );
         printf( "   Fine tilt exceeded in vertical synchronization (check alignment)\n" );
      }
         break;

      case err_tilt_measurement:    // 8: tilt could not be measured - very course tilt in horz sync (recoverable)
      {
         printf( "   Read Strip Sequence: Strip #%u Recoverable ERROR\n", stripNumber );
         printf( "   Coarse tilt could not be measured (check alignment)\n" );
      }
         break;

      case err_timer_exceeded:      // 9: watchdog timer runout (recoverable - check power)
      {
         printf( "   Read Strip Sequence: Strip #%u Recoverable ERROR\n", stripNumber );
         printf( "   Watchdog timer runout (check cables)\n" );
      }
         break;

      case err_out_of_sequence:     // strip number wasn't expected
      {
         printf( "   Read Strip Sequence: Recoverable ERROR - Wrong Strip Number\n" );

         if ( stripNumber > 1 )
         {
            printf( "   Please locate strip #%u of strip sequence '%s', place under reader and retry\n",
                    stripNumber, stripID );
         }
         else
         {
            printf( "   Please locate strip #%u of strip sequence, place under reader and retry\n", stripNumber );
         }
      }
         break;

      case err_strip_id_mismatch:   // expected strip id string doesn't match actual strip id
      {
         printf( "   Read Strip Sequence: Recoverable ERROR - Wrong Strip ID\n" );
         
         if ( stripNumber > 1 )
         {
            printf( "   Please place strip #%u of strip sequence '%s' under reader and retry\n",
                    stripNumber, stripID );
         }
         else
         {
            printf( "   Please place strip #%u of strip sequence under reader and retry\n", stripNumber );
         }
      }
         break;
      
      case err_truncated_strip:     // strip length > bytes received
      {
         // semantically invalid data strip (not well-formed)
         printf( "   Read Strip Sequence: TRUNCATED STRIP\n" );
         printf( "   Strip scan is not large enough to contain logical strip record\n" );

         readRepeat = false;
      }
         break;

      case err_checksum_mismatch:   // calculated checksum does not match strip checksum (field#6)
      {
         // semantically invalid data strip (not well-formed)
         printf( "   Read Strip Sequence: CHECKSUM MISMATCH\n" );
         printf( "   Strip checksum does not match checksum calculated from strip content\n" );

         readRepeat = false;
      }
         break;

      case err_crc_mismatch:   // calculated crc does not match strip crc (field#20)
      {
         // semantically invalid data strip (not well-formed)
         printf( "   Read Strip Sequence: CRC MISMATCH\n" );
         printf( "   Strip CRC16 does not match CRC16 calculated from strip content\n" );

         readRepeat = false;
      }
         break;

      case err_bad_transmission:    // communication breakdown between host and optical reader
      {
         printf( "   Read Strip Sequence: Strip #%u Recoverable ERROR\n", stripNumber );
         printf( "   Communication breakdown between host and optical reader (check cables)\n" );
      }
         break;

      case err_memory_insufficient: // unable to allocate all memory necessary for strip sequence read
      {
         printf( "   Read Strip Sequence: INSUFFICIENT MEMORY\n" );
         printf( "   unable to allocate all memory necessary for strip sequence read\n" );

         readRepeat = false;
      }
         break;

      default:
         readRepeat = false;

   };

   return readRepeat;
}


void readStripSequence( bool isImmediate )
{
   struct StripSequenceType sequence = { 0 };

   enum CZNRXResult result = unknown_result;

   bool readRepeat;

   uint16_t stripCount = 0;

   char selection;

   char option[] =
   {
      ' ',
      'Q', 'q', 0x1B // ESC
   };

   if ( false == isImmediate )
   {
      printf( ">>> Read Strip Sequence\n" );
   }

   sequence.scan.buffer = ( char* )calloc( 1, settings.maxStripSizeBytes );

   if ( NULL != sequence.scan.buffer )
   {
      sequence.scan.size = settings.maxStripSizeBytes;

      if ( 0 == settings.readerPortOpen )
      {
         openSerialPort( settings.readerPort );
      }

      readRepeat = true;

      do
      {
         while ( true == readRepeat ) 
         {
            result = readSoftstrip( isImmediate, stripCount + 1U, &sequence );

            if ( success_result == result )
            {
               result = parseStripContent( isImmediate, stripCount + 1U, &sequence );
            }
   
            // strip read and parsed successfully
            if ( success_result == result )
            {
               if ( 0 == stripCount )
               {
                  strncpy( sequence.stripID, sequence.lStrip.prefix.header.id, 6 );
               }

               printf( "      Read Strip Sequence: Strip #%u of %s\n", ( stripCount + 1U ), sequence.stripID );

               readRepeat = false;

               saveStripSequenceData( isImmediate, stripCount + 1U, &sequence );
            }
            else
            {
               readRepeat = displayProblemHint( result, stripCount + 1U, sequence.stripID );

               relinquishStripData( stripCount + 1U, &sequence );
            }

            if ( ( true == readRepeat ) && ( stripCount < 1 ) )
            {
               selection = prompt( "\nPress a key to retry (ESC/q/Q to quit):> ", option, sizeof( option ) / sizeof( option[ 0 ] ), option );

               switch( selection )
               {
                  case 0x1B:
                  case 'q' :
                  case 'Q' :
                  {
                     printf( "\n   Read Strip Sequence: USER CANCELED\n" );
                     printf( "   user canceled optical strip read and processing\n\n" );

                     result = usr_canceled;
                     readRepeat = false;
                  }
                  break;

                  default:
                     break;
               }
            }
         }

         // multistrip sequence
         if ( success_result == result )
         {
            ++stripCount;

            if ( 0 < sequence.remainingTotalBytes )
            {
               readRepeat = true;
            }
         }      
      }
      while ( true == readRepeat );

      // Read Strip Sequence post-processing
      if ( success_result == result )
      {
         mergeStripFileData( isImmediate, &sequence ); 

         relinquishStripData( 1U, &sequence );

//       remove subdirectory


      }      

   }
   else
   {
      printf( "   Read Strip Sequence: INSUFFICIENT MEMORY\n" );
      printf( "   0 bytes received\n\n" );
   }

   if ( NULL != sequence.scan.buffer )
   {
      free( sequence.scan.buffer );
   }
}
