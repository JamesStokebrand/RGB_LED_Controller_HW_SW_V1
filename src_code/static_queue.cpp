/****************************************************
    Static Queue Class

    File:   static_queue.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    static_queue.cpp file is part of the RGB LED Controller and Node 
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

#include <util/atomic.h>

#ifndef _STATIC_QUEUE_H_
#include "static_queue.h"
#endif

/*
    NOTE NOTE NOTE !!! 
    Due to the use of uint8_t for the front/rear/count variables, the
    max size of this queue is 255!
*/

bool STATIC_QUEUE_EVENT_LISTING::cqueue::Enqueue(event_element_class const &A)
{
    if(((rear==STATIC_QUEUE_DEFAULT_SIZE-1)&&(front==0)) || 
        (front==rear+1))
    {
        return false;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if(rear==-1)
        {
            rear++;
            front++;
        }
        else if(rear==STATIC_QUEUE_DEFAULT_SIZE-1)
        {
            rear=0;
        }
        else
        {
            rear++;
        }

        data[rear] = A;
        count++;
    }

    if (count > HighWaterValue) HighWaterValue = count;

    return true;
}

bool STATIC_QUEUE_EVENT_LISTING::cqueue::Dequeue(event_element_class &A)
{
    if(front==-1)
    {
        return false;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        A = data[front];
        count--;

        if(front==rear)
        {
            front=-1;rear=-1;
        }
        else if(front==STATIC_QUEUE_DEFAULT_SIZE-1)
        {
            front=0;
        }
        else
        {
            front++;
        }
    }
    return true;
}

