#ifndef _PIN_CLASS_H_
#define _PIN_CLASS_H_

/****************************************************
    Pin Class

    File:   pin_class.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    pin_class.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file implements the input/output pin definitions.  This 
     simplifies the defining of pins by an enum and allows the 
     abstraction of pin resources.  (IE Buttons/LEDs/Rotary Encoders etc)

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
    2014 Aug 14  James Stokebrand   Initial creation.

*****************************************************/


#include <avr/io.h>

#ifndef _OBSERVER_CLASS_H_
#include "observer_class.h"
#endif


#define PIN_CHANGE_INTERRUPT_CONTROL_REGISTER   PCICR

// PCIE0 controls interrupts for the PB[0:7] pins
#define PIN_CHANGE_INTERRUPT_ENABLE_ZERO        PCIE0
// PCIE1 controls interrupts for the PC[0:6] pins
#define PIN_CHANGE_INTERRUPT_ENABLE_ONE         PCIE1
// PCIE2 controls interrupts for the PD[0:7] pins
#define PIN_CHANGE_INTERRUPT_ENABLE_TWO         PCIE2

// Writing a 1 to the ISC10 bit in the EICRA enables interrupts on Rising/Falling edges
#define EXTERNAL_INTERRUPT_CONTROL_REGISTER_A   EICRA
#define INTERRUPT_SENSE_CONTROL_ONE_BIT_ZERO    ISC10

class IOPinDefines
{
public:

    // These are specific to the ATmega328p
    typedef enum {
         E_PIN_PORTB_BASE = 0
        ,E_PIN_PB0 = PB0 + E_PIN_PORTB_BASE
        ,E_PIN_PB1 = PB1 + E_PIN_PORTB_BASE
        ,E_PIN_PB2 = PB2 + E_PIN_PORTB_BASE
        ,E_PIN_PB3 = PB3 + E_PIN_PORTB_BASE
        ,E_PIN_PB4 = PB4 + E_PIN_PORTB_BASE
        ,E_PIN_PB5 = PB5 + E_PIN_PORTB_BASE
        ,E_PIN_PB6 = PB6 + E_PIN_PORTB_BASE
        ,E_PIN_PB7 = PB7 + E_PIN_PORTB_BASE

        ,E_PIN_PORTC_BASE = E_PIN_PB7 + 1
        ,E_PIN_PC0 = PC0 + E_PIN_PORTC_BASE
        ,E_PIN_PC1 = PC1 + E_PIN_PORTC_BASE
        ,E_PIN_PC2 = PC2 + E_PIN_PORTC_BASE
        ,E_PIN_PC3 = PC3 + E_PIN_PORTC_BASE
        ,E_PIN_PC4 = PC4 + E_PIN_PORTC_BASE
        ,E_PIN_PC5 = PC5 + E_PIN_PORTC_BASE
        ,E_PIN_PC6 = PC6 + E_PIN_PORTC_BASE
        // NOTE there is no PC7

        ,E_PIN_PORTD_BASE = E_PIN_PC6 + 1
        ,E_PIN_PD0 = PD0 + E_PIN_PORTD_BASE
        ,E_PIN_PD1 = PD1 + E_PIN_PORTD_BASE
        ,E_PIN_PD2 = PD2 + E_PIN_PORTD_BASE
        ,E_PIN_PD3 = PD3 + E_PIN_PORTD_BASE
        ,E_PIN_PD4 = PD4 + E_PIN_PORTD_BASE
        ,E_PIN_PD5 = PD5 + E_PIN_PORTD_BASE
        ,E_PIN_PD6 = PD6 + E_PIN_PORTD_BASE
        ,E_PIN_PD7 = PD7 + E_PIN_PORTD_BASE
        ,E_PIN_LAST_DEFINE 
    } E_PinDef;

    // Currently only support Disabled or Rise/Falling edge triggered interrupts
    typedef enum {
         E_PIN_INTR_DISABLED
        ,E_PIN_INTR_ANY_EDGE
        ,E_PIN_INTR_LAST_DEFINE
    } E_PinIntrType;

    IOPinDefines(E_PinDef const &aPin, E_PinIntrType const &anIntr = E_PIN_INTR_DISABLED);
    virtual ~IOPinDefines() {}

    volatile uint8_t* _PinReg;
    volatile uint8_t* _PortReg;
    volatile uint8_t* _DDRReg;
    E_PinIntrType _PinInterruptType;
    E_PinDef _Pin;
    uint8_t _Bit;
};

class InputPinClass 
: public IOPinDefines
{
public:
    InputPinClass(IOPinDefines::E_PinDef const &A
                 ,IOPinDefines::E_PinIntrType const &B = IOPinDefines::E_PinIntrType::E_PIN_INTR_DISABLED)
    : IOPinDefines(A,B)
    {
        // Set pin as input
        *(_DDRReg) &= ~(1<<_Bit);

        // Activate pull up resistor
        *(_PortReg) |= (1<<_Bit);
    }
     
    virtual ~InputPinClass() {}

    inline bool Read() __attribute__((always_inline))
    {
        return ((*(_PinReg) & (1<<_Bit)) ? true : false);
    }

};

class OutputPinClass
: public IOPinDefines
{
public:
    OutputPinClass(IOPinDefines::E_PinDef const &A)
    : IOPinDefines(A,IOPinDefines::E_PinIntrType::E_PIN_INTR_DISABLED) 
    { }

    virtual ~OutputPinClass() {}

    virtual void On() = 0;
    virtual void Off() = 0;

    inline void Toggle() __attribute__((always_inline))
    {
        *(_PortReg) ^= (1<<_Bit);
    }
};

class LED_CommonCathode // LED tied to ground 
: public OutputPinClass
{
public:
    LED_CommonCathode(IOPinDefines::E_PinDef const &A)
    : OutputPinClass(A)
    {
        // Init the pin to off 
        Off();

        // Set the pin as an output
        *(_DDRReg) |= (1<<_Bit);
    }

    virtual ~LED_CommonCathode() {}

    virtual inline void On() __attribute__((always_inline))
    {
        *(_PortReg) |= (1<<_Bit);
    }

    virtual inline void Off() __attribute__((always_inline))
    {
        *(_PortReg) &= ~(1<<_Bit);
    }
};

class LED_CommonAnode  // LED tied to V+
: public OutputPinClass
{
public:
    LED_CommonAnode(IOPinDefines::E_PinDef const &A)
    : OutputPinClass(A)
    {
        // Init the pin to off 
        Off();

        // Set the pin as an output
        *(_DDRReg) |= (1<<_Bit);
    }

    virtual ~LED_CommonAnode() {}

    virtual inline void On() __attribute__((always_inline))
    {
        *(_PortReg) &= ~(1<<_Bit);
    }

    virtual inline void Off() __attribute__((always_inline))
    {
        *(_PortReg) |= (1<<_Bit);
    }
};

#endif

