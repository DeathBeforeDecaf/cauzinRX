
#ifdef __BORLANDC__
#include "src/strprx16/serlib.h"

#define   INP(x)       inp((x))
#define   OUTP(x,y)    outp((x),(y))
#elif _MSC_VER
#include "serlib.h"

#define   INP(x)       _inp((x))
#define   OUTP(x,y)    _outp((x),(y))
#endif

// retain the original com port interrupt handler
void ( _interrupt _far *prevISR )();

char ser_buffer_circ[ CIRCULAR_BUFFER_SIZE ];

char* ser_buffer = ser_buffer_circ;  // the receive buffer

uint16_t ser_buffer_size = CIRCULAR_BUFFER_SIZE;
uint16_t ser_end = 0;
uint16_t ser_start = 0;   // indexes into receive buffer
volatile uint8_t ser_busy = 0;       // flag to prevent buffer updates while ISR writes are occurring

int16_t prev_int_mask;                  // original interrupt mask on the PIC
int16_t open_port;                      // the currently open port


//////////////////////////////////////////////////////////////////////////////
//
// Interrupt Service Routine for the initialized serial port.  Retrieves the
// next character out of the receive buffer register 0 and places it into the
// software buffer.

void _interrupt _far SerialISR( void )
{
   // advise other functions so the buffer doesn't get corrupted
   ser_busy = 1;

   // move received character into next position in software buffer
   ser_buffer[ ser_end ] = INP( open_port );

   // wrap buffer index if overflowing
   if ( ++ser_end == ser_buffer_size )
   {
      ser_end = 0;
   }

   // bump start index if overrun
   if ( ser_start == ser_end )
   {
      if ( ++ser_start == ser_buffer_size )
      {
         ser_start = 0;
      }
   }

   // issue an End of Interrupt, acknowledging interrupt completion
   // and allowing receipt of additional interrupts

   OUTP( SCMD_INT_CONTROL, SINT_END_NONSPECIFIC );

   // surrender busy flag
   ser_busy = 0;
}


//////////////////////////////////////////////////////////////////////////////

// Opens up the serial port, sets it's configuration, turns on all the needed
// bits to make interrupts happen and loads the Interrupt Service Routine

void Open_Serial( int16_t port_base, int16_t baud_divisor, int16_t configuration )
{
   // save the port for other functions
   open_port = port_base;

   // set baud divisor on latch register
   OUTP( port_base + SREG_LINE_CONTROL, SCFG_LATCH_ACCESS );

   // send low and high bytes of baud rate divisor
   OUTP( port_base + SDIV_LATCH_LOW_BYTE, baud_divisor & 0xFF );
   OUTP( port_base + SDIV_LATCH_HIGH_BYTE, baud_divisor >> 8 );

   // set the configuration for the port
   OUTP( port_base + SREG_LINE_CONTROL, configuration );

   // enable interrupt handling
   OUTP( port_base + SREG_MODEM_CONTROL, SCFG_MODEM_OUT2_ENABLE );

   OUTP( port_base + SREG_INT_ENABLE, SINT_RX_READY );

   // install the interrupt service routine
   switch ( port_base )
   {
      case COM_1 :
      {
         prevISR = _dos_getvect( SINT_COM1_COM3 );

         _dos_setvect( SINT_COM1_COM3, SerialISR );

         printf( "\nOpening Communications Channel Com Port #1...\n" );
      }
         break;

      case COM_2 :
      {
         prevISR = _dos_getvect( SINT_COM2_COM4 );

         _dos_setvect( SINT_COM2_COM4, SerialISR );

         printf( "\nOpening Communications Channel Com Port #2...\n" );
      }
         break;

      case COM_3 :
      {
         prevISR = _dos_getvect( SINT_COM1_COM3 );

         _dos_setvect( SINT_COM1_COM3, SerialISR );

         printf( "\nOpening Communications Channel Com Port #3...\n" );
      }
         break;

      case COM_4 :
      {
         prevISR = _dos_getvect( SINT_COM2_COM4 );

         _dos_setvect( SINT_COM2_COM4, SerialISR );

         printf( "\nOpening Communications Channel Com Port #4...\n" );
      }
         break;
   }

   // enable interrupt on PIC
   prev_int_mask = INP( SCMD_INT_MASK );

   switch ( port_base )
   {
      case COM_1:
      case COM_3:
         OUTP( SCMD_INT_MASK, ( prev_int_mask & 0xEF ) );
         break;

      case COM_2:
      case COM_4:
         OUTP( SCMD_INT_MASK, ( prev_int_mask & 0xF7 ) );
         break;
   }

   // drain read buffer on startup
   while ( Ready_Serial() )
   {
      Serial_Read();
   }
}


