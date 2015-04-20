/****************************************************
    State Class

    File:   state_class.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    state_class.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file is a helper class that will work behind the scenes to help 
      implement state machines.

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

// Process is called with new events pushed into
//  the state machine.
void base_state_class::process(const event_element_class &A)
{ 
    // Normal processing
    (this->*state)(A);

    // State transition requested?
    if (transition)
    {
        // Executing a transition ... exit this state
        (this->*state)(EXIT_EVENT);

        // Set the new state
        state = next_state;

        // Executing a transition ... enter this new state
        (this->*state)(ENTER_EVENT);
        transition = false;
    }
}

// This method is to be called external to the state machine to 
//  force an immediate state transition.
void base_state_class::EXTERNAL_TRAN(volatile STATE target) {
    next_state = target;

    // Executing a transition ... exit this state
    (this->*state)(EXIT_EVENT);

    // Set the new state
    state = next_state;

    // Executing a transition ... enter this new state
    (this->*next_state)(ENTER_EVENT);
    transition = false;
}

