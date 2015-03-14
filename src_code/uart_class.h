#ifndef _UART_CLASS_H_
#define _UART_CLASS_H_

/************************************************************************
Title:    Interrupt UART library with receive/transmit circular buffers
Author:   Andy Gock
Software: AVR-GCC 4.1, AVR Libc 1.4
Hardware: any AVR with built-in UART, tested on AT90S8515 & ATmega8 at 4 Mhz
License:  GNU General Public License 
Usage:    see Doxygen manual

Based on original library by Peter Fluery, Tim Sharpe, Nicholas Zambetti.

https://github.com/andygock/avr-uart

LICENSE:
    Copyright (C) 2012 Andy Gock

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

LICENSE:
    Copyright (C) 2006 Peter Fleury

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
************************************************************************/

/************************************************************************
uart_available, uart_flush, uart1_available, and uart1_flush functions
were adapted from the Arduino HardwareSerial.h library by Tim Sharpe on 
11 Jan 2009.  The license info for HardwareSerial.h is as follows:

  HardwareSerial.h - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
************************************************************************/

/************************************************************************
Changelog for modifications made by Tim Sharpe, starting with the current
  library version on his Web site as of 05/01/2009. 

Date        Description
=========================================================================
05/12/2009  Added Arduino-style available() and flush() functions for both
            supported UARTs.  Really wanted to keep them out of the library, so
            that it would be as close as possible to Peter Fleury's original
            library, but has scoping issues accessing internal variables from
            another program.  Go C!

************************************************************************/

/** 
 *  @defgroup avr-uart UART Library
 *  @code #include <uart.h> @endcode
 * 
 *  @brief Interrupt UART library using the built-in UART with transmit and receive circular buffers. 
 *
 *  This library can be used to transmit and receive data through the built in UART. 
 *
 *  An interrupt is generated when the UART has finished transmitting or
 *  receiving a byte. The interrupt handling routines use circular buffers
 *  for buffering received and transmitted data.
 *
 *  The UART_RXn_BUFFER_SIZE and UART_TXn_BUFFER_SIZE constants define
 *  the size of the circular buffers in bytes. Note that these constants must be a power of 2.
 *
 *  You need to define these buffer sizes in uart.h
 *
 *  @note Based on Atmel Application Note AVR306
 *  @author Andy Gock <andy@gock.net>
 *  @note Based on original library by Peter Fleury and Tim Sharpe.
 */


/****************************************************
    Uart Class

    File:   uart_class.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    uart_class.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file was adapted from "uart.c/h" found on Github.  
     (See header files above). 
     - Removed code not related to ATmega328p.
     - Wrapped code in cpp class.
     - Wrapped by the comm class to facilitate message 
        communication.

    Rev History:
     When         Who                Description of change
    -----------  ----------         ------------------------
    2014 Aug 19  James Stokebrand   UART communicaton's wrapped 
                                      into a class.  The only
                                      advantage this gives is
                                      automatic initialization.

*****************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#ifndef _EVENT_LISTING_H_
#include "event_listing.h"
#endif

#ifndef _OBSERVER_CLASS_H_
#include "observer_class.h"
#endif

#ifndef _MCU_SLEEP_CLASS_H_
#include "mcu_sleep_class.h"
#endif

#define UART_RX0_BUFFER_MASK ( UART_RX0_BUFFER_SIZE - 1)
#define UART_TX0_BUFFER_MASK ( UART_TX0_BUFFER_SIZE - 1)

#ifndef UART_RX0_BUFFER_SIZE
    #define UART_RX0_BUFFER_SIZE 128 /**< Size of the circular receive buffer, must be power of 2 */
#endif
#ifndef UART_TX0_BUFFER_SIZE
    #define UART_TX0_BUFFER_SIZE 128 /**< Size of the circular transmit buffer, must be power of 2 */
#endif

#if ( UART_RX0_BUFFER_SIZE & UART_RX0_BUFFER_MASK )
    #error RX0 buffer size is not a power of 2
#endif
#if ( UART_TX0_BUFFER_SIZE & UART_TX0_BUFFER_MASK )
    #error TX0 buffer size is not a power of 2
#endif

#if (UART_RX0_BUFFER_SIZE > 65536)
    #error "Buffer too large, maximum allowed is 65536 bytes"
#endif

/** @brief  UART Baudrate Expression
 *  @param  xtalCpu  system clock in Mhz, e.g. 4000000L for 4Mhz          
 *  @param  baudRate baudrate in bps, e.g. 1200, 2400, 9600     
 */
#define UART_BAUD_SELECT(baudRate,xtalCpu) (((xtalCpu)+8UL*(baudRate))/(16UL*(baudRate))-1UL)

/** @brief  UART Baudrate Expression for ATmega double speed mode
 *  @param  xtalCpu  system clock in Mhz, e.g. 4000000L for 4Mhz           
 *  @param  baudRate baudrate in bps, e.g. 1200, 2400, 9600     
 */
#define UART_BAUD_SELECT_DOUBLE_SPEED(baudRate,xtalCpu) ((((xtalCpu)+4UL*(baudRate))/(8UL*(baudRate))-1)|0x8000)

#if ( (UART_RX0_BUFFER_SIZE+UART_TX0_BUFFER_SIZE) >= (RAMEND-0x60 ) )
#error "size of UART_RX0_BUFFER_SIZE + UART_TX0_BUFFER_SIZE larger than size of SRAM"
#endif

/* 
** high byte error return code of uart_getc()
*/
#define UART_FRAME_ERROR      0x08              /**< Framing Error by UART       */
#define UART_OVERRUN_ERROR    0x04              /**< Overrun condition by UART   */
#define UART_BUFFER_OVERFLOW  0x02              /**< receive ringbuffer overflow */
#define UART_NO_DATA          0x01              /**< no receive data available   */


#define UART0_STATUS   UCSR0A
#define UART0_CONTROL  UCSR0B
#define UART0_DATA     UDR0
#define UART0_UDRIE    UDRIE0

class UartBaseClass
: public event_element_class
, public EventSubject
{
public:

    UartBaseClass(E_InputHardware);

    virtual ~UartBaseClass() {}

    bool isEmpty();
    uint16_t available();
    void flush();

    bool getc(uint8_t &error, uint8_t &data);
    void putc(uint8_t const data);

#if 0
    // Not using these ... commented out for space
    bool peek(uint8_t &error, uint8_t &data);

    //void puts(String const &s);
    void puts_p(const char *progmem_s);
#endif

    void receive();
    void transmit();

    static UartBaseClass* pUart;

    // 0x7E is a flag byte for Start/Stop of a frame.
    static const uint8_t COMM_CLASS_FLAG_BYTE = 0x7E;

    //  Each 0x7D in the data stream is replaced with 0x7D 0x5D
    //  Each 0x7E in the data stream is replaced with 0x7D 0x5E
    static const uint8_t COMM_CLASS_ESCAPE_CHAR_START = 0x7D;
    static const uint8_t COMM_CLASS_BYTE_STUFF_XOR_VALUE = 0x20;

private:
    void init(uint8_t baudrate);

    volatile uint8_t UART_TxBuf[UART_TX0_BUFFER_SIZE];
    volatile uint8_t UART_RxBuf[UART_RX0_BUFFER_SIZE];

    volatile uint8_t UART_TxHead;
    volatile uint8_t UART_TxTail;
    volatile uint8_t UART_RxHead;
    volatile uint8_t UART_RxTail;
    volatile uint8_t UART_LastRxError;

};

#endif


