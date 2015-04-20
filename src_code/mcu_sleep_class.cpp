/****************************************************
    MCU Sleep Class

    File:   mcu_sleep_class.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    mcu_sleep_class.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file contains the code to sleep the ATmaga328p chip.
    There are two parts:  
     - The PRR (power reduction register) which is used to dynamically
        turn off unused hardware interfaces (IE TWI,SPI,Timers etc)
     - Safely set_sleep_mode() method.
     
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
    2014 Nov 18  James Stokebrand   Initial creation.

*****************************************************/

#ifndef _MCU_SLEEP_CLASS_H_
#include "mcu_sleep_class.h"
#endif

mcu_sleep_class* mcu_sleep_class::m_pInstance = nullptr;

mcu_sleep_class* mcu_sleep_class::getInstance()
{
    return m_pInstance ? m_pInstance : (m_pInstance = new mcu_sleep_class);
}

void mcu_sleep_class::DisableSleep()
{
    // Prevent the MCU to go into sleep mode.
    _AllowSleep = false;
}

void mcu_sleep_class::EnableSleep()
{
    // Allow the MCU to go into sleep mode.
    _AllowSleep = true;
}


// These methods enable/disable the status LED during sleep
//  for debugging needs

// Disable status LED
void mcu_sleep_class::DisableStatusLED() {
    // Disable Status LED.
    _EnableStatusLED = false;
    SleepStatusLED.Off();
}

// Enable status LED
void mcu_sleep_class::EnableStatusLED() {
    // Enable Status LED.
    _EnableStatusLED = true;
    SleepStatusLED.On();
}

void mcu_sleep_class::SetInterfaceUsage(E_PowerUsage const &_Interface, E_PowerInterfaceInUse const &_InUse)
{
    // If we didn't enable sleeping ... dont bother to set the bits.
    if (!_AllowSleep) return;

    // Individual modules will register with the sleep class
    //  to notify it that the interface is in use.
    if (_InUse == E_POWER_INTERFACE_DISABLE_POWER_SAVINGS) {
        // Interface is in use.  Set bit to 0(Zero) to enable
        //  and turn on the interface. 
        // IE  Interface is in USE!  Using power.  Dont want to sleep!
        _power_reduction_variable &= ~(1<<_Interface);
    } else if (_InUse == E_POWER_INTERFACE_ENABLE_POWER_SAVINGS) {
        // Interface is no longer in use.  Set to 1(one) to 
        //  turn off the interface to save power.
        // IE  Interface is *NOT* in use!  Turn it off to save power.
        _power_reduction_variable |= (1<<_Interface);
    }

    // Set the PRR
    PRR = _power_reduction_variable;
}

void mcu_sleep_class::GoMakeSleepNow()
{
    // Didn't enable sleep!  Just return.
    if (!_AllowSleep) return;

    // Possible power sleep mode types:
    //   SLEEP_MODE_IDLE - works
    //   SLEEP_MODE_ADC
    //   SLEEP_MODE_PWR_DOWN
    //   SLEEP_MODE_PWR_SAVE
    //   SLEEP_MODE_STANDBY
    //   SLEEP_MODE_EXT_STANDBY

    // select POWER SAVE mode for sleeping
    //  Allows timers and interrupts for wakeup
    set_sleep_mode(SLEEP_MODE_IDLE);

    cli();
    sleep_enable();
    sleep_bod_disable();

    // Toggle the status LED if enabled.
    if (_EnableStatusLED) SleepStatusLED.Toggle();

    sei();
    sleep_cpu();

    // ######################################
    // Have entered sleep here.
    // ######################################

    // Toggle the status LED if enabled.
    if (_EnableStatusLED) SleepStatusLED.Toggle();

    sleep_disable();
    sei();
}

void mcu_sleep_class::SetInputAndPullupResistor(IOPinDefines::E_PinDef const &A)
{
    // Didn't enable sleep!  Just return.
    if (!_AllowSleep) return;

    // This doesn't save much power ... but every little 
    //   bit helps.

    // To save power:
    //  Set unused pin as input
    //  Activate pull up resistor
    IOPinDefines temp(A);

    // Set pin as input
    *(temp._DDRReg) &= ~(1<<temp._Bit);

    // Activate pull up resistor
    *(temp._PortReg) |= (1<<temp._Bit);
}




