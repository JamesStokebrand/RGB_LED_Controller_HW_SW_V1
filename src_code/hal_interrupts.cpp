/****************************************************
    Hardware Abstraction Layer - Interrupts

    File:   hal_interrupts.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    hal_interrupts.cpp file is part of the RGB LED Controller and Node 
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

#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef _HAL_INTERRUPTS_H_
#include "hal_interrupts.h"
#endif


// PortB
PORTB_interrupt_subject* PORTB_interrupt_subject::pINTR_handler = 0;

PORTB_interrupt_subject::PORTB_interrupt_subject()
: InterruptSubjectPinIntr(&PCMSK0,PCIE0)
{
    pINTR_handler = this;
}

ISR(PCINT0_vect)
{
    PORTB_interrupt_subject::pINTR_handler->Notify();
}


// PortC
PORTC_interrupt_subject* PORTC_interrupt_subject::pINTR_handler = 0;

PORTC_interrupt_subject::PORTC_interrupt_subject()
: InterruptSubjectPinIntr(&PCMSK1,PCIE1)
{
    pINTR_handler = this;
}

ISR(PCINT1_vect)
{
    PORTC_interrupt_subject::pINTR_handler->Notify();
}


// PortD
PORTD_interrupt_subject* PORTD_interrupt_subject::pINTR_handler = 0;

PORTD_interrupt_subject::PORTD_interrupt_subject()
: InterruptSubjectPinIntr(&PCMSK2,PCIE2)
{
    pINTR_handler = this;
}

ISR(PCINT2_vect)
{
    PORTD_interrupt_subject::pINTR_handler->Notify();
}


// Timer2
TIMER2_interrupt_subject* TIMER2_interrupt_subject::pINTR_handler = 0;

#define PWM_FREQ 60UL
#define PWM_OCR (F_CPU/(PWM_FREQ*8UL*256UL))

TIMER2_interrupt_subject::TIMER2_interrupt_subject()
: InterruptSubjectPWM(&TIMSK2,OCIE2A)
, pwmCount(0)
{

    TIFR2 = (1 << TOV2);    /* clear interrupt */
    TCCR2B = (1 << CS21);   /* start timer (ck/8 prescalar) */
    TCCR2A = (1 << WGM21);  /* CTC mode */
    OCR2A = (PWM_OCR);      /* At least 30Hz */

    pINTR_handler = this;
};

ISR(TIMER2_COMPA_vect)
{
    TIMER2_interrupt_subject::pINTR_handler->Notify(
            TIMER2_interrupt_subject::pINTR_handler->pwmCount++);
}

// SPI
SPI_interrupt_subject* SPI_interrupt_subject::pINTR_handler = 0;

SPI_interrupt_subject::SPI_interrupt_subject() {
    pINTR_handler = this;
}

ISR(SPI_STC_vect)
{
    // Notify the observer of the Spi Data Register's contents
    uint8_t temp = SPDR;
    SPI_interrupt_subject::pINTR_handler->Notify(temp);
}

// TWI
TWI_interrupt_subject* TWI_interrupt_subject::pINTR_handler = 0;

TWI_interrupt_subject::TWI_interrupt_subject() {
    pINTR_handler = this;
}

ISR(TWI_vect)
{
    // Notify the observer of the TWI Status Register's contents
    TWI_interrupt_subject::pINTR_handler->Notify();
}



