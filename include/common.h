#ifndef COMMON_H
#define COMMON_H

#ifdef DEBUG_ON

#include <iostream>
#define DBG(M) std::cerr << " -- dbg: " << M << std::endl;

#else
#define DBG(M)

#endif

namespace strsim {
	class rnd_generator {
	public:
		typedef unsigned int value_type;
		/** Sampling from the generator */
		value_type virtual sample(void) = 0;
	};

}

#endif


