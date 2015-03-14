/****************************************************
    Pin Class

    File:   pin_class.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    pin_class.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file implements the input/output pin definitions.  This 
     simplifies the defining of pins by an enum and allows the 
     abstraction of pin resources.  (IE Buttons/LEDs/Rotary Encoders etc)

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
    2014 Sep 09  James Stokebrand   Initial creation.

    2014 Sep 24  James Stokebrand   Updated to include
                                      interrupt handlers

*****************************************************/

#ifndef _PIN_CLASS_H_
#include "pin_class.h"
#endif

#ifndef _HAL_INTERRUPTS_H_
#include "hal_interrupts.h"
#endif

// TODO: These dont belong here.  Need to be moved to button class.
PORTB_interrupt_subject aPortB_Inter;
PORTC_interrupt_subject aPortC_Inter;
PORTD_interrupt_subject aPortD_Inter;


IOPinDefines::IOPinDefines(E_PinDef const &aPin, E_PinIntrType const &anIntr)
{
    _Pin = aPin;
    _PinInterruptType = anIntr;

    // *NOTE* these PIN/PORT/DDR are backwards from 
    //  how they appear in the E_PinDef enum.
    if (aPin >= E_PIN_PD0) {
        // PortD
        _PinReg  = &PIND;
        _PortReg = &PORTD;
        _DDRReg  = &DDRD;
        _Subject = &aPortD_Inter;
        _Bit = aPin - E_PIN_PORTD_BASE;
    } else if (aPin >= E_PIN_PC0) {
        // PortC
        _PinReg  = &PINC;
        _PortReg = &PORTC;
        _DDRReg  = &DDRC;
        _Subject = &aPortC_Inter;
        _Bit = aPin - E_PIN_PORTC_BASE;
    } else {
        // PortB
        _PinReg  = &PINB;
        _PortReg = &PORTB;
        _DDRReg  = &DDRB;
        _Subject = &aPortB_Inter;
        _Bit = aPin - E_PIN_PORTB_BASE;
    }
};


