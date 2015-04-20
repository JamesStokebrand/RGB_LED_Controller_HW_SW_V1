#ifndef _MCU_SLEEP_CLASS_H_
#define _MCU_SLEEP_CLASS_H_

/****************************************************
    MCU Sleep Class

    File:   mcu_sleep_class.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    mcu_sleep_class.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file contains the code to sleep the ATmaga328p chip.
    There are two parts:  
     - The PRR (power reduction register) which is used to dynamically
        turn off unused hardware interfaces (IE TWI,SPI,Timers etc)
     - Safely set_sleep_mode() method.
     - This class is a singleton.
     
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

#include <avr/interrupt.h>
#include <avr/sleep.h>

#ifndef _EVENT_LISTING_H_
#include "event_listing.h"
#endif

#ifndef _PIN_CLASS_H_
#include "pin_class.h"
#endif

class mcu_sleep_class
{
public:
    // Define each bit in the PRR register
    typedef enum {
         E_TWI_INTERFACE        = PRTWI
        ,E_TIMER_ZERO_INTERFACE = PRTIM0
        ,E_TIMER_ONE_INTERFACE  = PRTIM1
        ,E_TIMER_TWO_INTERFACE  = PRTIM2
        ,E_SPI_INTERFACE        = PRSPI
        ,E_USART_INTERFACE      = PRUSART0
        ,E_ADC_INTERFACE        = PRADC

        // Must remain the last enum
        ,E_LAST_POWER_USE_ENUM
    } E_PowerUsage;

    // These map from the sleep.h defines
    typedef enum {
         E_MCU_SLEEP_MODE_IDLE
        ,E_MCU_SLEEP_MODE_ADC
        ,E_MCU_SLEEP_MODE_PWR_DOWN
        ,E_MCU_SLEEP_MODE_PWR_SAVE
        ,E_MCU_SLEEP_MODE_STANDBY
        ,E_MCU_SLEEP_MODE_EXT_STANDBY

        // Must remain the last enum
        ,E_MCU_SLEEP_MODE_LAST_ENUM
    } E_PowerSleepMode;

    typedef enum {
         E_POWER_INTERFACE_DISABLE_POWER_SAVINGS = 0
        ,E_POWER_INTERFACE_ENABLE_POWER_SAVINGS

        // Must be the last enum
        ,E_LAST_POWER_INTERFACE_ENUM
    } E_PowerInterfaceInUse;

    static mcu_sleep_class* getInstance();
    void DisableSleep();
    void EnableSleep();

    // Set the sleep mode
    void SetSleepMode(E_PowerSleepMode const &A);

    // These methods enable/disable the status LED for
    //  debugging
    void EnableStatusLED();
    void DisableStatusLED();

    void SetInterfaceUsage(E_PowerUsage const &_Interface, E_PowerInterfaceInUse const &_InUse);
    void GoMakeSleepNow();

    // To save power, set unused pins as input and turn on the pull up resistors
    void SetInputAndPullupResistor(IOPinDefines::E_PinDef const &A);

private:
    // Constructor is private for singleton
    mcu_sleep_class()
    : _AllowSleep(false)
    , _PowerSleepMode(E_MCU_SLEEP_MODE_IDLE) // default to Idle
    , SleepStatusLED(IOPinDefines::E_PinDef::E_PIN_PD2)
    , _EnableStatusLED(false)
    {

        // Disable status LED.
        DisableStatusLED();

        // Disable all interfaces
        _power_reduction_variable =
              (1<<PRTWI)     // turn off TWI
            | (1<<PRTIM0)    // turn off Timer/Counter0
            | (1<<PRTIM1)    // turn off Timer/Counter1
            | (1<<PRTIM2)    // turn off Timer/Counter2
            | (1<<PRSPI)     // turn off SPI
            | (1<<PRUSART0)  // turn off USART (will turn on again when reset)
            | (1<<PRADC);    // turn off ADC
    }

    // Copy constructor is private for singleton
    mcu_sleep_class(mcu_sleep_class const&);

    // Reference to itself
    static mcu_sleep_class* m_pInstance;

    // Destructor is private for singletons
    virtual ~mcu_sleep_class() {}

    // Equal operator is private for singletons
    void operator=(mcu_sleep_class const&);

    volatile uint8_t _power_reduction_variable;
    bool _AllowSleep;

    volatile E_PowerSleepMode _PowerSleepMode;

    // Status LED (Yellow)
    LED_CommonCathode SleepStatusLED;
    bool _EnableStatusLED;
};

#endif

