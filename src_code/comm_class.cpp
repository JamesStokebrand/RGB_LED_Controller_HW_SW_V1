/****************************************************

    Comm Class

    File:   comm_class.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    comm_class.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This code wraps the Uart Class to send structured msgs.

    Copyright (C) 2015 - James Stokebrand - 2015 Mar 12

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Rev History:
     When         Who                Description of change
    -----------  ----------         ------------------------
    2014 Oct 05  James Stokebrand   Initial Creation

*****************************************************/

#ifndef _COMM_CLASS_H_
#include "comm_class.h"
#endif

// MSG_LENGTH is equal to the folowing
//   Data msg:
//    Hardware ID (1 byte)
//    Event ID    (1 byte)
//    Data        (1 byte)
//   Total: 3
static const uint8_t MSG_LENGTH = 3;

void comm_class::Update(event_element_class const &A)
{
    // The point of this code is to have the UART Class "update" the
    //  Comm Class of the tx/rx events:
    //      E_UART_FLAG_BYTE_FOUND_EVENT
    //      E_UART_RX_EVENT
    //      E_UART_TX_EVENT
    // then decode() the msg and pass it into the event queue.
    if ((A.get_current_event() == E_UART_FLAG_BYTE_FOUND_EVENT) ||
        (A.get_current_event() == E_UART_RX_EVENT))
    {
        event_element_class temp;
        if (decode(temp))
        {
            Notify(temp);
        }
    }
}

/*
    Msg format:
        0x7E (START byte)
        <MSG struct> (variable size)
        0x7E (END Byte)

    All bytes between START/STOP bytes will be byte stuffed.
        0x7D in the msg body will be stuffed with 0x7D 0x5D
        0x7E in the msg body will be stuffed with 0x7D 0x5E
*/

void comm_class::encode(event_element_class const &A)
{
    // This "encode" method is used to send comm_class_event_msg_struct msgs.
    _UartClass.putc(UartBaseClass::COMM_CLASS_FLAG_BYTE);
    byte_stuff(A.get_current_hardware());
    byte_stuff(A.get_current_event());
    byte_stuff(A.get_current_data());
    _UartClass.putc(UartBaseClass::COMM_CLASS_FLAG_BYTE);
}

bool comm_class::decode(event_element_class &A)
{
    static uint8_t position;
    static uint8_t msg[MAX_SEARCH_BUFFER_SIZE];

    // Are there char's to process?
    while (!_UartClass.isEmpty())
    { 
        process(msg, position);
    }

    // Is this msg valid?
    if (current_receive_msg._MsgValid)
    {
        // Pass the event back to the caller.
        A.set(current_receive_msg._EventMsg._HardwareID
             ,current_receive_msg._EventMsg._EventID
             ,current_receive_msg._EventMsg._Uint8_Data);
        current_receive_msg._MsgValid = false;
        return true;
    }
    // Nothing yet ... 
    return false;
}

bool comm_class::confirm_length(uint8_t *, uint8_t const &len)
{
    if (len == MSG_LENGTH) return true;
    return false;
}


void comm_class::byte_stuff(uint8_t const &A)
{
    if ((A == UartBaseClass::COMM_CLASS_FLAG_BYTE) ||
        (A == UartBaseClass::COMM_CLASS_ESCAPE_CHAR_START))
    {
        // This value needs to be byte stuffed
        _UartClass.putc(UartBaseClass::COMM_CLASS_ESCAPE_CHAR_START);
        _UartClass.putc(A ^ UartBaseClass::COMM_CLASS_BYTE_STUFF_XOR_VALUE);
    } else {
        // No byte stuffing needed ... 
        _UartClass.putc(A);
    }
}


void comm_class::STATE_decode__search_for_flag_byte(uint8_t *, uint8_t &pos)
{
    // Set position to Zero ... ignore previous collected data.
    pos = 0;

    uint8_t error;
    uint8_t data;
    if (_UartClass.getc(error, data))
    {
        if (data == UartBaseClass::COMM_CLASS_FLAG_BYTE)
        {
            TRAN((STATE)&comm_class::STATE_decode__flag_byte_found);
            return;
        }
    }
    // No data available.
}

