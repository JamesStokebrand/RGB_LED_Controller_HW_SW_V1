/****************************************************
    PWM Six Display Class

    File:   pwm_six_display.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    pwm_six_display.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file abstracts the display of information on six LEDs.  The
     interface gives this display a LED interface (IE On, Off, Toggle and more)
    - Six LEDs are wrapped by Software PWM classes
    - Abstract the display of information from 0-255 or 0-15
    - Three types of display:
      . Fill from Left to Right
      . Fill from Right to Left
      . Dot display (PWM)

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
    2014 Nov 11  James Stokebrand   Initial creation.

*****************************************************/

#ifndef _PWM_SIX_DISPLAY_H_
#include "pwm_six_display.h"
#endif

#define ABS(a) ((a)<0?-(a):a)

void pwm_six_display::On()
{
    LED_1.On();
    LED_2.On();
    LED_3.On();
    LED_4.On();
    LED_5.On();
    LED_6.On();
}

void pwm_six_display::Off()
{
    LED_1.Off();
    LED_2.Off();
    LED_3.Off();
    LED_4.Off();
    LED_5.Off();
    LED_6.Off();
}

void pwm_six_display::Display(E_SixDisplayType const &A,uint8_t const &B)
{
    static E_SixDisplayType LastDisplay = E_SIX_DISPLAY_LAST_ENUM;

    // Not a valid display type.  Nothing to do.
    if (A == E_SIX_DISPLAY_LAST_ENUM) return;

    if (A != LastDisplay)
    {
        // Display type changed.
        LastDisplay = A;
        // Turn all the LEDs Off();
        Off();
    }
    

    switch(A)
    {
    case E_SIX_DISPLAY_LEFT_TO_RIGHT_255_VALUE:
        {
            if (B == 255)
            {
                // Value is 255 ... turn them all on.
                On();
                return;
            }
            if (B == 0) 
            {
                // Requested value is zero ... 
                Off();
                return;
            }

            pwm_class* pwm_array[6];

            pwm_array[0] = &LED_6;
            pwm_array[1] = &LED_5;
            pwm_array[2] = &LED_4;
            pwm_array[3] = &LED_3;
            pwm_array[4] = &LED_2;
            pwm_array[5] = &LED_1;

            // Loop through the list of LEDs and adjust
            uint16_t accumulator = (float)B/_255_LED_TICK_VALUE*255;
            for (uint8_t tt = 0; tt < 6; tt++)
            {
                if (accumulator == 0) {
                    pwm_array[tt]->Off();
                } else if (accumulator >= 255) {
                    pwm_array[tt]->On();
                    accumulator -= 255;
                } else if (accumulator < 255) {
                    pwm_array[tt]->setValue(accumulator);
                    accumulator = 0;
                }
            }
        }
    break;
    case E_SIX_DISPLAY_RIGHT_TO_LEFT_255_VALUE:
        {
            if (B == 255)
            {
                // Value is 255 ... turn them all on.
                Off();
                return;
            }
            if (B == 0) 
            {
                // Requested value is zero ... 
                On();
                return;
            }

            pwm_class* pwm_array[6];

            pwm_array[0] = &LED_6;
            pwm_array[1] = &LED_5;
            pwm_array[2] = &LED_4;
            pwm_array[3] = &LED_3;
            pwm_array[4] = &LED_2;
            pwm_array[5] = &LED_1;

            // Loop through the list of LEDs and adjust
            uint16_t accumulator = (float)B/_255_LED_TICK_VALUE*255;
            for (uint8_t tt = 0; tt < 6; tt++)
            {
                if (accumulator == 0) {
                    pwm_array[tt]->On();
                } else if (accumulator >= 255) {
                    pwm_array[tt]->Off();
                    accumulator -= 255;
                } else if (accumulator < 255) {
                    pwm_array[tt]->setValue(255-accumulator);
                    accumulator = 0;
                }
            }
        }
    break;
    case E_SIX_DISPLAY_DOT_IND_255_VALUE:
        {
            pwm_class* pwm_array[8];

            pwm_array[0] = nullptr;
            pwm_array[1] = &LED_6;
            pwm_array[2] = &LED_5;
            pwm_array[3] = &LED_4;
            pwm_array[4] = &LED_3;
            pwm_array[5] = &LED_2;
            pwm_array[6] = &LED_1;
            pwm_array[7] = nullptr;

            // Loop through the list of LEDs and adjust
            uint16_t accumulator = (float)B/_255_LED_TICK_VALUE*255+127;
            for (uint8_t tt = 1; tt < 8; tt++)
            {
                if (accumulator == 0) {
                    if(pwm_array[tt]) pwm_array[tt]->Off();
                } else if (accumulator >= 255) {
                    if(pwm_array[tt]) pwm_array[tt]->Off();
                    accumulator -= 255;
                } else if (accumulator < 255) {
                    if (pwm_array[tt-1]) pwm_array[tt-1]->setValue(255-accumulator);
                    if (pwm_array[tt])   pwm_array[tt]->  setValue(accumulator);
                    accumulator = 0;
                }
            }
        }
    break;
    case E_SIX_DISPLAY_DOT_IND_015_VALUE:
        {
            uint16_t temp = B*_15_LED_TICK_VALUE;
            if (temp > 255) temp = 255;
            Display(E_SIX_DISPLAY_DOT_IND_255_VALUE,temp);
        }
    break;
    case E_SIX_DISPLAY_LAST_ENUM:
        // fall through
    default:
        // Do nothing for these two cases.
    break;
    };
}



