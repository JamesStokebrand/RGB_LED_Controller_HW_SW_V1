#ifndef _EVENT_LISTING_H_
#define _EVENT_LISTING_H_

/****************************************************
    AVR Event Listing 

    File:   event_listing.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    event_listing.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This code defines what events are sent through the event queue.
    Defines the event element class to carry the data through the system.

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
    2014 Aug 05  James Stokebrand   Initial creation.
    2014 Aug 06  James Stokebrand   Updated to separate hardware with
                                      Possible events

*****************************************************/

typedef enum {
    // Buttons
     E_BUTTON_01        = 0x00
    ,E_BUTTON_02        // 0x01
    ,E_BUTTON_03        // 0x02
    ,E_BUTTON_04        // 0x03

    // Timers
    ,E_TIMER_01         // 0x04
     
    // USARTs
    ,E_UART_00          // 0x05

    // Rotary Encoder 
    ,E_ROTARY_ENCODER_01 // 0x06

    // State Machine
    ,E_STATE_MACHINE    // 0x07

    // RGB LED Controller Hardware
    ,E_RGB_CONTROLLER     // 0x08

    // RGB LED Node Hardware
    ,E_RGB_NODE           // 0x09

    // SPI Hardware
    ,E_SPI_01             // 0x0A

    // EEPROM Hardware (makes use of spi)
    ,E_EEPROM_01          // 0x0B

    // TWI/I2C Hardware
    ,E_TWI_01             // 0x0C

    // BlinkM Hardware (makes use of TWI)
    ,E_BLINKM_01          // 0x0D

    // Must be the last item on the list
    ,E_LAST_HARDWARE_EVENT 
} E_InputHardware;

// List of possible events 
typedef enum {
    // Button specific
     E_BUTTON_IS_RELEASED  = 0x00
    ,E_BUTTON_IS_PRESSED  // 0x01

    // Timer specific
    ,E_TIMER_START        // 0x02
    ,E_TIMER_STOP         // 0x03
    ,E_TIMER_EXPIRE       // 0x04

    // USART specific
    ,E_UART_TX_COMPLETE   // 0x05
    ,E_UART_RX_EVENT      // 0x06
    ,E_UART_FLAG_BYTE_FOUND_EVENT // 0x07

    // Rotary Encoder specific
    ,E_ROTARY_ENCODER_ROTATED_CW  // 0x08
    ,E_ROTARY_ENCODER_ROTATED_CCW // 0x09

    // State machine specific
    ,E_ENTER_STATE        // 0x0A
    ,E_EXIT_STATE         // 0x0B

    // RGB Controller specific
    //  RGB Color methods
    ,E_SET_RED             = 0x10
    ,E_SET_GREEN          // 0x11
    ,E_SET_BLUE           // 0x12
    //  HSI Color methods
    ,E_SET_HUE            // 0x13
    ,E_SET_SATURATION     // 0x14
    ,E_SET_INTENSITY      // 0x15
    //  Rotary Encoder
    ,E_RE_CW              // 0x16
    ,E_RE_CCW             // 0x17
    ,E_RE_PRESSED         // 0x18
    ,E_RE_RELEASED        // 0x19
    //  Button Combos
    ,E_ONLY_RED           // 0x1A
    ,E_ONLY_GREEN         // 0x1B
    ,E_ONLY_BLUE          // 0x1C
    ,E_ALL_OFF            // 0x1D
    ,E_ALL_HALF           // 0x1E
    ,E_ALL_ON             // 0x1F
    //  Node Selection
    ,E_SELECT             // 0x20
    ,E_FORCE_FEEDBACK     // 0x21

    // RGB Node specific
    //  RGB Color
    ,E_LED_RED_PWM         = 0x30
    ,E_LED_GREEN_PWM      // 0x31
    ,E_LED_BLUE_PWM       // 0x32
    //  HSL Color 
    ,E_LED_HUE_PWM        // 0x33
    ,E_LED_SATURATION_PWM // 0x34
    ,E_LED_INTENSITY_PWM  // 0x35

    // SPI Baseline
    ,E_SPI_BYTE_COMPLETE   = 0x40
    ,E_SPI_MSG_COMPLETE   // 0x41 Notified when the SPI queue is empty (IE msg complete)

    // EEPROM msg responses
    ,E_EEPROM_READ_REQUEST = 0x50
    ,E_EEPROM_READ_COMPLETE
    ,E_EEPROM_WRITE_REQUEST
    ,E_EEPROM_WRITE_COMPLETE
    ,E_EEPROM_STATUS_REQUEST
    ,E_EEPROM_STATUS_COMPLETE

    // TWI Baseline
    ,E_TWI_WRITE_COMPLETE  = 0x60
    ,E_TWI_READ_COMPLETE
    ,E_TWI_ERROR_EVENT

    // BlinkM msg responses
    //  Unexpected TWI response for the current state
    ,E_BLINKM_UNEXPECTED_TWI_RESPONSE_FOR_STATE = 0x70

    //  Get Current RGB Color Responses
    ,E_BLINKM_GET_RGB_RED_RESPONSE
    ,E_BLINKM_GET_RGB_GREEN_RESPONSE
    ,E_BLINKM_GET_RGB_BLUE_RESPONSE
    ,E_BLINKM_GET_RGB_COLOR_ERROR

    // Read Script Line Response
    ,E_BLINKM_READ_SCRIPT_LINE_DURATION_RESPONSE
    ,E_BLINKM_READ_SCRIPT_LINE_COMMAND_RESPONSE
    ,E_BLINKM_READ_SCRIPT_LINE_ARG1_RESPONSE
    ,E_BLINKM_READ_SCRIPT_LINE_ARG2_RESPONSE
    ,E_BLINKM_READ_SCRIPT_LINE_ARG3_RESPONSE
    ,E_BLINKM_READ_SCRIPT_LINE_ERROR

    // Get Address Response
    ,E_BLINKM_GET_ADDRESS_RESPONSE
    ,E_BLINKM_GET_ADDRESS_ERROR

    // Get Firmware Version
    ,E_BLINKM_GET_FIRMWARE_MAJOR_VERSION_RESPONSE
    ,E_BLINKM_GET_FIRMWARE_MINOR_VERSION_RESPONSE
    ,E_BLINKM_GET_FIRMWARE_VERSION_ERROR

    // Must remain the last item on the list
    ,E_LAST_INPUT_EVENT   
} E_InputEvent;

