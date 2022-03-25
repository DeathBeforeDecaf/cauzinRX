#ifndef CZSTRIP_H
#define CZSTRIP_H

#include <stdlib.h>  // MAX_PATH

#ifdef __BORLANDC__
#include "src/strprx16/serlib.h"
#include "src/strprx16/stdtypes.h"
#elif _MSC_VER
#include "serlib.h"
#include "stdtypes.h"
#endif

struct FileEntryType
{
   uint8_t   category;     // 3.4.13 (1)
   uint8_t   os_type;      // 3.4.14 (1)
// uint32_t  length : 24;  // 3.4.15 (3) file length on disk(max 16,777,215 bytes)
   char*     name;         // 3.4.16 (1<-->n bytes) null terminated filename
   uint8_t   terminator;   // 3.4.17 (1) filename terminator, 0xFF indicates executable content
   uint8_t   adjunct_size; // 3.4.18 (1) size of file metadata trailing
   uint8_t*  adjunct;      // 3.4.18+ (0-255 bytes) metadata (expansion block)

//   uint16_t  size;         // total size (in bytes) of this composite entry data
                           // 1+1+3+len(name)+1+1+len(adjunct)                                      
};


struct FileType
{
   char*                  name;
   uint32_t               sizeBytes;      // 3.4.15 (3) file length on disk(max 16,777,215 bytes) 
   uint32_t               remainingBytes; // convenience metric for byte count yet to be written
   time_t                 lastModified;   

   struct FileEntryType   entry;

   char                   path[];
};


struct HeaderSegmentType
{
   uint8_t  checksum;         // 3.4.6  (1 byte) parity-based checksum
   char     id[ 6 ];          // 3.4.7  (6 bytes)
   uint8_t  number;           // 3.4.8  (1 byte) Sequence Number (7 bit, 1-127)
   uint8_t  type;             // 3.4.9  (1 byte) Strip Type (0x00 data strip, 0x01 key strip, 0x02-0xFF undefined)
   uint16_t attributes;       // 3.4.10 (2 byte) optional CRC enabled iff ( attributes & 0x8000 )
};


struct DataPrefixType
{
   // header fields - all strips
   uint8_t  data_sync;        // 3.4.3  (1 byte)  0x00
   uint16_t reader_expansion; // 3.4.4  (2 bytes) 0x0000
   uint16_t length;           // 3.4.5  (2 bytes -> strip length)

   struct HeaderSegmentType header;
};


struct DataStripType
{
   uint8_t hSync;             // 3.4.1  0x04-0x0C (4-12 nibbles per row)
   uint8_t vSync;             // 3.4.2  0x40-0x80 (4R0*.0025") to (8R0*.0025")

   struct DataPrefixType prefix;
                                    
   uint16_t optionalCRC;      // fld20: 0 or 2 bytes

   struct DataStripType* nextStrip;
};


struct MetaPrefixType
{
   // header fields - all strips
   uint8_t  data_sync;        // 3.4.3  (1 byte)  0x00
   uint16_t reader_expansion; // 3.4.4  (2 bytes) 0x0000
   uint16_t length;           // 3.4.5  (2 bytes -> strip length)

   struct HeaderSegmentType header;

   // header fields - logical strip
   uint8_t host_os;           // 3.4.11
   uint8_t file_count;        // 3.4.12
};


struct MetaStripType
{
   uint8_t hSync;             // 3.4.1  0x04-0x0C (4-12 nibbles per row)
   uint8_t vSync;             // 3.4.2  0x40-0x80 (8R0*.0025") to (4R0*.0025")

   struct MetaPrefixType prefix;

   struct FileType* file;

   uint16_t optionalCRC;      // fld20: 0 or 2 bytes

   struct DataStripType* nextStrip;
};


struct StripSequenceType
{
   struct MetaStripType lStrip;

   char stripID[ 7 ];

   uint32_t expectedTotalBytes;
   uint32_t remainingTotalBytes;

   struct SerialRXBuffer scan;    // originating reader scan for this strip

   uint16_t stripStart;
   uint16_t stripContent;
   uint16_t stripEnd;

   char workingPath[ _MAX_PATH + 14 ]; // workingDirectory / _CAUZIN_.RX /
};


enum CZNRXSegmentType
{
   unknown_segment = 0,
   leader,
   strip_length,
   checksum,
   strip_id,
   sequence_number,
   strip_type,
   attributes,
   host_os,
   file_count,
   category,
   os_type,
   file_length,
   file_name,
   adjunct,
   body,
   trailer_crc,
   end_of_strip,
   trailer_extraneous
};


enum CZNRXResult
{
   unknown_result = 0,
   success_result,
   err_line_unreadable,     // 0: corridor closed (recoverable)
   err_horz_sync,           // 1: horizontal synchronization (recoverable)
   err_top_too_close,       // 2: strip too close to top (recoverable)
   err_invalid_expansion,   // 3: invalid expansion bytes (non-recoverable)
   err_course_tilt,         // 4: T1 too small - strip margin tilt exceeded (recoverable)
   err_vert_sync,           // 5: vertical synchronization not established (recoverable?)
   err_data_sync,           // 6: data synchronization not established (recoverable?)
   err_fine_tilt,           // 7: tilt > 2.0 scans - fine tilt in vert sync (recoverable)
   err_tilt_measurement,    // 8: tilt could not be measured - very course tilt in horz sync (recoverable)
   err_timer_exceeded,      // 9: watchdog timer runout (recoverable - check power)
   err_out_of_sequence,     // strip number wasn't expected
   err_strip_id_mismatch,   // expected strip id string doesn't match actual strip id
   err_truncated_strip,     // strip length > bytes received
   err_checksum_mismatch,   // calculated checksum does not match strip checksum (field#6)
   err_crc_mismatch,        // calculated cyclic redundency check does not match strip CRC (field#20)
   err_bad_transmission,    // communication breakdown between host and optical reader
   err_memory_insufficient, // unable to allocate all memory necessary for strip sequence read
   usr_canceled             // user canceled multi-strip read and processing
};



#endif
