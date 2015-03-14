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

    File:   timer_class.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    timer_class.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This is a port of the TimerOne.h/cpp file to this project.
    - Wrapped code into a class
    - Updated to generate timer events for state machine

    Rev History:
     When         Who                Description of change
    -----------  ----------         ------------------------
    2014 Oct 05  James Stokebrand   Initial creation.

*****************************************************/

#include <avr/interrupt.h>

#ifndef _TIMER_CLASS_H_
#include "timer_class.h"
#endif

#ifndef _MCU_SLEEP_CLASS_H_
#include "mcu_sleep_class.h"
#endif

#define TIMER1_RESOLUTION 65536UL  // Timer1 is 16 bit

#define MILLISECOND_RESOLUTION 1000
#define TEN_MILLISECOND_RESOLUTION 10000

// DEBUG == 1 will issue additional events
//  E_TIMER_START
//  E_TIMER_STOP
#define DEBUG 0

timer_class* timer_class::pTimer = 0;

timer_class::timer_class(E_InputHardware const &A)
: event_element_class(A)
, period(0)
, timer_select(0)
, timer_type(E_LAST_TIMER_TYPE)
, timerID(0)
{
    pTimer = this;

    // Init the TCCR1(X) register
    TCCR1B = 0;
    TCCR1A = 0;

    temp.clear();
}

uint8_t timer_class::set(uint32_t const &time
                     ,E_TimerType const &A
                     ,E_InputEvent const &B)
{
    // Set the timer type for later use.
    timer_type = A;

    // Set the timer event to return to the caller.
    set_current_event(B);

    uint32_t msecs;

    if (time == 0) {
        msecs = 1;
    } else {
        msecs = time;
    }
    

    // Calculate the resolution and the appropriate timer bits
    const uint32_t timer_calc = (F_CPU / 2000000) * msecs * MILLISECOND_RESOLUTION;
    if (timer_calc < TIMER1_RESOLUTION) {
        timer_select = (1 << CS10);
        period = timer_calc;
    } else
    if (timer_calc < TIMER1_RESOLUTION * 8) {
        timer_select = (1 << CS11);
        period = timer_calc / 8;
    } else
    if (timer_calc < TIMER1_RESOLUTION * 64) {
        timer_select = (1 << CS11) | (1 << CS10);
        period = timer_calc / 64;
    } else
    if (timer_calc < TIMER1_RESOLUTION * 256) {
        timer_select = (1 << CS12);
        period = timer_calc / 256;
    } else
    if (timer_calc < TIMER1_RESOLUTION * 1024) {
        timer_select = (1 << CS12) | (1 << CS10);
        period = timer_calc / 1024;
    } else {
        timer_select = (1 << CS12) | (1 << CS10);
        period = TIMER1_RESOLUTION - 1;
    }

    timerID++;

    // timerID == 0 is a special number.  
    //  Using timerID of 0(zero) will always start/stop
    //  the current timer.
    if (timerID == 0) timerID = 1;
    return timerID;

}

bool timer_class::start(uint8_t const &A) {

    if (!isAttached()) return false; // No observer ... dont bother to start.

    // Disable sleep 
    mcu_sleep_class::getInstance()->SetInterfaceUsage(
        mcu_sleep_class::E_TIMER_ONE_INTERFACE,
        mcu_sleep_class::E_POWER_INTERFACE_DISABLE_POWER_SAVINGS);

    // Init the TCCR1(X) register
    TCCR1B = 0;
    TCCR1A = 0;

    // If A is NOT equal to zero and A does not 
    //  match the current timerID then this isnt the correct
    //  timer.  
    if ((A != 0) && (timerID != A)) return false;

    // Reset period
    ICR1 = TCNT1 = period;

    // Then set with the correct values
    TCCR1B = (1 << WGM13) | timer_select;

    // Enable the timer interrupt
    TIMSK1 |= (1 << TOIE1);

#if DEBUG
    temp.set(get_current_hardware(),E_TIMER_START, A);
    Notify(temp);
#endif

    // Disable sleep 
    mcu_sleep_class::getInstance()->SetInterfaceUsage(
        mcu_sleep_class::E_TIMER_ONE_INTERFACE,
        mcu_sleep_class::E_POWER_INTERFACE_DISABLE_POWER_SAVINGS);

    return true;
}

bool timer_class::stop(uint8_t const &A) {

    // If A is NOT equal to zero and A does not 
    //  match the current timerID then this isnt the correct
    //  timer.  
    if ((A != 0) && (timerID != A)) return false;

    // DISABLE the timer interrupt
    TIMSK1 &= ~(1 << TOIE1);

    // Clear the TCCR1B register
    TCCR1B = 0;
    TCCR1A = 0;

#if DEBUG
    temp.set(get_current_hardware(),E_TIMER_STOP, A);
    Notify(temp);
#endif

    // Enable sleep 
    mcu_sleep_class::getInstance()->SetInterfaceUsage(
        mcu_sleep_class::E_TIMER_ONE_INTERFACE,
        mcu_sleep_class::E_POWER_INTERFACE_ENABLE_POWER_SAVINGS);

    return true;
}

void timer_class::Expired() {

    temp.set(get_current_hardware(), E_TIMER_EXPIRE, timerID);
    Notify(temp);

    if (timer_type != E_REPEATING_TIMER)
    {
        // Stop the timer.. 
        stop(timerID);
    }
    // ELSE the timer will automatically repeat
}

ISR(TIMER1_OVF_vect) {
	timer_class::pTimer->Expired();
}


