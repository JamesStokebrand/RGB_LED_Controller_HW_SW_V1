#ifndef _OBSERVER_CLASS_H_
#define _OBSERVER_CLASS_H_

/****************************************************
    Observer Class

    File:   observer_class.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    observer_class.h file is part of the RGB LED Controller and Node 
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

#include <avr/io.h>

#ifndef _EVENT_LISTING_H_
#include "event_listing.h"
#endif


static const uint8_t DEFAULT_NUMBER_OF_PIN_INTERRUPT_OBSERVERS = 8;

static const uint8_t DEFAULT_NUMBER_OF_PWM_INTERRUPT_OBSERVERS = 3;


class EventObserver
{
public:
    virtual ~EventObserver() {}
    virtual void Update(event_element_class const &A) = 0;
protected:
    EventObserver() {}
};

class EventSubject
{
public:
    virtual ~EventSubject() { _EventObserver = nullptr; }
    virtual void Attach(EventObserver* const &A);
    virtual void Detach();
    virtual void Notify(event_element_class const &A);
    bool isAttached() 
    { 
        if (_EventObserver) return true;
        return false;
    }
protected:
    EventSubject():_EventObserver(nullptr) {}
private:
    /* *NOTE* each Subject can have only ONE Observer */
    EventObserver *_EventObserver;
};

// ######## PWM
class InterruptObserverPWM
{
public:
    InterruptObserverPWM() {}
    virtual ~InterruptObserverPWM() {}
    virtual void Update(uint8_t const &_Pwm) = 0;
};

class InterruptSubjectPWM
{
public:
    virtual ~InterruptSubjectPWM();
    virtual void Attach(InterruptObserverPWM* const &A, uint8_t &ID);
    virtual void Detach(uint8_t const &ID);
    virtual void Notify(uint8_t const &_Pwm);
protected:
    InterruptSubjectPWM(
         volatile uint8_t* TimerEnableReg
        ,uint8_t TimerEnablePin)
    : _TimerEnableReg(TimerEnableReg)
    , _TimerEnablePin(TimerEnablePin)
    {
        InitObservers();
    }

private:
    void InitObservers();

    const static uint8_t _NumberOfObservers = DEFAULT_NUMBER_OF_PWM_INTERRUPT_OBSERVERS;
    InterruptObserverPWM *_Observer[_NumberOfObservers];
    uint8_t ObserverCount;

    volatile uint8_t* _TimerEnableReg;
    uint8_t           _TimerEnablePin;
};


// ######## Pin Interrupts
class InterruptObserver
{
public:
    InterruptObserver() {}
    virtual ~InterruptObserver() {}
    virtual void Update() = 0;
};

class InterruptSubjectPinIntr
{
public:
    virtual ~InterruptSubjectPinIntr()
    { 
        InitObservers(); 
    }
    virtual void Attach(InterruptObserver* const &A, uint8_t const &Bit=0);
    virtual void Detach(uint8_t const &Bit=0);
    virtual void Notify();

protected:
    InterruptSubjectPinIntr(volatile uint8_t* PinMaskIntrReg,uint8_t PinChangeIntrEnableBit);

private:
    // NOTE: Can have 8 pins max on each interrupt.
    const static uint8_t _NumberOfObservers = DEFAULT_NUMBER_OF_PIN_INTERRUPT_OBSERVERS;
    InterruptObserver *_Observers[_NumberOfObservers];
    uint8_t ObserverCount;

    volatile uint8_t* _PinMaskIntrReg;
    uint8_t _PinChangeIntrEnableBit;

    void InitObservers(); 
};


// ######## SPI
class InterruptObserverSPI
{
public:
    InterruptObserverSPI() {}
    virtual ~InterruptObserverSPI() {}
    virtual void Update(uint8_t const &_SPDR_value) = 0;
};

class InterruptSubjectSPI
{
public:
    virtual ~InterruptSubjectSPI();
    virtual void Attach(InterruptObserverSPI* const &A);
    virtual void Detach();
    virtual void Notify(uint8_t const &_SPDR_value);
protected:
    InterruptSubjectSPI():_Observer(nullptr) {}

private:
    InterruptObserverSPI *_Observer;
};

// ######## TWI
class InterruptObserverTWI
{
public:
    InterruptObserverTWI() {}
    virtual ~InterruptObserverTWI() {}
    virtual void Update() = 0;
};

class InterruptSubjectTWI
{
public:
    virtual ~InterruptSubjectTWI();
    virtual void Attach(InterruptObserverTWI* const &A);
    virtual void Detach();
    virtual void Notify();
protected:
    InterruptSubjectTWI():_Observer(nullptr) {}

private:
    InterruptObserverTWI *_Observer;
};

#endif

