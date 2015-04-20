#ifndef _RGB_CONTROLLER_STATE_MACHINE_H_
#define _RGB_CONTROLLER_STATE_MACHINE_H_

/****************************************************
    RGB LED Controller State Machine

    File:   rgb_controller_state_machine.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    rgb_controller_state_machine.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file implements the RGB LED Controller State Machine.  This 
     machine will send commands to the RGB LED Node to change the 
     colors of the common anode RGB LED.

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

#ifndef _STATE_CLASS_H_
#include "state_class.h"
#endif

#ifndef _COMM_CLASS_H_
#include "comm_class.h"
#endif

#ifndef _TIMER_CLASS_H_
#include "timer_class.h"
#endif

#ifndef _PWM_SIX_DISPLAY_H_
#include "pwm_six_display.h"
#endif

#define DEBUG 0

#define ABS(a) ((a)<0?-(a):a)

class rgb_controller_state_machine
:public base_state_class
{
public:

    typedef enum {
         E_COLOR_RGB  = 4  // Red/Green/Blue Color Model
        ,E_COLOR_HSL  = 11 // Hue/Saturation/Luminosity Color Model

        // Must be the last ENUM 
        ,E_COLOR_MODEL_LAST
    } E_ColorModel;


    typedef enum {
         E_STATUS_LED_ENABLED  = 4  // Status LED is ON  - displaying when the MCU is awake.
        ,E_STATUS_LED_DISABLED = 11 // Status LED is OFF - displaying ... nothing.

        // Must be the last ENUM
        ,E_STATUS_LED_LAST_ENUM
    } E_StatusLED;

    rgb_controller_state_machine(EventQueue *event_queue)
    : base_state_class((STATE)&rgb_controller_state_machine::STATE_IDLE)
    , _colorModel(E_COLOR_HSL) // Init to the HSL color model.
    , _statusLED(E_STATUS_LED_DISABLED) // Init Status LED to disabled.
    , _CURRENT_ADDRESS(MIN_ADDRESS)
    , _event_queue(event_queue)
    , _timerID(255)
    , _rotary_encoder_count(0)
    , PwmDisplay(
         IOPinDefines::E_PinDef::E_PIN_PC5
        ,IOPinDefines::E_PinDef::E_PIN_PC4
        ,IOPinDefines::E_PinDef::E_PIN_PC2
        ,IOPinDefines::E_PinDef::E_PIN_PC3
        ,IOPinDefines::E_PinDef::E_PIN_PC1
        ,IOPinDefines::E_PinDef::E_PIN_PC0
        // The true here makes these LEDs common cathode
        ,true)
    {
        // Attach the USART to the event queue
        _Comm.Attach(event_queue);
        _timer.Attach(event_queue);

        // Disable Status LED.
        mcu_sleep_class::getInstance()->DisableStatusLED();
    }

    virtual ~rgb_controller_state_machine() {}

private:

    void STATE_IDLE(event_element_class &A)
    {

#if DEBUG
_Comm.encode(A);
#endif

        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Set the Idle timer
                _timerID = _timer.set(IDLE_TIMEOUT);
                _timer.start(_timerID);
            }
            if (A.get_current_event() == E_EXIT_STATE)
            {
                // Cancel Idle timer
                _timer.stop(_timerID);
                _timerID = 0;
            }
        break;
        case E_TIMER_01:
            if ((A.get_current_event() == E_TIMER_EXPIRE) &&
                (A.get_current_data() == _timerID))
            {
                // Idle Timeout.  Turn off display
                PwmDisplay.Off();
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 has been pressed ... transition
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PRESSED);
            }
        break;
        case E_BUTTON_02:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 2 has been pressed ... transition
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_2_PRESSED);
            }
        break;
        case E_BUTTON_03:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 3 has been pressed ... transition
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_3_PRESSED);
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                send_msg(E_RE_CW);
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                send_msg(E_RE_CCW);
            break;
            case E_BUTTON_IS_PRESSED:
                send_msg(E_RE_PRESSED);
            break;
            case E_BUTTON_IS_RELEASED:
                send_msg(E_RE_RELEASED);
            break;
            default:
                // Ignore all other events
            break;
            }
        break;
        case E_RGB_NODE: // Feedback from the Node

            // Reset the idle timer to make sure to 
            //  turn off the display.

            // Cancel Idle timer
            _timer.stop(_timerID);
            _timerID = 0;

            // Set the Idle timer
            _timerID = _timer.set(IDLE_TIMEOUT);
            _timer.start(_timerID);


            switch(A.get_current_event())
            {
            case E_LED_RED_PWM:
            case E_LED_HUE_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_LEFT_TO_RIGHT_255_VALUE, A.get_current_data());
            break;
            case E_LED_GREEN_PWM:
            case E_LED_SATURATION_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_255_VALUE, A.get_current_data());
            break;
            case E_LED_BLUE_PWM:
            case E_LED_INTENSITY_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_RIGHT_TO_LEFT_255_VALUE, A.get_current_data());
            default:
                // Ignore all other events from Node
            break;
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }

    void STATE_BUTTON_1_PRESSED(event_element_class &A)
    {
#if DEBUG 
_Comm.encode(A);
#endif
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Set the timer
                _timerID = _timer.set(BUTTON_TIMEOUT);
                _timer.start(_timerID);
            }
            if (A.get_current_event() == E_EXIT_STATE)
            {
                // Cancel timer
                _timer.stop(_timerID);
                _timerID = 0;
            }
        break;
        case E_TIMER_01:
            if ((A.get_current_event() == E_TIMER_EXPIRE) &&
                (A.get_current_data() == _timerID))
            {
                // Button 1 press and timeout.  Send ONLY_RED msg
                send_msg(E_ONLY_RED);

                // Sequence complete ... go back to idle
                TRAN((STATE)&rgb_controller_state_machine::STATE_IDLE);
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {

                //  Button 1 press and release.
                if (_colorModel == E_COLOR_RGB)
                {
                    //  Using RGB color model send E_SET_RED msg
                    send_msg(E_SET_RED);
                } else if (_colorModel == E_COLOR_HSL) {
                    //  Using HSL color model send E_SET_HUE msg
                    send_msg(E_SET_HUE);
                }

                // Sequence complete ... go back to idle
                TRAN((STATE)&rgb_controller_state_machine::STATE_IDLE);
            }
        break;
        case E_BUTTON_02:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 and Button 2 are pressed ... transition
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PLUS_BUTTON_2_PRESSED);
            }
        break;
        case E_BUTTON_03:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 and Button 3 are pressed ... transition
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PLUS_BUTTON_3_PRESSED);
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                send_msg(E_RE_CW);
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                send_msg(E_RE_CCW);
            break;
            case E_BUTTON_IS_PRESSED:
                send_msg(E_RE_PRESSED);
            break;
            case E_BUTTON_IS_RELEASED:
                send_msg(E_RE_RELEASED);
            break;
            default:
                // Ignore all other events
            break;
            }
        break;
        case E_RGB_NODE: // Feedback from the Node
            switch(A.get_current_event())
            {
            case E_LED_RED_PWM:
            case E_LED_HUE_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_LEFT_TO_RIGHT_255_VALUE, A.get_current_data());
            break;
            case E_LED_GREEN_PWM:
            case E_LED_SATURATION_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_255_VALUE, A.get_current_data());
            break;
            case E_LED_BLUE_PWM:
            case E_LED_INTENSITY_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_RIGHT_TO_LEFT_255_VALUE, A.get_current_data());
            default:
                // Ignore all other events from Node
            break;
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }

    void STATE_BUTTON_2_PRESSED(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Set the timer
                _timerID = _timer.set(BUTTON_TIMEOUT);
                _timer.start(_timerID);
            }
            if (A.get_current_event() == E_EXIT_STATE)
            {
                // Cancel timer
                _timer.stop(_timerID);
                _timerID = 0;
            }
        break;
        case E_TIMER_01:
            if ((A.get_current_event() == E_TIMER_EXPIRE) &&
                (A.get_current_data() == _timerID))
            {
                // Button 2 press and timeout.  Send ONLY_GREEN msg
                send_msg(E_ONLY_GREEN);

                // Sequence complete ... go back to idle
                TRAN((STATE)&rgb_controller_state_machine::STATE_IDLE);
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 and Button 2 are pressed ... transition
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PLUS_BUTTON_2_PRESSED);
            }   
        break;  
        case E_BUTTON_02:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                //  Button 2 press and release.
                if (_colorModel == E_COLOR_RGB)
                {
                    //  Using RGB color model send E_SET_GREEN msg
                    send_msg(E_SET_GREEN);
                } else if (_colorModel == E_COLOR_HSL) {
                    //  Using HSL color model send E_SET_SATURATION msg
                    send_msg(E_SET_SATURATION);
                }

                // Sequence complete ... go back to idle
                TRAN((STATE)&rgb_controller_state_machine::STATE_IDLE);
            }
        break;
        case E_BUTTON_03:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 and Button 3 are pressed ... transition
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_2_PLUS_BUTTON_3_PRESSED);
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                send_msg(E_RE_CW);
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                send_msg(E_RE_CCW);
            break;
            case E_BUTTON_IS_PRESSED:
                send_msg(E_RE_PRESSED);
            break;
            case E_BUTTON_IS_RELEASED:
                send_msg(E_RE_RELEASED);
            break;
            default:
                // Ignore all other events
            break;
            }
        break;
        case E_RGB_NODE: // Feedback from the Node
            switch(A.get_current_event())
            {
            case E_LED_RED_PWM:
            case E_LED_HUE_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_LEFT_TO_RIGHT_255_VALUE, A.get_current_data());
            break;
            case E_LED_GREEN_PWM:
            case E_LED_SATURATION_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_255_VALUE, A.get_current_data());
            break;
            case E_LED_BLUE_PWM:
            case E_LED_INTENSITY_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_RIGHT_TO_LEFT_255_VALUE, A.get_current_data());
            default:
                // Ignore all other events from Node
            break;
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }

    void STATE_BUTTON_3_PRESSED(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Set the timer
                _timerID = _timer.set(BUTTON_TIMEOUT);
                _timer.start(_timerID);
            }
            if (A.get_current_event() == E_EXIT_STATE)
            {
                // Cancel timer
                _timer.stop(_timerID);
                _timerID = 0;
            }
        break;
        case E_TIMER_01:
            if ((A.get_current_event() == E_TIMER_EXPIRE) &&
                (A.get_current_data() == _timerID))
            {
                // Button 3 press and timeout.  Send ONLY_BLUE msg
                send_msg(E_ONLY_BLUE);

                // Sequence complete ... go back to idle
                TRAN((STATE)&rgb_controller_state_machine::STATE_IDLE);
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 and Button 3 are pressed ... transition
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PLUS_BUTTON_3_PRESSED);
            }
        break;
        case E_BUTTON_02:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 2 and Button 3 are pressed ... transition
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_2_PLUS_BUTTON_3_PRESSED);
            }
        break;
        case E_BUTTON_03:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                //  Button 3 press and release.
                if (_colorModel == E_COLOR_RGB)
                {
                    //  Using RGB color model send E_SET_BLUE msg
                    send_msg(E_SET_BLUE);
                } else if (_colorModel == E_COLOR_HSL) {
                    //  Using HSL color model send E_SET_INTENSITY msg
                    send_msg(E_SET_INTENSITY);
                }

                // Sequence complete ... go back to idle
                TRAN((STATE)&rgb_controller_state_machine::STATE_IDLE);
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                send_msg(E_RE_CW);
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                send_msg(E_RE_CCW);
            break;
            case E_BUTTON_IS_PRESSED:
                send_msg(E_RE_PRESSED);
            break;
            case E_BUTTON_IS_RELEASED:
                send_msg(E_RE_RELEASED);
            break;
            default:
                // Ignore all other events
            break;
            }
        break;
        case E_RGB_NODE: // Feedback from the Node
            switch(A.get_current_event())
            {
            case E_LED_RED_PWM:
            case E_LED_HUE_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_LEFT_TO_RIGHT_255_VALUE, A.get_current_data());
            break;
            case E_LED_GREEN_PWM:
            case E_LED_SATURATION_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_255_VALUE, A.get_current_data());
            break;
            case E_LED_BLUE_PWM:
            case E_LED_INTENSITY_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_RIGHT_TO_LEFT_255_VALUE, A.get_current_data());
            default:
                // Ignore all other events from Node
            break;
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }

    void STATE_BUTTON_1_PLUS_BUTTON_2_PRESSED(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Set the timer
                _timerID = _timer.set(BUTTON_TIMEOUT);
                _timer.start(_timerID);
            }
            if (A.get_current_event() == E_EXIT_STATE)
            {
                // Cancel timer
                _timer.stop(_timerID);
                _timerID = 0;
            }
        break;
        case E_TIMER_01:
            if ((A.get_current_event() == E_TIMER_EXPIRE) &&
                (A.get_current_data() == _timerID))
            {
                // Button 1 plus Button 2 with timeout ... turn all LEDs off.
                send_msg(E_ALL_OFF);

                // Sequence complete ... go back to idle
                TRAN((STATE)&rgb_controller_state_machine::STATE_IDLE);
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                // Button 1 releases ... Button 2 still pressed 
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_2_PRESSED);
            }
        break;
        case E_BUTTON_02:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                // Button 2 released ... Button 1 still pressed
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PRESSED);
            }
        break;
        case E_BUTTON_03:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 and Button 2 and button 3 are pressed .. transition
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PLUS_BUTTON_2_PLUS_BUTTON_3);
                
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                send_msg(E_RE_CW);
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                send_msg(E_RE_CCW);
            break;
            case E_BUTTON_IS_PRESSED:
                send_msg(E_RE_PRESSED);
            break;
            case E_BUTTON_IS_RELEASED:
                send_msg(E_RE_RELEASED);
            break;
            default:
                // Ignore all other events
            break;
            }
        break;
        case E_RGB_NODE: // Feedback from the Node
            switch(A.get_current_event())
            {
            case E_LED_RED_PWM:
            case E_LED_HUE_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_LEFT_TO_RIGHT_255_VALUE, A.get_current_data());
            break;
            case E_LED_GREEN_PWM:
            case E_LED_SATURATION_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_255_VALUE, A.get_current_data());
            break;
            case E_LED_BLUE_PWM:
            case E_LED_INTENSITY_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_RIGHT_TO_LEFT_255_VALUE, A.get_current_data());
            default:
                // Ignore all other events from Node
            break;
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }

    void STATE_BUTTON_1_PLUS_BUTTON_3_PRESSED(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Set the timer
                _timerID = _timer.set(BUTTON_TIMEOUT);
                _timer.start(_timerID);
            }
            if (A.get_current_event() == E_EXIT_STATE)
            {
                // Cancel timer
                _timer.stop(_timerID);
                _timerID = 0;
            }
        break;
        case E_TIMER_01:
            if ((A.get_current_event() == E_TIMER_EXPIRE) &&
                (A.get_current_data() == _timerID))
            {
                // Button 1 plus Button 3 with timeout ... turn all LEDs to 50%
                send_msg(E_ALL_HALF);

                // Sequence complete ... go back to idle
                TRAN((STATE)&rgb_controller_state_machine::STATE_IDLE);
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                // Button 1 released ... Button 3 still pressed
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_3_PRESSED);
            }
        break;
        case E_BUTTON_02:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 Plus Button 2 Plus Button 3 are pressed
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PLUS_BUTTON_2_PLUS_BUTTON_3);
            }
        break;
        case E_BUTTON_03:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                // Button 3 is released ... Button 1 still pressed
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PRESSED);
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                send_msg(E_RE_CW);
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                send_msg(E_RE_CCW);
            break;
            case E_BUTTON_IS_PRESSED:
                send_msg(E_RE_PRESSED);
            break;
            case E_BUTTON_IS_RELEASED:
                send_msg(E_RE_RELEASED);
            break;
            default:
                // Ignore all other events
            break;
            }
        break;
        case E_RGB_NODE: // Feedback from the Node
            switch(A.get_current_event())
            {
            case E_LED_RED_PWM:
            case E_LED_HUE_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_LEFT_TO_RIGHT_255_VALUE, A.get_current_data());
            break;
            case E_LED_GREEN_PWM:
            case E_LED_SATURATION_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_255_VALUE, A.get_current_data());
            break;
            case E_LED_BLUE_PWM:
            case E_LED_INTENSITY_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_RIGHT_TO_LEFT_255_VALUE, A.get_current_data());
            default:
                // Ignore all other events from Node
            break;
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }

    void STATE_BUTTON_2_PLUS_BUTTON_3_PRESSED(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Set the timer
                _timerID = _timer.set(BUTTON_TIMEOUT);
                _timer.start(_timerID);
            }
            if (A.get_current_event() == E_EXIT_STATE)
            {
                // Cancel timer
                _timer.stop(_timerID);
                _timerID = 0;
            }
        break;
        case E_TIMER_01:
            if ((A.get_current_event() == E_TIMER_EXPIRE) &&
                (A.get_current_data() == _timerID))
            {
                // Button 2 plus Button 3 with timeout ... turn all LEDs ON (100%)
                send_msg(E_ALL_ON);

                // Sequence complete ... go back to idle
                TRAN((STATE)&rgb_controller_state_machine::STATE_IDLE);
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 plus Button 2 plus Button 3 are pressed ...
                //   Transition to STATE_BUTTON_1_PLUS_BUTTON_2_PLUS_BUTTON_3
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PLUS_BUTTON_2_PLUS_BUTTON_3);
            }
        break;
        case E_BUTTON_02:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                // Button 2 released ... Button 3 still pressed
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_3_PRESSED);
            }
        break;
        case E_BUTTON_03:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                // Button 3 released ... Button 2 still pressed
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_2_PRESSED);
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                send_msg(E_RE_CW);
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                send_msg(E_RE_CCW);
            break;
            case E_BUTTON_IS_PRESSED:
                send_msg(E_RE_PRESSED);
            break;
            case E_BUTTON_IS_RELEASED:
                send_msg(E_RE_RELEASED);
            break;
            default:
                // Ignore all other events
            break;
            }
        break;
        case E_RGB_NODE: // Feedback from the Node
            switch(A.get_current_event())
            {
            case E_LED_RED_PWM:
            case E_LED_HUE_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_LEFT_TO_RIGHT_255_VALUE, A.get_current_data());
            break;
            case E_LED_GREEN_PWM:
            case E_LED_SATURATION_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_255_VALUE, A.get_current_data());
            break;
            case E_LED_BLUE_PWM:
            case E_LED_INTENSITY_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_RIGHT_TO_LEFT_255_VALUE, A.get_current_data());
            default:
                // Ignore all other events from Node
            break;
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }

    void STATE_BUTTON_1_PLUS_BUTTON_2_PLUS_BUTTON_3(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Set the timer
                _timerID = _timer.set(BUTTON_TIMEOUT);
                _timer.start(_timerID);
            }
            if (A.get_current_event() == E_EXIT_STATE)
            {
                // Cancel timer
                _timer.stop(_timerID);
                _timerID = 0;
            }
        break;
        case E_TIMER_01:
            if ((A.get_current_event() == E_TIMER_EXPIRE) &&
                (A.get_current_data() == _timerID))
            {
                // Button 1 plus Button 2 plus Button 3 with timeout ... 

                //  Transition to LED_SELECT state
                TRAN((STATE)&rgb_controller_state_machine::STATE_LED_SELECT);
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                // Button 1 released ... Button 2 and Button 3 still pressed
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_2_PLUS_BUTTON_3_PRESSED);
            }
        break;
        case E_BUTTON_02:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                // Button 2 released ... Button 1 and Button 3 still pressed
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PLUS_BUTTON_3_PRESSED);
            }
        break;
        case E_BUTTON_03:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                // Button 3 released ... Button 1 and Button 2 still pressed
                TRAN((STATE)&rgb_controller_state_machine::STATE_BUTTON_1_PLUS_BUTTON_2_PRESSED);
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                send_msg(E_RE_CW);
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                send_msg(E_RE_CCW);
            break;
            case E_BUTTON_IS_PRESSED:
                send_msg(E_RE_PRESSED);
            break;
            case E_BUTTON_IS_RELEASED:
                send_msg(E_RE_RELEASED);
            break;
            default:
                // Ignore all other events
            break;
            }
        break;
        case E_RGB_NODE: // Feedback from the Node
            switch(A.get_current_event())
            {
            case E_LED_RED_PWM:
            case E_LED_HUE_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_LEFT_TO_RIGHT_255_VALUE, A.get_current_data());
            break;
            case E_LED_GREEN_PWM:
            case E_LED_SATURATION_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_255_VALUE, A.get_current_data());
            break;
            case E_LED_BLUE_PWM:
            case E_LED_INTENSITY_PWM:
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_RIGHT_TO_LEFT_255_VALUE, A.get_current_data());
            default:
                // Ignore all other events from Node
            break;
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }

    void STATE_LED_SELECT(event_element_class &A)
    {

#if DEBUG
_Comm.encode(A);
#endif
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Turn the display on to indicate a change of state.
                PwmDisplay.On();

                // Set the RE CW/CCW count to zero
                _rotary_encoder_count = 0;

                // Reset the Node LED adjust value
                send_msg(E_RE_RELEASED);

                // Send the SELECT msg to NODE to 
                //  indicate to the user which
                //  nodes are currently selected.
                send_msg(E_SELECT);

                // Display the current address
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_015_VALUE, _CURRENT_ADDRESS);
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 has been pressed ... notify the user what nodes are
                //  currently selected
                send_msg(E_SELECT);
            }
        break;
        case E_BUTTON_02:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button2 pressed.  Go to Awake status LED select state.
                TRAN((STATE)&rgb_controller_state_machine::STATE_LED_SELECT_AWAKE_LED_STATUS);
            }
        break;
        case E_BUTTON_03:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button3 pressed.  Go to color mode select state.
                TRAN((STATE)&rgb_controller_state_machine::STATE_LED_SELECT_COLOR_MODE_SELECT);
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                _rotary_encoder_count++;
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                _rotary_encoder_count--;
            break;
            case E_BUTTON_IS_PRESSED:
                // RE Button press and release is the sequence to exit LED_SELECT mode
                // Transition to await the RE release.
                TRAN((STATE)&rgb_controller_state_machine::STATE_LED_SELECT_PRESSED);
            break;
            default:
                // Ignore all other events
            break;
            }

            if (ABS(_rotary_encoder_count) > ROTARY_ENCODER_COUNT_FOR_ADDRESS_CHANGE)
            {
                int16_t temp;
                if (_rotary_encoder_count > 0)
                {
                    temp = _CURRENT_ADDRESS++;
                    if (temp > MAXIMUM_ADDRESS)
                    {
                        _CURRENT_ADDRESS = MAXIMUM_ADDRESS;
                    }
                } else { // RE count < 0
                    temp = _CURRENT_ADDRESS--;
                    if (temp <= MIN_ADDRESS)
                    {
                        _CURRENT_ADDRESS = MIN_ADDRESS;
                    }
                }
                send_msg(E_SELECT);
                _rotary_encoder_count=0;
                // Display the current address
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_015_VALUE, _CURRENT_ADDRESS);
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }

    void STATE_LED_SELECT_COLOR_MODE_SELECT(event_element_class &A)
    {

#if DEBUG
_Comm.encode(A);
#endif

        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Set the RE CW/CCW count to zero
                _rotary_encoder_count = 0;

                // Send the SELECT msg to NODE to 
                //  indicate to the user which
                //  nodes are currently selected.
                send_msg(E_SELECT);

                // Display the current color model.
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_015_VALUE, (uint8_t)_colorModel);
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 has been pressed ... notify the user what nodes are
                //  currently selected
                send_msg(E_SELECT);
            }
        break;
        case E_BUTTON_03:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                // Button3 released.  Go back to STATE_LED_SELECT
                TRAN((STATE)&rgb_controller_state_machine::STATE_LED_SELECT);
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                _rotary_encoder_count++;
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                _rotary_encoder_count--;
            break;
            case E_BUTTON_IS_PRESSED:
                // RE Button press and release is the sequence to exit LED_SELECT mode
                // Transition to await the RE release.
                TRAN((STATE)&rgb_controller_state_machine::STATE_LED_SELECT_PRESSED);
            break;
            default:
                // Ignore all other events
            break;
            }

            if (ABS(_rotary_encoder_count) > ROTARY_ENCODER_COUNT_FOR_ADDRESS_CHANGE)
            {
                if (_rotary_encoder_count > 0)
                {
                    // Select E_COLOR_HSL
                    _colorModel = E_COLOR_HSL;
                } else { // RE count < 0
                    // Select E_COLOR_RGB
                    _colorModel = E_COLOR_RGB;
                }
                send_msg(E_SELECT);
                _rotary_encoder_count=0;
                // Display the color model.
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_015_VALUE, (uint8_t)_colorModel);
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }

    void STATE_LED_SELECT_AWAKE_LED_STATUS(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif

        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Set the RE CW/CCW count to zero
                _rotary_encoder_count = 0;

                // Send the SELECT msg to NODE to 
                //  indicate to the user which
                //  nodes are currently selected.
                send_msg(E_SELECT);

                // Display the current color model.
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_015_VALUE, (uint8_t)_statusLED);
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 has been pressed ... notify the user what nodes are
                //  currently selected
                send_msg(E_SELECT);
            }
        break;
        case E_BUTTON_02:
            if (A.get_current_event() == E_BUTTON_IS_RELEASED)
            {
                // Button2 released.  Go back to STATE_LED_SELECT
                TRAN((STATE)&rgb_controller_state_machine::STATE_LED_SELECT);
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                _rotary_encoder_count++;
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                _rotary_encoder_count--;
            break;
            case E_BUTTON_IS_PRESSED:
                // RE Button press and release is the sequence to exit LED_SELECT mode
                // Transition to await the RE release.
                TRAN((STATE)&rgb_controller_state_machine::STATE_LED_SELECT_PRESSED);
            break;
            default:
                // Ignore all other events
            break;
            }

            if (ABS(_rotary_encoder_count) > ROTARY_ENCODER_COUNT_FOR_ADDRESS_CHANGE)
            {
                if (_rotary_encoder_count > 0) {
                    // Disable the status LED
                    _statusLED = E_STATUS_LED_DISABLED; // Indicate the current status
                    send_msg(E_DISABLE_STATUS_LED); // Notify the node to DISABLE
                    mcu_sleep_class::getInstance()->DisableStatusLED(); // Disable the controller's status LED.
                } else { // RE count < 0
                    // Enable the status LED.
                    _statusLED = E_STATUS_LED_ENABLED; // Indicate the current status
                    send_msg(E_ENABLE_STATUS_LED); // Notify the node to ENABLE
                    mcu_sleep_class::getInstance()->EnableStatusLED(); // Enable the controller's status LED.
                }
                send_msg(E_SELECT);
                _rotary_encoder_count=0;
                // Display the status of the status LED.
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_015_VALUE, (uint8_t)_statusLED);
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }

    void STATE_LED_SELECT_PRESSED(event_element_class &A)
    {
#if DEBUG
_Comm.encode(A);
#endif
        switch(A.get_current_hardware())
        {
        case E_STATE_MACHINE:
            if (A.get_current_event() == E_ENTER_STATE)
            {
                // Display the current address
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_015_VALUE, _CURRENT_ADDRESS);
            }
            else if (A.get_current_event() == E_EXIT_STATE)
            {
                // Set the RE CW/CCW count to zero
                _rotary_encoder_count = 0;

                // Turn off the display when existing this state
                PwmDisplay.Off();

                // Force the node to send color feedback
                send_msg(E_FORCE_FEEDBACK);
            }
        break;
        case E_BUTTON_01:
            if (A.get_current_event() == E_BUTTON_IS_PRESSED)
            {
                // Button 1 has been pressed ... notify the user what nodes are
                //  currently selected
                send_msg(E_SELECT);
            }
        break;
        case E_ROTARY_ENCODER_01:
            switch(A.get_current_event())
            {
            case E_ROTARY_ENCODER_ROTATED_CW:
                _rotary_encoder_count++;
            break;
            case E_ROTARY_ENCODER_ROTATED_CCW:
                _rotary_encoder_count--;
            break;
            case E_BUTTON_IS_RELEASED:
                // RE Button press and release is the sequence to exit LED_SELECT mode
                // Transition to STATE_IDLE
                TRAN((STATE)&rgb_controller_state_machine::STATE_IDLE);
            break;
            default:
                // Ignore all other events
            break;
            }

            if (ABS(_rotary_encoder_count) > ROTARY_ENCODER_COUNT_FOR_ADDRESS_CHANGE)
            {
                int16_t temp;
                if (_rotary_encoder_count > 0)
                {
                    temp = _CURRENT_ADDRESS++;
                    if (temp >= MAXIMUM_ADDRESS) 
                    {
                        _CURRENT_ADDRESS = MAXIMUM_ADDRESS;
                    }
                } else { // RE count < 0
                    temp = _CURRENT_ADDRESS--;
                    if (temp <= MIN_ADDRESS)
                    {
                        _CURRENT_ADDRESS = MIN_ADDRESS;
                    }
                }
                send_msg(E_SELECT);
                _rotary_encoder_count=0;
                // Display the current address
                PwmDisplay.Display(pwm_six_display::E_SixDisplayType::E_SIX_DISPLAY_DOT_IND_015_VALUE, _CURRENT_ADDRESS);
            }
        break;
        default:
            // Ignore all other msgs
        break;
        }
    }
    
    void send_msg(E_InputEvent const &event)
    {
        event_element_class _temp;

        // Assemble the msg
        _temp.set(E_RGB_CONTROLLER,event,_CURRENT_ADDRESS);
        // Send via comm
        _Comm.encode(_temp);
    }

    // Comm Class
    comm_class _Comm;

    // Define the current color model
    E_ColorModel _colorModel;

    // Turn on the Status LED indicating when the MCU is awake
    E_StatusLED _statusLED;

    // Node address is the address read from the DIP switches
    uint8_t _CURRENT_ADDRESS;
    // Node 4 button DIP can support a maximum address space of 2^4 (16)
    //  Addrees 0(zero) is a special address which communicates to all
    //  nodes.
    static const uint8_t MAXIMUM_ADDRESS = 15;
    static const int8_t MIN_ADDRESS = 0;
    static const uint8_t ROTARY_ENCODER_COUNT_FOR_ADDRESS_CHANGE = 20;

    EventQueue *_event_queue;

    timer_class _timer;
    uint8_t _timerID;

    // Standard Button Timeout in MS.  (IE 2s)
    static const uint16_t BUTTON_TIMEOUT = 2000U;

    // Standard Idle Timeout in MS.  (IE 1 min = 60s = 60*100 10xms = 60,000ms)
    static const uint16_t IDLE_TIMEOUT = 40*1000U;

    int8_t _rotary_encoder_count;

    pwm_six_display PwmDisplay;
};


#endif

