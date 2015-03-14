/****************************************************
    Observer Class

    File:   observer_class.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    observer_class.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file implements a variety of Subject/Observer helper
     objects.

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
    2014 Oct 05  James Stokebrand   Initial creation.

*****************************************************/


#include <util/atomic.h>

#ifndef _OBSERVER_CLASS_H_
#include "observer_class.h"
#endif

#ifndef _PIN_CLASS_H_
#include "pin_class.h"
#endif

#ifndef _MCU_SLEEP_CLASS_H_
#include "mcu_sleep_class.h"
#endif


// ##############################
// ## EventSubject Object
// ##############################
void EventSubject::Attach(EventObserver* const &A)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        _EventObserver = A;
    }
}

void EventSubject::Detach()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        _EventObserver = nullptr;
    }
}

void EventSubject::Notify(event_element_class const &A)
{
    // Notify observer of this event
    if (isAttached()) _EventObserver->Update(A);
}


// ##############################
// ## InterruptSubjectPWM Object
// ##  This object wraps the software PWM interrupt handler
// ##############################
InterruptSubjectPWM::~InterruptSubjectPWM()
{
    // Disable this interrupt.
    *(_TimerEnableReg) &= ~(1<<_TimerEnablePin);

    // Clear all the observers
    InitObservers();
}


void InterruptSubjectPWM::Attach(InterruptObserverPWM* const &A, uint8_t &ID)
{
    ID = 0xFF; // Set the maximum.
    if (ObserverCount == _NumberOfObservers)
    {
        // Observer list is full ... 
        return;
    }
    // Add this observer to the first empty observer space.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        uint8_t jj = 0;
        while (jj < _NumberOfObservers)
        {
            if (_Observer[jj] == nullptr) 
            {
                // Found a free observer slot
                _Observer[jj] = A;

                // Set the return ID
                ID = jj;

                // break out of the while loop
                jj = _NumberOfObservers; // break out of the while loop
            }
            else 
            {
                // Keep movin'
                jj++;
            }
        }
        if (ID < 0xFF) ObserverCount++;
    }

    // If we have newly attached observers ... enable the interrupt
    if (ObserverCount==1)
    {
        // Disable timer2 power savings
        mcu_sleep_class::getInstance()->SetInterfaceUsage(
            mcu_sleep_class::E_TIMER_TWO_INTERFACE,
            mcu_sleep_class::E_POWER_INTERFACE_DISABLE_POWER_SAVINGS);

        // Reset counter to zero.
        TCNT2 = 0;

        // Clear interrupt before we enable
        TIFR2 = (1 << TOV2);

        // Enable timer interrupt
        *(_TimerEnableReg) |= (1<<_TimerEnablePin);
    }
}

void InterruptSubjectPWM::Detach(uint8_t const &ID)
{
    // Something to detach?
    if (ObserverCount == 0) return;
    if (ID > _NumberOfObservers) return;
    if (_Observer[ID] == nullptr) return;

    // Detach the observer at ID.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        _Observer[ID] = nullptr;
        ObserverCount--;

        // No observers?  Then disable the timer interrupt
        if (ObserverCount==0) 
        {
            // Disable the timer interrupt
            *(_TimerEnableReg) &= ~(1<<_TimerEnablePin);

            // ENABLE timer2 power savings interface
            mcu_sleep_class::getInstance()->SetInterfaceUsage(
                mcu_sleep_class::E_TIMER_TWO_INTERFACE,
                mcu_sleep_class::E_POWER_INTERFACE_ENABLE_POWER_SAVINGS);
        }
    }
}

void InterruptSubjectPWM::Notify(uint8_t const &_Pwm)
{
    // Update the subject
    for (uint8_t jj=0; jj<_NumberOfObservers; jj++)
    {
        if (_Observer[jj]) _Observer[jj]->Update(_Pwm);
    }
}

void InterruptSubjectPWM::InitObservers()
{
    // Set them all to nullptr.
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        for (uint8_t jj=0; jj<_NumberOfObservers; jj++)
        {
            _Observer[jj]=nullptr;
        }

        ObserverCount=0;
    }
}