class event_element_class
{
public:
    event_element_class(E_InputHardware const A = E_LAST_HARDWARE_EVENT
                       ,E_InputEvent const B = E_LAST_INPUT_EVENT
                       ,uint8_t const C = 0)
    : aHardware(A)
    , anEvent(B)
    , theData(C)
    {}

    virtual ~event_element_class() {}

    void set(E_InputHardware const A, E_InputEvent const B, uint8_t const C=0)
    {
        aHardware = A;
        anEvent = B;
        theData = C;
    }

    void set(event_element_class const &A)
    {
        set(A.get_current_hardware()
           ,A.get_current_event()
           ,A.get_current_data());
    }

    void get(E_InputHardware &A, E_InputEvent &B, uint8_t &C)
    {
        A = aHardware;
        B = anEvent;
        C = theData;
    }

    void get(event_element_class &A)
    {
        A.set(aHardware, anEvent, theData);
    }

    E_InputHardware get_current_hardware() const
    {
        return aHardware;
    }

    void set_current_hardware(E_InputHardware const &A)
    {
        aHardware = A;
    }

    E_InputEvent get_current_event() const
    {
        return anEvent;
    }

    void set_current_event(E_InputEvent const &A)
    {
        anEvent = A;
    }

    uint8_t get_current_data() const
    {
        return theData;
    }

    void set_current_data(uint8_t const &A)
    {
        theData = A;
    }

    bool operator == (event_element_class const &A) const {
        if ((A.get_current_hardware() == get_current_hardware()) &&
            (A.get_current_event()    == get_current_event())    &&
            (A.get_current_data()     == get_current_data()))
        {
            return true;
        }
        return false;
    }

    bool operator != (event_element_class const &A) const {
        if ((A.get_current_hardware() != get_current_hardware()) ||
            (A.get_current_event()    != get_current_event())    ||
            (A.get_current_data()     != get_current_data()))
        {
            return true;
        }
        return false;
    }

    event_element_class& operator = (event_element_class const &A) {
        if (this != &A)
        {
            set(A.aHardware,A.anEvent,A.theData);
        }
        return *this;
    }

    void clear() 
    {
        set(E_LAST_HARDWARE_EVENT,E_LAST_INPUT_EVENT,0);
    }

private:
    E_InputHardware aHardware;
    E_InputEvent anEvent;
    uint8_t theData;
};


#endif


