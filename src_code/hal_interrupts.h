#ifndef _HAL_INTERRUPTS_H_
#define _HAL_INTERRUPTS_H_

/****************************************************
    Hardware Abstraction Layer - Interrupts

    File:   hal_interrupts.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    hal_interrupts.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file wraps the interrupt handlers into subject/observer objects.
     
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
    2014 Sep 24  James Stokebrand   Initial creation.

*****************************************************/


#ifndef _OBSERVER_CLASS_H_
#include "observer_class.h"
#endif

class PORTB_interrupt_subject
: public InterruptSubjectPinIntr
{
public:
    PORTB_interrupt_subject();
    virtual ~PORTB_interrupt_subject() {}
    static PORTB_interrupt_subject* pINTR_handler;
private:
};

class PORTC_interrupt_subject
: public InterruptSubjectPinIntr
{
public:
    PORTC_interrupt_subject();
    virtual ~PORTC_interrupt_subject() {}
    static PORTC_interrupt_subject* pINTR_handler;
private:
};

class PORTD_interrupt_subject
: public InterruptSubjectPinIntr
{
public:
    PORTD_interrupt_subject();
    virtual ~PORTD_interrupt_subject() {}
    static PORTD_interrupt_subject* pINTR_handler;
private:
};

class TIMER2_interrupt_subject
: public InterruptSubjectPWM
{
public:
    TIMER2_interrupt_subject();
    virtual ~TIMER2_interrupt_subject() {}
    static TIMER2_interrupt_subject* pINTR_handler;
    volatile uint8_t pwmCount;
private:
};

class SPI_interrupt_subject
: public InterruptSubjectSPI
{
public:
    SPI_interrupt_subject();
    virtual ~SPI_interrupt_subject() {}
    static SPI_interrupt_subject* pINTR_handler;
private:
};

class TWI_interrupt_subject
: public InterruptSubjectTWI
{
public:
    TWI_interrupt_subject();
    virtual ~TWI_interrupt_subject() {}
    static TWI_interrupt_subject* pINTR_handler;
private:
};

#endif