//////////////////////////////////////////////////////////////////////////////
// closes the port by turning off interrupts and restoring the previous interrupt vector

void Close_Serial( int16_t port_base )
{
   // disable the interrupts
   OUTP( port_base + SREG_MODEM_CONTROL, 0 );

   OUTP( port_base + SREG_INT_ENABLE, 0 );

   OUTP( SCMD_INT_MASK, prev_int_mask );

   // reset original isr handler
   switch ( port_base )
   {
      case COM_1 :
      {
         _dos_setvect( SINT_COM1_COM3, prevISR );

         printf( "\nClosing Communications Channel Com Port #1.\n" );
      }
         break;

      case COM_2 :
      {
         _dos_setvect( SINT_COM2_COM4, prevISR );

         printf( "\nClosing Communications Channel Com Port #2.\n" );
      }
         break;

      case COM_3 :
      {
         _dos_setvect( SINT_COM1_COM3, prevISR );

         printf( "\nClosing Communications Channel Com Port #3.\n" );
      }
         break;

      case COM_4 :
      {
         _dos_setvect( SINT_COM2_COM4, prevISR );

         printf( "\nClosing Communications Channel Com Port #4.\n" );
      }
         break;
   }
}


//////////////////////////////////////////////////////////////////////////////
//

void Attach_Buffer( struct SerialRXBuffer* rx )
{
   while ( ser_busy ) { } // spin while ISR finishes write

   if ( NULL != rx )
   {
      ser_buffer_size = rx->size;
      ser_buffer = rx->buffer;
      ser_end = 0;
      ser_start = 0;
   }
}


//////////////////////////////////////////////////////////////////////////////
//

void Detach_Buffer( struct SerialRXBuffer* rx )
{
   while ( ser_busy ) { } // spin while ISR finishes write

   if ( ( NULL != rx ) && ( ser_buffer != ser_buffer_circ ) )
   {
      rx->startIndex = 0;

      if ( ser_start <= ser_end )
      {
         rx->bytesReceived = ser_end - ser_start;
      }
      else // ( ser_end < ser_start )
      {
         rx->bytesReceived = ( ser_buffer_size - 1 - ser_start + ser_end  );
      }

      rx->buffer = ser_buffer;
   }

   ser_buffer_size = CIRCULAR_BUFFER_SIZE;

   ser_buffer = ser_buffer_circ;  // the stack-based receive buffer

   ser_end = 0;
   ser_start = 0;
}


//////////////////////////////////////////////////////////////////////////////
//
// returns number of buffered bytes or 0 if the buffer is empty

uint16_t Ready_Serial()
{
   if ( ser_start <= ser_end )
   {
      return ( ser_end - ser_start );
   }
   
   return ( ser_buffer_size - 1 - ser_start + ser_end );
}


//////////////////////////////////////////////////////////////////////////////
//

char Serial_Peek()
{
   uint16_t end;
   uint16_t start;

   while ( ser_busy ) { } // spin while ISR finishes write

   end = ser_end;
   start = ser_start;

   // if character(s) ready in buffer
   if ( end != start )
   {
      if ( end > 0 )
      {
         return ser_buffer[ end - 1 ];
      }

      return ser_buffer[ ser_buffer_size - 1 ];
   }

   // buffer was empty return a NULL
   return 0;
}


//////////////////////////////////////////////////////////////////////////////
//
// read character from the circular buffer and return it

char Serial_Read()
{
   while ( ser_busy ) { } // spin while ISR finishes write

   // if character(s) ready in buffer
   if ( ser_end != ser_start )
   {
      // get the character out of buffer
      char result = ser_buffer[ ser_start ];

      // wrap buffer index if needed
      if ( ++ser_start == ser_buffer_size )
      {
         ser_start = 0;
      }

      // send data back to caller
      return result;
   }
   else  // buffer was empty return a NULL
   {
      return 0;
   }
}


//////////////////////////////////////////////////////////////////////////////
//

void Serial_Write( char txByte )
{
   // Wait for the transmit buffer to clear, then write a single character to the
   // transmit buffer.  Temporarily toggle off interrupts while sending

   // wait for empty transmit buffer
   while( !( INP( open_port + SREG_LINE_STATUS ) & SXN_TX_BUFFER_EMPTY ) ) { }

   // turn off interrupts
   _asm cli

   // send the character
   OUTP( open_port, txByte );

   // turn interrupts back on
   _asm sti
}

//////////////////////////////////////////////////////////////////////////////
