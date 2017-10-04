
#include <iostream>
#include <fstream>
#include "store.h"

#define NUM_TEST 10000
#define MEAN 3.0
#define STDDEV 0.5
#define RATE 2
#define TIME_RANGE 10000
#define VISUAL_TEST "visual/data/test"

int main() {
	//strsim::gaussian_generator rg(MEAN, STDDEV);
	strsim::exponential_generator rg(RATE);
	unsigned long * cmf = new unsigned long [TIME_RANGE];
	
	for (unsigned int i = 0; i < TIME_RANGE; ++i) {
		cmf[i] = 0;
	}

	for (unsigned int i = 0; i < NUM_TEST; ++i) {
		long data = rg.sample();
		if (data < 0) {
			continue;
		}
		for (unsigned long j = data; j < TIME_RANGE; ++j) {
			cmf[j]++;
		}
	}

	std::ofstream report(VISUAL_TEST);
	for (unsigned int i = 0; i < TIME_RANGE; ++i) {
		report << i << "," << cmf[i] << std::endl;
	}
	
	report.close();

	delete [] cmf;

}

