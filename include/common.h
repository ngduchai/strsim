#ifndef COMMON_H
#define COMMON_H

#ifdef DEBUG_ON

#include <iostream>
#define DBG(M) std::cerr << " -- dbg: " << M << std::endl;

#else
#define DBG(M)

#endif

#endif


