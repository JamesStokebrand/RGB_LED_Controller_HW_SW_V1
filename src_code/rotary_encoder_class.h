#ifndef _ROTARY_ENCODER_CLASS_H_
#define _ROTARY_ENCODER_CLASS_H_

/****************************************************
    Rotary Encoder Class

    File:   rotary_encoder_class.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    rotary_encoder_class.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file implements a rotary encoder interface that makes use of 
     two button classes to indicate CW/CCW.  This will generate
     CW/CCW events.

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
    2014 Aug 19  James Stokebrand   Initial Creation

    2014 Aug 27  James Stokebrand   Updated to derive an 
                                    encoder that supports
                                    button press.

    2014 Sep 3   James Stokebrand   Removed button press
                                     support.  Button
                                     press can be implemented
                                     with the button class.

*****************************************************/

#ifndef _EVENT_LISTING_H_
#include "event_listing.h"
#endif

#ifndef _PIN_CLASS_H_
#include "pin_class.h"
#endif

#ifndef _OBSERVER_CLASS_H_
#include "observer_class.h"
#endif

class RotaryEncoderClass
: public EventSubject
, public InterruptObserver
, public event_element_class
{
public:
    RotaryEncoderClass(E_InputHardware HARDWARE
                ,IOPinDefines::E_PinDef const &A
                ,IOPinDefines::E_PinDef const &B
                ,IOPinDefines::E_PinIntrType const &C)
    : event_element_class(HARDWARE,E_LAST_INPUT_EVENT)
    // NOTE do *NOT* pass the E_PinIntrType "C" to the buttonX objects.
    //   The Interrupt observer should be attached to this object and not attached
    //   to the two button objects.
    , buttonA(HARDWARE,A)
    , buttonB(HARDWARE,B)
    {
        lastEncoded = 0;
        _PinInterruptType = C;
    }

    virtual ~RotaryEncoderClass() {}

    void Attach(EventObserver* const &A)
    {
        EventSubject::Attach(A);

        // Now that we have an observer, attach to the interrupt observer
        //  to notify it of events.
        if (_PinInterruptType != IOPinDefines::E_PinIntrType::E_PIN_INTR_DISABLED) {
            // Attach both pins to the Interrupt Class ...
            //  NOTE: pins may (and can) be on different ISR vectors
            buttonA._Subject->Attach(this,buttonA._Bit);
            buttonB._Subject->Attach(this,buttonB._Bit);
        }
    }

    void Detach()
    {
        EventSubject::Detach();

        // Observer detached, detach from the the interrupt observer
        //  there is nobody to notify of the events.
        if (_PinInterruptType != IOPinDefines::E_PinIntrType::E_PIN_INTR_DISABLED) {
            // Detach both pins to the Interrupt Class ...
            //  NOTE: pins may (and can) be on different ISR vectors
            buttonA._Subject->Detach(buttonA._Bit);
            buttonB._Subject->Detach(buttonB._Bit);
        }
    }
 
    bool CheckStatus(void)
    {
        // NOTE only one status should change at a time.
        if (buttonA.CheckStatus() || buttonB.CheckStatus())
        {
            bool MSB = buttonA.Read(); //MSB = most significant bit
            bool LSB = buttonB.Read(); //LSB = least significant bit

            uint8_t encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
            uint8_t sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

            E_InputEvent rotation_direction = E_LAST_INPUT_EVENT;
            if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
                rotation_direction = E_ROTARY_ENCODER_ROTATED_CW;
            }
            if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
                rotation_direction = E_ROTARY_ENCODER_ROTATED_CCW;
            }

            set_current_event(rotation_direction);
            lastEncoded = encoded; //store this value for next call

            // SUCCESS!
            return true;
        }

        return false;
    }

    void Update()
    { 
        if (CheckStatus())
        {
            // SUCCESS!
            event_element_class A(get_current_hardware(), get_current_event());
            Notify(A);
        }
    }

private:
    ButtonClass buttonA;
    ButtonClass buttonB;

    IOPinDefines::E_PinIntrType _PinInterruptType;
    uint8_t lastEncoded;
};

#endif


