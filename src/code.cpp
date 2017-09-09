#include "code.h"

void strsim::soliton_generator::setup(
		strsim::degree_generator::value_type seed) {
	using value_type = strsim::degree_generator::value_type;
	// build the distribution table
	delete[] _cdf;
	_size = seed;
	_cdf = new double [seed];
	_cdf[0] = 1 / seed;
	for (value_type i = 1; i < seed; ++i) {
		_cdf[i] = _cdf[i-1] + 1 / (i * (i+1));
	}
}

strsim::degree_generator::value_type strsim::soliton_generator::sample() {
	using value_type = strsim::degree_generator::value_type;
	std::default_random_engine gen;
	double seed = _dist(gen);
	// search for seed over cdf using binary search
	value_type f = 0;
	value_type l = _size - 2;
	while (f < l) {
		value_type m = (f + l) / 2;
		if (_cdf[m] <= seed && seed < _cdf[m+1]) {
			return m + 1;
		}else if (_cdf[m] > seed) {
			l = m - 1;
		}else{
			f = m + 1;
		}
	}
	return f + 1;
}

void strsim::rateless_coder::encode(unsigned int inum, unsigned int onum,
		std::vector<strsim::coded_block *> &b) {
	
}

unsigned int strsim::rateless_coder::decode(strsim::coded_block * b) {
	
	return _block_left;
}


