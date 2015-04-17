/****************************************************
    State Class

    File:   state_class.cpp
    Author: James Stokebrand

    Rev History:
     When         Who                Description of change
    -----------  ----------         ------------------------
    2014 Oct 05  James Stokebrand   Initial Creation

    This is a state machine base class that will help to 
     implement state machines on AVR hardware.

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

