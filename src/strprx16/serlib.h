#ifndef __SERLIB_H__
#define __SERLIB_H__

#ifdef _MSC_VER
#include <conio.h> // _inp(), _outp
#endif

#include <dos.h> // _dos_setvect()
#include <stdio.h>

#include "stdtypes.h"

// #include "czcmds.h"  // CZSerial

// D E F I N E S  ////////////////////////////////////////////////////////////

// IRQ Priorities, pg 667, Indispencible PC Hardware Book
#define SINT_COM1_COM3          0x0C  // hardware level interrupt, com 1 & 3, IRQ4
#define SINT_COM2_COM4          0x0B  // hardware level interrupt, com 2 & 4, IRQ3

// 8259A Programmable Interrupt Control, pg 668
#define SCMD_INT_CONTROL        0x20  // PIC operational control
#define SCMD_INT_MASK           0x21  // PIC

// operation command word 2, pg 673
#define SINT_END_NONSPECIFIC    0x20  // issue an End of Interrupt to acknowledge interrupt completion

// serial register adresses (offset), pg 969
#define SREG_INT_ENABLE            1
#define SREG_INT_ID                2
#define SREG_LINE_CONTROL          3  // control data config. and divisor latch
#define SREG_MODEM_CONTROL         4
#define SREG_LINE_STATUS           5
#define SREG_MODEM_STATUS          6  // modem status of cts, ring etc.
#define SREG_SCRATCH_PAD           7  // available only on uart 16450/16550

// divisor latch access bit, pg 969
#define SDIV_LATCH_LOW_BYTE        0  // low byte of baud rate divisor
#define SDIV_LATCH_HIGH_BYTE       1  // high byte of divisor latch

// interrupt enable register, pg 970
#define SINT_RX_READY           0x01  // interrupt when one byte is available in receive buffer

// baud_rate = 115200 / divisor therefore divisor = 115200 / baud_rate
#define SDIV_BAUD_110           1047
#define SDIV_BAUD_300            384
#define SDIV_BAUD_1200            96  // baud rate divisors for 1200 baud - 19200
#define SDIV_BAUD_2400            48
#define SDIV_BAUD_4800            24
#define SDIV_BAUD_9600            12
#define SDIV_BAUD_19200            6
#define SDIV_BAUD_38400            3
#define SDIV_BAUD_57600            2
#define SDIV_BAUD_115200           1

#define COM_1                  0x3F8
#define COM_2                  0x2F8
#define COM_3                  0x3E8  // IRQ4 (or polling)
#define COM_4                  0x2E8  // IRQ3 (or polling)

// line control/data format register, pg 973
#define SCFG_BITS_5             0x00  // 5 bit characters
#define SCFG_BITS_6             0x01  // 6 bit characters
#define SCFG_BITS_7             0x02  // 7 bit characters
#define SCFG_BITS_8             0x03  // 8 bit characters

#define SCFG_STOP_1             0x00  // 1 stop bit per character
#define SCFG_STOP_2             0x04  // 2 stop bits per character

#define SCFG_PARITY_NONE        0x00  // no parity bit in serial bit stream
#define SCFG_PARITY_ODD         0x08
#define SCFG_PARITY_EVEN        0x18
#define SCFG_PARITY_MARK        0x28  // parity bit is always set to 1
#define SCFG_PARITY_SPACE       0x38  // parity bit is always set to 0

#define SCFG_LATCH_ACCESS       0x80  // set baud rate divisor

// modem control register, pg 974.
#define SCFG_MODEM_OUT2_ENABLE  0x08  // master interrupt bit enables individual interrupts

// line status register, pg 976
#define SXN_TX_BUFFER_EMPTY     0x20  // transmit buffer emtpy, 1 == no byte in transmit buffer



#define CIRCULAR_BUFFER_SIZE 512      // current size of circulating receive buffer

// E X T E R N S //////////////////////////////////////////////////////////////

#ifdef __BORLANDC__
extern void interrupt ( _far *prevISR )( ... );   // holds original com port interrupt handler
#elif _MSC_VER
extern void ( _interrupt _far *prevISR )();       // holds original com port interrupt handler
#endif

extern char                   ser_buffer_circ[];
extern char*                  ser_buffer;
extern uint16_t               ser_end;
extern uint16_t               ser_start;          // indexes into receive buffer
extern volatile uint8_t       ser_busy;           // flag to prevent buffer updates while ISR writes are occurring

extern int16_t                prev_int_mask;      // original interrupt mask on the PIC
extern int16_t                open_port;          // currently open port


//////////////////////////////////////////////////////////////////////////////

struct SerialRXBuffer
{
   uint16_t size;
   uint16_t startIndex;
   uint16_t bytesReceived;
   uint8_t  isAcknowledged;

   char* buffer;
};

// P R O T O T Y P E S ///////////////////////////////////////////////////////

#ifdef __BORLANDC__
void _interrupt _far SerialISR( ... );
#elif _MSC_VER
void _interrupt _far SerialISR( void );
#endif

void Open_Serial( int16_t port_base, int16_t baud_divisor, int16_t configuration );
void Close_Serial( int16_t port_base );

void Attach_Buffer( struct SerialRXBuffer* rx );
void Detach_Buffer( struct SerialRXBuffer* rx );

uint16_t Ready_Serial();
char Serial_Peek();
char Serial_Read();
void Serial_Write( char ch );

#endif
