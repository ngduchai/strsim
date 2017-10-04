
/*
 *	Estimate the CMF of arrival time, and the completion
 *	time of data with and without caching
 *	Input:
 *		- raw_size
 *		- dup_factor
 *		- cache factor
 *		- Disk latency parameters
 *	Output:
 *		- The CMF of arrival time
 *		- The CMF of completion time with caching
 *		- The CMF of completion time without caching
 *
 * */

#include <iostream>
#include <fstream>
#include <algorithm>
#include "store.h"

#define NUM_TEST 10000
#define TIME_RANGE 10000
#define VISUAL_TEST "visual/data/dlcmf"

int main(int argc, char* argv[]) {
	
	if (argc != 6) {
		std::cerr << "Usage dl [raw_size] [dup_factor]" <<
			" [cache_factor] [mu] [sigma]" << std::endl;
		return 1;
	}

	const unsigned int RAW_SIZE = std::stoi(argv[1]);
	const double DUP_FACTOR = std::stod(argv[2]);
	const double CACHE_FACTOR = std::stod(argv[3]);
	const double MEAN = std::stod(argv[4]);
	const double STDDEV = std::stod(argv[5]);

	const unsigned int DUP_SIZE = RAW_SIZE * DUP_FACTOR;
	const unsigned int CACHE_SIZE = RAW_SIZE * CACHE_FACTOR;
	strsim::gaussian_generator rg(MEAN, STDDEV);
	//const double RATE = 1 / MEAN;
	//strsim::exponential_generator rg(RATE);
	unsigned long * acmf = new unsigned long [TIME_RANGE];
	unsigned long * cmf = new unsigned long [TIME_RANGE];
	unsigned long * mcmf = new unsigned long [TIME_RANGE];
	std::vector<unsigned int> arrive;
	
	for (unsigned int i = 0; i < TIME_RANGE; ++i) {
		acmf[i] = 0;
		cmf[i] = 0;
		mcmf[i] = 0;
	}

	for (unsigned int i = 0; i < NUM_TEST; ++i) {
		arrive.clear();
		for (unsigned int j = 0; j < DUP_SIZE; ++j) {
			unsigned int rta = rg.sample();
			arrive.push_back(rta);
			for (unsigned int j = rta; j < TIME_RANGE; ++j) {
				acmf[j]++;
			}
		}
		std::sort(arrive.begin(), arrive.end(),
				[] (unsigned int f, unsigned int s) -> bool {
					return (f < s);
				});
		for (unsigned long j = arrive[RAW_SIZE-1]; j < TIME_RANGE; ++j) {
			cmf[j]++;
		}
		for (unsigned long j = arrive[RAW_SIZE - CACHE_SIZE - 1];
				j < TIME_RANGE; ++j) {
			mcmf[j]++;
		}
	}

	std::ofstream report(VISUAL_TEST);
	for (unsigned int i = 0; i < TIME_RANGE; ++i) {
		report << i << "," <<
			double(acmf[i]) / double(acmf[TIME_RANGE-1]) << "," <<
			double(cmf[i]) / double(cmf[TIME_RANGE-1]) << "," <<
			double(mcmf[i]) / double(mcmf[TIME_RANGE-1]) << std::endl;
	}
	
	report.close();

	delete [] acmf;
	delete [] cmf;
	delete [] mcmf;

}

