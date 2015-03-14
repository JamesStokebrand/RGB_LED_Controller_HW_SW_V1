#ifndef _PWM_SIX_DISPLAY_H_
#define _PWM_SIX_DISPLAY_H_

/****************************************************
    PWM Six Display Class

    File:   pwm_six_display.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    pwm_six_display.h file is part of the RGB LED Controller and Node 
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

#ifndef _PWM_CLASS_H_
#include "pwm_class.h"
#endif

class pwm_six_display
{
public:
    // Display type
    typedef enum {
         E_SIX_DISPLAY_LEFT_TO_RIGHT_255_VALUE
        ,E_SIX_DISPLAY_RIGHT_TO_LEFT_255_VALUE
        ,E_SIX_DISPLAY_DOT_IND_255_VALUE
        ,E_SIX_DISPLAY_DOT_IND_015_VALUE
        ,E_SIX_DISPLAY_LAST_ENUM
    } E_SixDisplayType;

    pwm_six_display(IOPinDefines::E_PinDef const &LED1
                ,IOPinDefines::E_PinDef const &LED2
                ,IOPinDefines::E_PinDef const &LED3
                ,IOPinDefines::E_PinDef const &LED4
                ,IOPinDefines::E_PinDef const &LED5
                ,IOPinDefines::E_PinDef const &LED6
                ,bool const &CommonCathode = true
                ,E_SixDisplayType const &_DisplayInit= E_SIX_DISPLAY_LAST_ENUM
                ,uint8_t const &_StartValue = 0)
    : LED_1(LED1,CommonCathode)
    , LED_2(LED2,CommonCathode)
    , LED_3(LED3,CommonCathode)
    , LED_4(LED4,CommonCathode)
    , LED_5(LED5,CommonCathode)
    , LED_6(LED6,CommonCathode)
    {
        if (_DisplayInit != E_SIX_DISPLAY_LAST_ENUM)
        {
            Display(_DisplayInit,_StartValue);
        }
    }

    virtual ~pwm_six_display() {}

    void On();
    void Off();

    void Display(E_SixDisplayType const &A,uint8_t const &B);

private:
    pwm_class LED_1;
    pwm_class LED_2;
    pwm_class LED_3;
    pwm_class LED_4;
    pwm_class LED_5;
    pwm_class LED_6;

    static const uint8_t _255_LED_TICK_VALUE = 42;
    static const uint8_t _15_LED_TICK_VALUE = 17;
};

#endif
