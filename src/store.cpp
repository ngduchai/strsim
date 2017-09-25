#include "store.h"

#define SCALE 1000

void strsim::erlang_generator::setup(unsigned int shape,
		double rate, double delay) {
	_shape = shape;
	_rate = rate;
	_delay = delay;

}
strsim::erlang_generator::value_type strsim::erlang_generator::sample(void) {
	double eln = 1.0;
	for (unsigned int i = 0; i < _shape; ++i) {
		eln *= _dist(_gen);
	}
	return SCALE * (_delay + -1.0 / _rate * std::log(eln));
}


strsim::rnd_generator::value_type strsim::gaussian_generator::sample(void) {
	double smp = 0;
	do {
		smp = _dist(_gen);
	} while (smp < 0);
	return SCALE * smp;
}



