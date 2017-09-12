#include "code.h"
#include "store.h"
#include <iostream>

using namespace std;
using namespace strsim;

#define NUM_BLOCKS 20
#define NUM_TEST 10000

#define SHAPE 3
#define RATE 2.0
#define DELAY 1.0

int main(void) {
	unsigned int * degrees = new unsigned int [NUM_BLOCKS];
	
	for (unsigned int i = 0; i < NUM_BLOCKS; ++i) {
		degrees[i] = 0;
	}
	cout << "Test Soliton Distribution Generator" << endl;
	soliton_generator sg(NUM_BLOCKS);
	for (unsigned int i = 0; i < NUM_TEST; ++i) {
		degrees[sg.sample() - 1]++;
	}
	for (unsigned int i = 0; i < NUM_BLOCKS; ++i) {
		cout << i+1 << "," << double(degrees[i]) / NUM_TEST << endl;
	}
	
	for (unsigned int i = 0; i < NUM_BLOCKS; ++i) {
		degrees[i] = 0;
	}
	cout << "Test Uniform Distribution Generator" << endl;
	uniform_generator ug(NUM_BLOCKS);
	for (unsigned int i = 0; i < NUM_TEST; ++i) {
		degrees[ug.sample() - 1]++;
	}
	for (unsigned int i = 0; i < NUM_BLOCKS; ++i) {
		cout << i+1 << "," << double(degrees[i]) / NUM_TEST << endl;
	}

	delete[] degrees;

	/*
	cout << "Test Erlang Distribution Generator" << endl;
	erlang_generator eg(SHAPE, RATE, DELAY);
	for (unsigned int i = 0; i < NUM_TEST; ++i) {
		cout << eg.sample() << endl;
	}
	*/
	
	return 0;
}

