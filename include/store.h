#ifndef STORE_H
#define STORE_H

#include "code.h"
#include "common.h"
#include <vector>
#include <random>
#include <math.h>

namespace strsim {

	class erlang_generator : public rnd_generator {
	public:
		erlang_generator() : _gen(std::random_device()()), _dist(0, 1) {};
		erlang_generator(unsigned int shape, double rate, double delay) : 
				erlang_generator() {
			setup(shape, rate, delay);		
		}
		void setup(unsigned int shape, double rate, double delay);
		value_type virtual sample(void);
	private:
		std::mt19937 _gen;
		std::uniform_real_distribution<double> _dist;
		unsigned int _shape;
		double _rate;
		double _delay;
	};

}

#endif

