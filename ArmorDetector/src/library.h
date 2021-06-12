#ifndef LIBRARY_H
#define LIBRARY_H

#ifndef linux
  #error only able to run on Linux
#endif

#include "detector.h"
#include "config.h"

#define EXPORT

extern "C"
{
    EXPORT void detect(float & x, float & y);
};

#endif // LIBRARY_H
