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

    File:   uart_class.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    uart_class.cpp file is part of the RGB LED Controller and Node 
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

#ifndef _UART_CLASS_H_
#include "uart_class.h"
#endif

// Set to 1 to have this class generate these events
#define NOTIFY_OF_TX_COMPLETE_EVENTS 0
#define NOTIFY_OF_RX_EVENTS 1
#define NOTIFY_OF_FLAG_BYTE_EVENTS 1

UartBaseClass* UartBaseClass::pUart = 0;

UartBaseClass::UartBaseClass(E_InputHardware A)
: event_element_class(A,E_LAST_INPUT_EVENT)
{
    pUart = this;

    // Notify the sleep class that the USART interface is in use.
    mcu_sleep_class::getInstance()->SetInterfaceUsage(
        mcu_sleep_class::E_USART_INTERFACE,
        mcu_sleep_class::E_POWER_INTERFACE_DISABLE_POWER_SAVINGS);

    // Init the UART.
    //  BAUD and F_CPU are defined in the Makefile
    init(UART_BAUD_SELECT(BAUD,F_CPU));
}

void UartBaseClass::init(uint8_t baudrate)
{
    UART_TxHead = 0;
    UART_TxTail = 0;
    UART_RxHead = 0;
    UART_RxTail = 0;

    /* Set baud rate */
    if ( baudrate & 0x8000 ) {
        UART0_STATUS = (1<<U2X0);  //Enable 2x speed
        baudrate &= ~0x8000;
    }
    UBRR0H = (uint8_t)(baudrate>>8);
    UBRR0L = (uint8_t) baudrate;

    /* Enable USART receiver and transmitter and receive complete interrupt */
    UART0_CONTROL = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);

    /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
    UCSR0C = (3<<UCSZ00);
}

bool UartBaseClass::isEmpty()
{
    if (available() > 0) { return false; }
    else return true;
}

uint16_t UartBaseClass::available()
{
    // Returns the number of chars available in the rx buffer.
    return (UART_RX0_BUFFER_SIZE + UART_RxHead - UART_RxTail) & UART_RX0_BUFFER_MASK;
}

void UartBaseClass::flush()
{
    // Clears the RX buffer.
    UART_RxHead = UART_RxTail;
}

bool UartBaseClass::getc(uint8_t &error, uint8_t &data)
{
    uint16_t tmptail;

    if ( UART_RxHead == UART_RxTail ) {
        error = UART_NO_DATA;
        return false;   /* no data available */
    }

    /* calculate /store buffer index */
    tmptail = (UART_RxTail + 1) & UART_RX0_BUFFER_MASK;
    UART_RxTail = tmptail;

    /* get data from receive buffer */
    data = UART_RxBuf[tmptail];

    error = UART_LastRxError;
    return true;
}

#if 0
bool UartBaseClass::peek(uint8_t &error, uint8_t &data)
{
    uint16_t tmptail;

    if ( UART_RxHead == UART_RxTail ) {
        return false;   /* no data available */
    }

    tmptail = (UART_RxTail + 1) & UART_RX0_BUFFER_MASK;

    /* get data from receive buffer */
    data = UART_RxBuf[tmptail];

    error = UART_LastRxError;
    return true;

}
#endif

void UartBaseClass::putc(uint8_t const data)
{
    uint16_t tmphead;

    tmphead  = (UART_TxHead + 1) & UART_TX0_BUFFER_MASK;

    if ( tmphead == UART_TxTail ) {
        // Enable UDRE and Force a TX interrupt
        UART0_CONTROL |= (1<<UART0_UDRIE);

        ;/* wait for free space in buffer */
    }

    UART_TxBuf[tmphead] = data;
    UART_TxHead = tmphead;

    /* enable UDRE interrupt */
    UART0_CONTROL |= (1<<UART0_UDRIE);

}

#if 0
void UartBaseClass::puts(String const &s)
{
    const char * temp = s.c_str();
    while (*temp) {
        putc(*temp++);
    }
}
#endif

#if 0
void UartBaseClass::puts_p(const char *progmem_s)
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) ) {
        putc(c);
    }
}
#endif

void UartBaseClass::receive()
{
    uint16_t tmphead;
    uint8_t data;
    uint8_t usr;
    uint8_t lastRxError;

    /* read UART status register and UART data register */
    usr  = UART0_STATUS;
    data = UART0_DATA;

    /* */
    lastRxError = (usr & ((1<<FE0)|(1<<DOR0)) );

    /* calculate buffer index */
    tmphead = ( UART_RxHead + 1) & UART_RX0_BUFFER_MASK;

    if ( tmphead == UART_RxTail ) {
        /* error: receive buffer overflow */
        lastRxError = UART_BUFFER_OVERFLOW >> 8;
    } else {
        /* store new index */
        UART_RxHead = tmphead;
        /* store received data in buffer */
        UART_RxBuf[tmphead] = data;
    }
    UART_LastRxError = lastRxError;

#if NOTIFY_OF_FLAG_BYTE_EVENTS
    // Notify listener of this event.
    if (data == COMM_CLASS_FLAG_BYTE)
    {
        event_element_class A;
        A.set(get_current_hardware(),E_InputEvent::E_UART_FLAG_BYTE_FOUND_EVENT);
        Notify(A);
    }
#endif

#if NOTIFY_OF_RX_EVENTS
    event_element_class A;
    A.set(get_current_hardware(),E_InputEvent::E_UART_RX_EVENT);
    Notify(A);
#endif
}

void UartBaseClass::transmit()
{
    uint16_t tmptail;

    if ( UART_TxHead != UART_TxTail) {
        /* calculate and store new buffer index */
        tmptail = (UART_TxTail + 1) & UART_TX0_BUFFER_MASK;
        UART_TxTail = tmptail;
        /* get one byte from buffer and write it to UART */
        UART0_DATA = UART_TxBuf[tmptail];  /* start transmission */
    } else {
        /* tx buffer empty, disable UDRE interrupt */
        UART0_CONTROL &= ~(1<<UART0_UDRIE);

#if NOTIFY_OF_TX_COMPLETE_EVENTS
        // Notify listener of this event.
        event_element_class A(get_current_hardware(),E_InputEvent::E_UART_TX_COMPLETE);
        Notify(A);
#endif
    }
}


ISR(USART_RX_vect)
{
    UartBaseClass::pUart->receive();
}


ISR(USART_UDRE_vect)
{
    UartBaseClass::pUart->transmit();
}



