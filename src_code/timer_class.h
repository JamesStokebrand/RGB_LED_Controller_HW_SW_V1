#ifndef _TIMER_CLASS_H_
#define _TIMER_CLASS_H_

/*
 *  Interrupt and PWM utilities for 16 bit Timer1 on ATmega168/328
 *  Original code by Jesse Tane for http://labs.ideo.com August 2008
 *  Modified March 2009 by Jérôme Despatis and Jesse Tane for ATmega328 support
 *  Modified June 2009 by Michael Polli and Jesse Tane to fix a bug in setPeriod() which caused the timer to stop
 *  Modified Oct 2009 by Dan Clemens to work with timer1 of the ATMega1280 or Arduino Mega
 *  Modified April 2012 by Paul Stoffregen
 *  Modified again, June 2014 by Paul Stoffregen
 *
 *  This is free software. You can redistribute it and/or modify it under
 *  the terms of Creative Commons Attribution 3.0 United States License. 
 *  To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/us/ 
 *  or send a letter to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
 *
 */

/****************************************************
    Timer Class

    File:   timer_class.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    timer_class.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This is a port of the TimerOne.h/cpp file to this project.
    - Wrapped code into a class
    - Updated to generate timer events for state machine

    Rev History:
     When         Who                Description of change
    -----------  ----------         ------------------------
    2014 Oct 05  James Stokebrand   Initial creation.

*****************************************************/

#ifndef _OBSERVER_CLASS_H_
#include "observer_class.h"
#endif

#ifndef _EVENT_LISTING_H_
#include "event_listing.h"
#endif

class timer_class
: public EventSubject
, public event_element_class
{
public:

    typedef enum {
         E_ONE_SHOT_TIMER
        ,E_REPEATING_TIMER
        ,E_LAST_TIMER_TYPE
    } E_TimerType;

    timer_class(E_InputHardware const &A = E_InputHardware::E_TIMER_01);
    virtual ~timer_class() {} 

    uint8_t set(uint32_t const &ms
            ,E_TimerType const &B=E_TimerType::E_ONE_SHOT_TIMER
            ,E_InputEvent const &C=E_InputEvent::E_TIMER_EXPIRE);
    bool start(uint8_t const &A=0);
    bool stop(uint8_t const &A=0);

    // Called in the timer overflow ISR
    void Expired();

    static timer_class* pTimer;
	volatile uint16_t icr1;

private:
    uint32_t period;
    uint8_t timer_select;
    E_TimerType timer_type;
    uint8_t timerID;

    event_element_class temp;
};

#endif
