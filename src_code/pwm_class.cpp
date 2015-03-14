/* $Id: SoftPWM.h 129 2011-05-27 20:37:08Z bhagman@roguerobotics.com $

  A Software PWM Library
  
  Written by Brett Hagman
  http://www.roguerobotics.com/
  bhagman@roguerobotics.com

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*************************************************/

/****************************************************
    PWM Class

    File:   pwm_class.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    pwm_class.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This code is a port of the SoftPWM.h/cp code using
     subject/observer objects.  This implements a interrupt
     based software PWM.

    Rev History:
     When         Who                Description of change
    -----------  ----------         ------------------------
    2014 Oct 05  James Stokebrand   Initial Creation

*****************************************************/

#include <util/atomic.h>

#ifndef _PWM_CLASS_H_
#include "pwm_class.h"
#endif


// Timer2 interrupt object instance
TIMER2_interrupt_subject aTIMER2_Inter;

pwm_class::pwm_class(IOPinDefines::E_PinDef const &A
    , bool const &CommonCathode
    , uint8_t const &StartValue)
: _ObserverID(0xFF)
{

    if (CommonCathode) 
    {
        // Common Cathode (to Ground)
        _LED = new LED_CommonCathode(A);
    } 
    else 
    {
        // ELSE Common Anode (to V+))
        _LED = new LED_CommonAnode(A);
    }

    // Set the interrupt handler
    _Subject = &aTIMER2_Inter;

    // Set the initial value
    setValue(StartValue);
}

void pwm_class::setValue(uint8_t const &A)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        _PwmValue = A;
    }

#if 1
    // If the value is ON or OFF
    if (_PwmValue == 0x00) { 
        Off(); // Detach and turn led OFF
        return; // Return here to avoid the _ObserverID check below
    } else if (_PwmValue == 0xFF) {
        On();  // Detach and turn led ON
        return; // Return here to avoid the _ObserverID check below
    } 
#endif

    // Value is somewhere inbetween top and bottom ...
    if (_ObserverID == 0xFF) {
        _Subject->Attach(this,_ObserverID);
    }
}

void pwm_class::setPercent(uint8_t const &A)
{
    uint8_t temp = (((uint16_t)A * 255) / 100);
    setValue(temp);
}

void pwm_class::On()
{
    // If we are fully ON.  Detach
    //  from the interrupt handler

    // Detach first to stop updates
    _Subject->Detach(_ObserverID);
    // Turn the LED ON
    _LED->On();
    // Set observer ID to detached
    _ObserverID = 0xFF;
}

void pwm_class::Off()
{
    // If we are fully OFF.  Detach
    //  from the interrupt handler

    // Detach first to stop updates
    _Subject->Detach(_ObserverID);
    // Turn the LED OFF
    _LED->Off();
    // Set observer ID to detached
    _ObserverID = 0xFF;
}

void pwm_class::Toggle()
{
    // Flip the value (pwm based)
    uint8_t temp;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        temp = _PwmValue;
    }
    if ((temp == 0xFF) || (temp == 0))
    {
        _LED->Toggle();
    } else {
        temp += 128;
        setValue(temp);
    }
}

// Update is called from the Timer ISR
void pwm_class::Update(uint8_t const &_Pwm)
{
    if (_PwmValue <= _Pwm)
    {
        _LED->Off();
    }
    else 
    {
        // Turn the pin on
        _LED->On();
    }
}


