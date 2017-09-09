#include "code.h"
#include <iostream>

using namespace std;
using namespace strsim;

#define NUM_BLOCKS 20
#define NUM_TEST 10000

int main(void) {
	unsigned int * degrees = new unsigned int [NUM_BLOCKS];

	cout << "Test Soliton Distribution Generator" << endl;
	soliton_generator sg(NUM_BLOCKS);
	for (unsigned int i = 0; i < NUM_TEST; ++i) {
		degrees[sg.sample() - 1]++;
	}
	for (unsigned int i = 0; i < NUM_BLOCKS; ++i) {
		cout << i+1 << "," << double(degrees[i]) / NUM_TEST << endl;
	}
	
	delete[] degrees;
	return 0;
}

