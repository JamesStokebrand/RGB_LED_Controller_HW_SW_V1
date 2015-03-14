#ifndef NEW_H
#define NEW_H

/* Header to define new/delete operators as they aren't provided by avr-gcc by default
   Taken from http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=59453 
 */

/****************************************************
    New implementation

    File:   new.h
    Author: James Stokebrand
    jamesstokebrand AT gmail DOT com

    new.h file is part of the RGB LED Controller and Node 
     version 1 hardware project.

    This file defines the new/delete and new[]/delete[] methods.
    - This code was copied without changes from GitHub:
        https://github.com/arduino/Arduino/tree/master/hardware/arduino/cores/arduino

    Rev History:
     When         Who                Description of change
    -----------  ----------         ------------------------
    2014 Oct 05  James Stokebrand   Initial creation.

*****************************************************/

#include <stdlib.h>

void * operator new(size_t size);
void * operator new[](size_t size);
void operator delete(void * ptr);
void operator delete[](void * ptr);

__extension__ typedef int __guard __attribute__((mode (__DI__)));

extern "C" int __cxa_guard_acquire(__guard *);
extern "C" void __cxa_guard_release (__guard *);
extern "C" void __cxa_guard_abort (__guard *); 

extern "C" void __cxa_pure_virtual(void);

#endif


