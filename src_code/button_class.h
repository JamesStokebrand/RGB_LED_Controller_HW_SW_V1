#ifndef _BUTTON_CLASS_H_
#define _BUTTON_CLASS_H_

/****************************************************

    Button Class

    File:   button_class.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    button_class.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file is used to attach to an AVR pin and use it 
    as an input.  When a button is pushed or released it 
    will return a button event.

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
    2014 Aug 13  James Stokebrand   Initial version.
    2014 Aug 24  James Stokebrand   Created interrupt version 
                                     of the ButtonClass

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

#ifndef _HAL_INTERRUPTS_H_
#include "hal_interrupts.h"
#endif


class ButtonClass 
: public InputPinClass
, public event_element_class
, public EventSubject
, public InterruptObserver
{
public:
    ButtonClass(E_InputHardware HARDWARE
            ,IOPinDefines::E_PinDef const &A
            ,IOPinDefines::E_PinIntrType const &B = IOPinDefines::E_PinIntrType::E_PIN_INTR_DISABLED)
    : InputPinClass(A,B)
    , event_element_class(HARDWARE,E_LAST_INPUT_EVENT)
    {
        // Read/set the current button status
        CheckStatus();

        // Lookup the interrupt subject for this pin.
        _Subject = lookup_port_interrupt_subject::return_port_interrupt_subject(_Pin);
    }

    virtual ~ButtonClass() {
        Detach();
    }

    bool IsPressed()
    {
        CheckStatus();

        if (get_current_event()==E_BUTTON_IS_PRESSED)
        {
            return true;
        }
        return false;
    }

    void Attach(EventObserver* A)
    {
        // Read/set the current button status
        CheckStatus();

        EventSubject::Attach(A);

        // Observer attached ... Attach() to the interrupt observer 
        //  to enable interrupts and send events
        if (_PinInterruptType != IOPinDefines::E_PinIntrType::E_PIN_INTR_DISABLED) {
            // Attach to the Interrupt Class
            if (_Subject) _Subject->Attach(this,_Bit);
        }
    }

    void Detach()
    {
        EventSubject::Detach();

        // Observer detached ... Detach() from the interrupt observer 
        //  to disable interrupts
        if (_PinInterruptType != IOPinDefines::E_PinIntrType::E_PIN_INTR_DISABLED) {
            // Attach to the Interrupt Class
            if (_Subject) _Subject->Detach(_Bit);
        }
    }

    bool CheckStatus(void)
    {
        // Read the pin associated with this button
        E_InputEvent temp = Read() ? E_BUTTON_IS_PRESSED : E_BUTTON_IS_RELEASED;
        // Did its status change?
        if (get_current_event() != temp)
        {
            // Remember its new state
            set_current_event(temp);
            return true;
        }
        return false;
    }

    void Update()
    { 
        if(CheckStatus())
        {
            // The status changed ... notify observer
            event_element_class A(get_current_hardware(),get_current_event());
            Notify(A);
        }
    }

    // Store its interrupt subject.
    InterruptSubjectPinIntr *_Subject;
private:
};

#endif


