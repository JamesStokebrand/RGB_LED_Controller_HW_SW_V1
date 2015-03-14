#ifndef _STATIC_QUEUE_H_
#define _STATIC_QUEUE_H_

/****************************************************
    Static Queue Class

    File:   static_queue.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    static_queue.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file implements statically allocated queue class of 
     "event_element_class" objects.

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
    2014 Oct 30 James Stokebrand   Initial creation.

*****************************************************/

#include <stdbool.h>

#ifndef _EVENT_LISTING_H_
#include "event_listing.h"
#endif

namespace STATIC_QUEUE_EVENT_LISTING
{

/*
    NOTE NOTE NOTE !!! 
    Due to the use of uint8_t for the front/rear/count variables, the
    max size of this queue is 255!
*/

class cqueue
{
public:
    cqueue()
    : front(-1)
    , rear(-1)
    , count(0)
    , HighWaterValue(0)
    { }

    virtual ~cqueue() {}
    bool Enqueue(event_element_class const &A);
    bool Dequeue(event_element_class &A);

    inline uint8_t ElemNum(void) { return count; }
    inline bool IsEmpty(void) { return ElemNum(); }
    inline uint8_t HighWaterMark(void) { return HighWaterValue; }

private:
    volatile int8_t front,rear;
    volatile uint8_t count;
    static const uint8_t STATIC_QUEUE_DEFAULT_SIZE=32;
    event_element_class data[STATIC_QUEUE_DEFAULT_SIZE];
    uint8_t HighWaterValue;
};

}

#endif

