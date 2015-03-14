#ifndef _PWM_CLASS_H_
#define _PWM_CLASS_H_

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

    File:   pwm_class.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    pwm_class.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This code is a port of the SoftPWM.h/cp code using
     subject/observer objects.  This implements a interrupt
     based software PWM.

    Rev History:
     When         Who                Description of change
    -----------  ----------         ------------------------
    2014 Oct 05  James Stokebrand   Initial Creation

*****************************************************/

#ifndef _PIN_CLASS_H_
#include "pin_class.h"
#endif

#ifndef _HAL_INTERRUPTS_H_
#include "hal_interrupts.h"
#endif

class pwm_class
: public InterruptObserverPWM
{
public:
    pwm_class(IOPinDefines::E_PinDef const &A
            , bool const &CommonCathode = true
            , uint8_t const &_StartValue = 0);

    void setValue(uint8_t const &A);
    void setPercent(uint8_t const &A);
    void On();
    void Off();
    void Toggle();

    inline uint8_t getValue()
    {
        return _PwmValue;
    }

protected:
    void Update(uint8_t const &_Pwm);

private:
    volatile uint8_t _PwmValue;
    uint8_t _ObserverID;
    OutputPinClass *_LED;

    TIMER2_interrupt_subject* _Subject;
};


#endif

