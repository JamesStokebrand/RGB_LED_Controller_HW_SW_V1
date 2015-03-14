/****************************************************
    RGB LED Controller v1 hardware

    File:   main.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    main.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file contains the main() method and the forever loop.  It 
     creates the RGB Controller State machine and the code to sleep the AVR
     chip.
     
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

/*****************************************************

AVR Chip Fuses:

--- FUSES ---
BODLEVEL    = DISABLED
RSTDISBL    = [ ]
DWEN        = [ ]
SPIEN       = [X]
WDTON       = [ ]
EESAVE      = [ ]
BOOTSZ      = 2048W_3800
BOOTRST     = [ ]
CKDIV8      = [ ]
CKOUT       = [ ]
SUT_CKSEL   = INTRCOSC_8MHZ_6CK_14CK_0MS

EXTENDED    = 0xFF (valid)
HIGH        = 0xD9 (valid)
LOW         = 0xC2 (valid)

--- LOCK BITS ---
LB      = NO_LOCK
BLB0    = NO_LOCK
BLB1    = NO_LOCK

LOCKBIT = 0xFF (valid)

*****************************************************/

#include <avr/io.h>
//#include <util/delay.h>
#include <avr/interrupt.h>

#ifndef NEW_H
#include "new.h"
#endif

#ifndef _BUTTON_CLASS_H_
#include "button_class.h"
#endif

#ifndef _EVENT_QUEUE_H_
#include "event_queue.h"
#endif

#ifndef _ROTARY_ENCODER_CLASS_H_
#include "rotary_encoder_class.h"
#endif

#ifndef _COMM_CLASS_H_
#include "comm_class.h"
#endif

#ifndef _RGB_CONTROLLER_STATE_MACHINE_H_
#include "rgb_controller_state_machine.h"
#endif

#ifndef _PWM_SIX_DISPLAY_H_
#include "pwm_six_display.h"
#endif

#ifndef _MCU_SLEEP_CLASS_H_
#include "mcu_sleep_class.h"
#endif

int main(void)
{
    // Event queue that really ties the room together
    EventQueue event_queue;

#if 0
    // Status LED (Yellow)
    LED_CommonCathode StatusLED(IOPinDefines::E_PinDef::E_PIN_PD2);
#endif

    // BLUE
    // Buttons (count three)
    ButtonClass Button_1(E_BUTTON_01
                        ,IOPinDefines::E_PinDef::E_PIN_PD7
                        ,IOPinDefines::E_PinIntrType::E_PIN_INTR_ANY_EDGE);
    Button_1.Attach(&event_queue);

    // GREEN
    ButtonClass Button_2(E_BUTTON_02
                        ,IOPinDefines::E_PinDef::E_PIN_PD6
                        ,IOPinDefines::E_PinIntrType::E_PIN_INTR_ANY_EDGE);
    Button_2.Attach(&event_queue);

    // RED
    ButtonClass Button_3(E_BUTTON_03
                        ,IOPinDefines::E_PinDef::E_PIN_PD5
                        ,IOPinDefines::E_PinIntrType::E_PIN_INTR_ANY_EDGE);
    Button_3.Attach(&event_queue);


    // Rotary Encoder (CCW/CW and button press/release)
    RotaryEncoderClass RotaryEncoder(E_ROTARY_ENCODER_01
                        ,IOPinDefines::E_PinDef::E_PIN_PB1
                        ,IOPinDefines::E_PinDef::E_PIN_PB2
                        ,IOPinDefines::E_PinIntrType::E_PIN_INTR_ANY_EDGE);
    RotaryEncoder.Attach(&event_queue);

    ButtonClass RotaryEncoderButton(E_ROTARY_ENCODER_01
                        ,IOPinDefines::E_PinDef::E_PIN_PB0
                        ,IOPinDefines::E_PinIntrType::E_PIN_INTR_ANY_EDGE);
    RotaryEncoderButton.Attach(&event_queue);

    // RGB Controller state machine
    rgb_controller_state_machine RGB_Controller(&event_queue);

    // Enable MCU sleep
    mcu_sleep_class::getInstance()->EnableSleep();

    sei();

    event_element_class anEvent;

    for (;;) 
    {
        if (event_queue.Dequeue(anEvent))
        {

            // Process events through the 
            //  RGB Controller state machine
            RGB_Controller.process(anEvent);

        } else {

            // Nothing in the queue ... go to sleep
            mcu_sleep_class::getInstance()->GoMakeSleepNow();

        }
    }

    return 0;
}


