/* Header to define new/delete operators as they aren't provided by avr-gcc by default
   Taken from http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=59453 
 */

/****************************************************
    New implementation

    File:   new.cpp
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    new.cpp file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file defines the new/delete and new[]/delete[] methods.
    - This code was copied without changes from GitHub:
        https://github.com/arduino/Arduino/tree/master/hardware/arduino/cores/arduino

    Rev History:
     When         Who                Description of change
    -----------  ----------         ------------------------
    2014 Oct 05  James Stokebrand   Initial creation.

*****************************************************/

#ifndef NEW_H
#include "new.h"
#endif

void * operator new(size_t size)
{
  return malloc(size);
}

void * operator new[](size_t size)
{
  return malloc(size);
}

void operator delete(void * ptr)
{
  free(ptr);
}

void operator delete[](void * ptr)
{
  free(ptr);
}

int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);};
void __cxa_guard_release (__guard *g) {*(char *)g = 1;};
void __cxa_guard_abort (__guard *) {}; 

void __cxa_pure_virtual(void) {};