void comm_class::STATE_decode__flag_byte_found(uint8_t *msg, uint8_t &pos)
{
    uint8_t error;
    uint8_t data;
    if (_UartClass.getc(error, data))
    {
        if (data == UartBaseClass::COMM_CLASS_FLAG_BYTE) 
        { 
            //  Consume all extra COMM_CLASS_FLAG_BYTE
            return;
        }
        else if (data == UartBaseClass::COMM_CLASS_ESCAPE_CHAR_START)
        {
            // Escape char found ... next byte should be byte thinned.
            TRAN((STATE)&comm_class::STATE_decode__byte_thin);
            return;
        }
#if 1
        else if (pos >= MAX_SEARCH_BUFFER_SIZE-1)
        {
            // msg buffer is out of space ... still haven't found 
            // the next FLAG Byte
            // Transition back to searching for a flag byte
            TRAN((STATE)&comm_class::STATE_decode__search_for_flag_byte);
        }
#endif

        // This is the start of the msg ... store it.
        msg[pos++] = data;

        // No more flag bytes expected, transition to collect the rest of the msg
        TRAN((STATE)&comm_class::STATE_decode__collect_msg);
    }
    // No bytes available.
}

void comm_class::STATE_decode__collect_msg(uint8_t *msg, uint8_t &pos)
{
    uint8_t error;
    uint8_t data;
    if (_UartClass.getc(error, data))
    {
        if (data == UartBaseClass::COMM_CLASS_FLAG_BYTE)
        {
            // Found FLAG byte ... End of message.
            // Check length
            if (confirm_length(msg,pos))
            {
                current_receive_msg._EventMsg._HardwareID = (E_InputHardware) msg[0*sizeof(uint8_t)];
                current_receive_msg._EventMsg._EventID    = (E_InputEvent) msg[1*sizeof(uint8_t)];
                current_receive_msg._EventMsg._Uint8_Data = msg[2*sizeof(uint8_t)];
                current_receive_msg._MsgValid = true;
            }
#if 0
            // Is this a valid msg?
            if ((current_receive_msg._EventMsg._HardwareID >= E_InputHardware::E_LAST_HARDWARE_EVENT) ||
                (current_receive_msg._EventMsg._EventID >= E_InputEvent::E_LAST_INPUT_EVENT))
            {
                // Hardware and Input event is out of bounds.  This is bad.  This msg is not valid.
                current_receive_msg._MsgValid = false;
            }
#endif

            // Transition back to searching for a flag byte
            TRAN((STATE)&comm_class::STATE_decode__search_for_flag_byte);
            return;
        }
        else if (data == UartBaseClass::COMM_CLASS_ESCAPE_CHAR_START)
        {
            // Escape char found ... next byte should be byte thinned.
            TRAN((STATE)&comm_class::STATE_decode__byte_thin);            
            return;
        }
#if 1
        else if (pos >= MAX_SEARCH_BUFFER_SIZE-1)
        {
            // msg buffer is out of space ... still haven't found 
            // the complete msg

            // Transition back to searching for a flag byte
            TRAN((STATE)&comm_class::STATE_decode__search_for_flag_byte);
        }
#endif

        // No flag byte found, slurp up more of the msg.
        msg[pos++] = data;
    }
    // No bytes available.
}

void comm_class::STATE_decode__byte_thin(uint8_t *msg, uint8_t &pos)
{
    uint8_t error;
    uint8_t data;
    if (_UartClass.getc(error, data))
    {
        // byte thin this byte by XOR and store it in the raw msg buffer
        msg[pos++] = data ^ UartBaseClass::COMM_CLASS_BYTE_STUFF_XOR_VALUE;

        // Transition back to collecting msg
        TRAN((STATE)&comm_class::STATE_decode__collect_msg);
        return;
    }
    // No bytes available ... 
}


