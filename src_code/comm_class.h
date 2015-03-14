#ifndef _COMM_CLASS_H_
#define _COMM_CLASS_H_

/****************************************************

    Comm Class

    File:   comm_class.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    comm_class.h file is part of the RGB LED Controller and Node 
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

#include <stdbool.h>

#ifndef _UART_CLASS_H_
#include "uart_class.h"
#endif

class comm_class
: public EventObserver
, public EventSubject
{
public:
    comm_class()
    :_UartClass(E_UART_00)
    {
        _UartClass.Attach(this);

        TRAN((STATE)&comm_class::STATE_decode__search_for_flag_byte);
        current_receive_msg._MsgValid = false;
        current_transmit_msg._MsgValid = false;
    }

    virtual ~comm_class() {
        _UartClass.Detach();
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

    // Encode and send an Event Msg
    void encode(event_element_class const &A);

    // Rx and Decode an Event Msg
    bool decode(event_element_class &A);

    virtual void Update(event_element_class const &A);

private:
    // Used internally

    typedef enum {
         E_COMM_CLASS_EVENT_MSG = 0x01 // Msg containing events
        ,E_COMM_CLASS_LAST_EVENT
    } E_CommClass_MsgType;

    struct comm_class_event_msg_struct {
        E_InputHardware     _HardwareID;
        E_InputEvent        _EventID;
        uint8_t             _Uint8_Data;
    };

    struct comm_class_current_msg_struct {
        bool                _MsgValid;
        comm_class_event_msg_struct _EventMsg;
    };

    // Rx and Tx msg structures
    comm_class_current_msg_struct current_receive_msg;
    comm_class_current_msg_struct current_transmit_msg;

    // Confirm the length of the RXed msg.
    bool confirm_length(uint8_t *raw, uint8_t const &len);

    // Byte stuff and send this byte
    void byte_stuff(uint8_t const &A);

    //##############################
    // State machine for tx/rx
    //##############################
    typedef void (comm_class::*STATE)(uint8_t *msg, uint8_t &pos);
    void TRAN(STATE target) {
        state = static_cast<STATE>(target);
    }
    void process(uint8_t *A, uint8_t &B) { (this->*state)(A,B); }
    STATE state;


    // Search for a flag byte in the current msg
    void STATE_decode__search_for_flag_byte(uint8_t *msg, uint8_t &pos);

    // Flag byte found, seek more flag bytes or additional bytes in the msg.
    void STATE_decode__flag_byte_found(uint8_t *msg, uint8_t &pos);

    // Collect the current msg until reaching a flag byte.
    void STATE_decode__collect_msg(uint8_t *msg, uint8_t &pos);

    // Stuffed byte found, byte thin the next byte.
    void STATE_decode__byte_thin(uint8_t *msg, uint8_t &pos);

    UartBaseClass _UartClass;

    static const uint8_t MAX_SEARCH_BUFFER_SIZE = 32;
};


#endif