// ##############################
// ## InterruptSubjectPinIntr Object
// ##  This object wraps PORT(X) interrupt handlers
// ##############################
InterruptSubjectPinIntr::InterruptSubjectPinIntr(
 volatile uint8_t* PinMaskIntrReg
,uint8_t PinChangeIntrEnableBit)
:_PinMaskIntrReg(PinMaskIntrReg)
,_PinChangeIntrEnableBit(PinChangeIntrEnableBit)
{
    InitObservers();
}

void InterruptSubjectPinIntr::Attach(InterruptObserver* const &A,uint8_t const &Bit)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        bool empty_observer = false;
        // Is this a new observer?
        if (_Observers[Bit] == nullptr) empty_observer = true;

        // IF it the _Observer[Bit] != nullptr then replace the observer but DONT increment the ObserverCount.
        _Observers[Bit] = A;

        // Observer was empty.  Increment the ObserverCount.
        if (empty_observer) ObserverCount++;

        // Enable the specific pin interrupt
        *(_PinMaskIntrReg) |= (1<<Bit);

        // Enable interupts for this ISR
        PIN_CHANGE_INTERRUPT_CONTROL_REGISTER |= (1<<_PinChangeIntrEnableBit);

        // Tell the control register to trigger on both rising and falling edges
        EXTERNAL_INTERRUPT_CONTROL_REGISTER_A |= (1<<INTERRUPT_SENSE_CONTROL_ONE_BIT_ZERO);
    }
}

void InterruptSubjectPinIntr::Detach(uint8_t const &Bit)
{
    // Are there observers to Detach?  Or is requested observer nullptr
    if ((ObserverCount==0) || (_Observers[Bit]==nullptr)) return; // nothing to do.  There is nothing to Detach

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Else remove this observer from the array
        _Observers[Bit]= nullptr;

        // Dec the ObserverCount
        ObserverCount--;
    }

    // Set this bit in the _PinMaskIntrReg to ZERO
    *(_PinMaskIntrReg) &= ~(1<<Bit);

    if (ObserverCount==0) {
        // ObserverCount is zero.  No observers on this ISR, disable the interrupt.
        PIN_CHANGE_INTERRUPT_CONTROL_REGISTER &= ~(1<<_PinChangeIntrEnableBit);
    }
}

void InterruptSubjectPinIntr::Notify()
{
    // Notify observers of an interrupt event
    for (uint8_t jj=0; jj<_NumberOfObservers; jj++)
    {
        if (_Observers[jj]) _Observers[jj]->Update();
    }
}

void InterruptSubjectPinIntr::InitObservers()
{
    // Init array of observers
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        for (uint8_t jj=0; jj<_NumberOfObservers; jj++)
        {
            // Set all observers to nullptr
            _Observers[jj]=nullptr;
            // Set all the bits in the _PinMaskIntrReg to ZERO
            *(_PinMaskIntrReg) &= ~(1<<jj);

            // Empty. Set the ObserverCount to zero.
            ObserverCount = 0;
        }
    }

    // Disable interrupts for this ISR
    PIN_CHANGE_INTERRUPT_CONTROL_REGISTER &= ~(1<<_PinChangeIntrEnableBit);
}

// ##############################
// ## InterruptObserverSPI Object
// ##  This object wraps SPI interrupt handler
// ##############################
InterruptSubjectSPI::~InterruptSubjectSPI() {
    Detach();
}

void InterruptSubjectSPI::Attach(InterruptObserverSPI* const &A) {
    _Observer = A;
}

void InterruptSubjectSPI::Detach() {
    _Observer = nullptr;
}

void InterruptSubjectSPI::Notify(uint8_t const &_SPDR_value) {
    // Update observer (if one is attached)
    if (_Observer) _Observer->Update(_SPDR_value);
}

// ##############################
// ## InterruptObserverTWI Object
// ##  This object wraps TWI interrupt handler
// ##############################
InterruptSubjectTWI::~InterruptSubjectTWI() {
    Detach();
}

void InterruptSubjectTWI::Attach(InterruptObserverTWI* const &A) {
    _Observer = A;
}

void InterruptSubjectTWI::Detach() {
    _Observer = nullptr;
}

void InterruptSubjectTWI::Notify() {
    // Update observer (if one is attached)
    if (_Observer) _Observer->Update();
}


